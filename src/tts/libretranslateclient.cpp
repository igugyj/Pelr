#include "libretranslateclient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>

LibreTranslateClient::LibreTranslateClient(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)), m_baseUrl("http://localhost:5000")
{
}

void LibreTranslateClient::setPort(int port)
{
    m_baseUrl = QString("http://localhost:%1").arg(port);
}

void LibreTranslateClient::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
}

void LibreTranslateClient::translate(const QString &text, const QString &targetLang)
{
    // 构造请求 JSON，参考 fetch 示例
    QJsonObject jsonBody;
    jsonBody["q"] = text;
    jsonBody["source"] = "auto"; // 自动检测源语言
    jsonBody["target"] = targetLang;
    jsonBody["format"] = "text";
    jsonBody["alternatives"] = 3;
    jsonBody["api_key"] = ""; // 本地服务通常不需要密钥

    QJsonDocument doc(jsonBody);
    QByteArray postData = doc.toJson(QJsonDocument::Compact);

    postJson("/translate", postData);
}

void LibreTranslateClient::postJson(const QString &endpoint, const QByteArray &data)
{
    if (m_currentReply)
    {
        // 取消之前的请求（可选）
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_manager->post(request, data);
    m_currentReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
    });
}

void LibreTranslateClient::handleReply(QNetworkReply *reply)
{
    // 被更新的请求取代（abort 后晚到的旧事件）→ 忽略
    if (reply != m_currentReply)
    {
        reply->deleteLater();
        return;
    }
    m_currentReply = nullptr; // 先清指针再 emit，防重入时序问题

    if (reply->error() != QNetworkReply::NoError)
    {
        emit translationError(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();
    QJsonDocument respDoc = QJsonDocument::fromJson(responseData);
    if (!respDoc.isObject())
    {
        emit translationError("Invalid JSON response from LibreTranslate");
        return;
    }

    QJsonObject respObj = respDoc.object();
    if (respObj.contains("translatedText"))
    {
        QString result = respObj.value("translatedText").toString();
        emit translationFinished(result);
    }
    else if (respObj.contains("error"))
    {
        QString errMsg = respObj.value("error").toString();
        emit translationError(errMsg);
    }
    else
    {
        emit translationError("Unknown response format");
    }
}