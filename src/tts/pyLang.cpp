#include "pyLang.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include "NotificationWidget.h"
using MessageType = NotificationWidget::MessageType;
PyLang::PyLang(QObject *parent)
    : QObject(parent),
      m_manager(new QNetworkAccessManager(this)),
      m_baseUrl("http://localhost:9140")
{
    connect(this, &PyLang::errorOccurred, this, [](const QString &err)
            {
                qWarning() << "[PyLang] Error:" << err;
                NotificationWidget::showNotification(tr("Warning"),err); });
    connect(this, &PyLang::translationTestCompleted, this, [](bool success)
            { qDebug() << "[PyLang] Translation test completed:" << success;
                success?NotificationWidget::showNotification(tr("Success"),tr("Translation test successful")):NotificationWidget::showNotification(tr("Failed"),tr("Translation test failed"),5000,MessageType::Warning); });
}

void PyLang::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
}

QNetworkReply *PyLang::get(const QString &endpoint)
{
    QNetworkRequest request(QUrl(m_baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return m_manager->get(request);
}

QList<QString> PyLang::parseStringList(const QByteArray &json, const QString &key)
{
    QList<QString> list;
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isObject())
        return list;

    QJsonValue val = doc.object().value(key);
    if (!val.isArray())
        return list;

    const QJsonArray arr = val.toArray();
    for (const QJsonValue &v : arr)
        if (v.isString())
            list.append(v.toString());
    return list;
}

void PyLang::fetchLanguages()
{
    QNetworkReply *reply = get("/languages");
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            return;
        }
        QList<QString> langs = parseStringList(reply->readAll(), "languages");
        emit languagesReady(langs); });
}

void PyLang::fetchProviders()
{
    QNetworkReply *reply = get("/providers");
    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            return;
        }
        QList<QString> providers = parseStringList(reply->readAll(), "providers");
        emit providersReady(providers); });
}

// ========== 新增翻译测试函数 ==========
void PyLang::testTranslation(const QString &text, const QString &targetLang, const QString &provider)
{
    // 构造 JSON 请求体
    QJsonObject jsonBody;
    jsonBody["text"] = text;
    jsonBody["target_lang"] = targetLang;
    jsonBody["provider"] = provider;

    QJsonDocument doc(jsonBody);
    QByteArray postData = doc.toJson(QJsonDocument::Compact);

    // 创建 POST 请求
    QNetworkRequest request(QUrl(m_baseUrl + "/test_translation"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_manager->post(request, postData);

    connect(reply, &QNetworkReply::finished, this, [this, reply]()
            {
        reply->deleteLater();

        // 网络错误直接失败
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            emit translationTestCompleted(false);
            return;
        }

        // 解析响应 JSON
        QByteArray responseData = reply->readAll();
        QJsonDocument respDoc = QJsonDocument::fromJson(responseData);

        if (!respDoc.isObject()) {
            emit errorOccurred("Invalid JSON response");
            emit translationTestCompleted(false);
            return;
        }

        QJsonObject respObj = respDoc.object();
        bool success = respObj.value("success").toBool(false);
        emit translationTestCompleted(success); });
}