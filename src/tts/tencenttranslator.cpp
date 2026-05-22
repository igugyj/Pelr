#include "tencenttranslator.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageAuthenticationCode>
#include <QCryptographicHash>
#include <QNetworkRequest>
#include <QUrl>
#include <QEventLoop>
#include <QTimer>

static const QString API_HOST = QStringLiteral("tmt.tencentcloudapi.com");
static const QString API_URL = QStringLiteral("https://") + API_HOST;
static const QString API_VERSION = QStringLiteral("2018-03-21");

TencentTranslator::TencentTranslator(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)), m_projectId(0)
{
}

void TencentTranslator::setCredentials(const QString &secretId,
                                       const QString &secretKey,
                                       const QString &region,
                                       int projectId)
{
    m_secretId = secretId;
    m_secretKey = secretKey;
    m_region = region.isEmpty() ? QStringLiteral("ap-guangzhou") : region;
    m_projectId = projectId;
}

void TencentTranslator::translate(const QString &text,
                                  const QString &targetLang,
                                  const QString &sourceLang)
{
    if (m_secretId.isEmpty() || m_secretKey.isEmpty())
    {
        emit translationError(QStringLiteral("Tencent credentials not set"));
        return;
    }
    if (text.isEmpty())
    {
        emit translationError(QStringLiteral("Empty text for translation"));
        return;
    }

    // 构建请求体
    QJsonObject body;
    body[QStringLiteral("SourceText")] = text;
    body[QStringLiteral("Source")] = sourceLang.isEmpty() ? QStringLiteral("auto") : sourceLang;
    body[QStringLiteral("Target")] = targetLang;
    body[QStringLiteral("ProjectId")] = m_projectId;

    QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);

    // 获取当前 UTC 时间
    QDateTime now = QDateTime::currentDateTimeUtc();

    // 发送请求
    QNetworkReply *reply = sendRequest(payload, QStringLiteral("TextTranslate"), now);
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
            {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit translationError(QStringLiteral("Network error: %1").arg(reply->errorString()));
            return;
        }

        const QByteArray responseData = reply->readAll();
        QString errorMsg;
        const QString result = parseResponse(responseData, errorMsg);

        if (result.isEmpty() && !errorMsg.isEmpty()) {
            emit translationError(errorMsg);
        } else {
            emit translationFinished(result);
        } });
}

QByteArray TencentTranslator::sign(const QString &service,
                                   const QString &action,
                                   const QByteArray &payload,
                                   const QDateTime &timestamp) const
{
    // 步骤 1：拼接规范请求串
    const QString httpRequestMethod = QStringLiteral("POST");
    const QString canonicalUri = QStringLiteral("/");
    const QString canonicalQueryString; // 空
    const QString canonicalHeaders =
        QStringLiteral("content-type:application/json; charset=utf-8\nhost:") + API_HOST + QStringLiteral("\n");
    const QString signedHeaders = QStringLiteral("content-type;host");
    const QByteArray hashedPayload = QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex().toLower();

    const QString canonicalRequest =
        httpRequestMethod + QStringLiteral("\n") +
        canonicalUri + QStringLiteral("\n") +
        canonicalQueryString + QStringLiteral("\n") +
        canonicalHeaders + QStringLiteral("\n") +
        signedHeaders + QStringLiteral("\n") +
        QString::fromLatin1(hashedPayload);

    // 步骤 2：拼接待签名字符串
    const QString dateStr = timestamp.toString(QStringLiteral("yyyy-MM-dd"));
    const QString timestampStr = QString::number(timestamp.toSecsSinceEpoch());
    const QString credentialScope = dateStr + QStringLiteral("/") + service + QStringLiteral("/tc3_request");
    const QByteArray hashedCanonicalRequest =
        QCryptographicHash::hash(canonicalRequest.toUtf8(), QCryptographicHash::Sha256).toHex().toLower();

    const QString stringToSign =
        QStringLiteral("TC3-HMAC-SHA256\n") +
        timestampStr + QStringLiteral("\n") +
        credentialScope + QStringLiteral("\n") +
        QString::fromLatin1(hashedCanonicalRequest);

    // 步骤 3：计算签名
    const QByteArray secretDate =
        QMessageAuthenticationCode::hash(
            dateStr.toUtf8(),
            ("TC3" + m_secretKey).toUtf8(),
            QCryptographicHash::Sha256);
    const QByteArray secretService =
        QMessageAuthenticationCode::hash(
            service.toUtf8(),
            secretDate,
            QCryptographicHash::Sha256);
    const QByteArray secretSigning =
        QMessageAuthenticationCode::hash(
            QByteArray("tc3_request"),
            secretService,
            QCryptographicHash::Sha256);
    const QByteArray signature =
        QMessageAuthenticationCode::hash(
            stringToSign.toUtf8(),
            secretSigning,
            QCryptographicHash::Sha256)
            .toHex()
            .toLower();

    // 步骤 4：拼接 Authorization
    const QString authorization =
        QStringLiteral("TC3-HMAC-SHA256 Credential=%1/%2, SignedHeaders=%3, Signature=%4")
            .arg(m_secretId, credentialScope, signedHeaders, QString::fromLatin1(signature));

    return authorization.toUtf8();
}

QNetworkReply *TencentTranslator::sendRequest(const QByteArray &payload,
                                              const QString &action,
                                              const QDateTime &timestamp)
{
    QNetworkRequest request{QUrl(API_URL)}; // 大括号，告别 Vexing Parse
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json; charset=utf-8"));

    const QString timestampStr = QString::number(timestamp.toSecsSinceEpoch());
    request.setRawHeader("X-TC-Action", action.toUtf8());
    request.setRawHeader("X-TC-Version", API_VERSION.toUtf8());
    request.setRawHeader("X-TC-Timestamp", timestampStr.toUtf8());
    request.setRawHeader("X-TC-Region", m_region.toUtf8());

    QByteArray authorization = sign(QStringLiteral("tmt"), action, payload, timestamp);
    request.setRawHeader("Authorization", authorization);

    return m_manager->post(request, payload);
}

QString TencentTranslator::parseResponse(const QByteArray &responseData, QString &errorMsg) const
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        errorMsg = QStringLiteral("JSON parse error: %1").arg(parseError.errorString());
        return {};
    }

    QJsonObject root = doc.object();
    if (root.contains(QStringLiteral("Response")))
    {
        QJsonObject resp = root[QStringLiteral("Response")].toObject();
        if (resp.contains(QStringLiteral("Error")))
        {
            QJsonObject err = resp[QStringLiteral("Error")].toObject();
            errorMsg = QStringLiteral("%1: %2")
                           .arg(err[QStringLiteral("Code")].toString(),
                                err[QStringLiteral("Message")].toString());
            return {};
        }
        if (resp.contains(QStringLiteral("TargetText")))
        {
            return resp[QStringLiteral("TargetText")].toString();
        }
        errorMsg = QStringLiteral("Response does not contain TargetText");
        return {};
    }

    errorMsg = QStringLiteral("Invalid response format");
    return {};
}

void TencentTranslator::testTranslate(const TTSConfig &config,
                                      const QString &text,
                                      QString *msg)
{
    if (!msg)
        return;

    if (text.isEmpty())
    {
        *msg = QStringLiteral("Error: Empty text");
        return;
    }

    // 创建临时对象，只负责这一次翻译
    TencentTranslator translator;
    translator.setCredentials(config.tr_tx_secret_id,
                              config.tr_tx_secret_key,
                              config.tr_tx_region.isEmpty() ? QStringLiteral("ap-guangzhou") : config.tr_tx_region,
                              config.tr_tx_project_id);

    QString targetLang = config.tr_tx_target_lang;
    if (targetLang.isEmpty())
    {
        *msg = QStringLiteral("Error: Target language not set");
        return;
    }

    QString sourceLang = config.tr_tx_source_lang.isEmpty()
                             ? QStringLiteral("auto")
                             : config.tr_tx_source_lang;

    QEventLoop loop;
    QString result;
    QString err;

    QObject::connect(&translator, &TencentTranslator::translationFinished,
                     &loop, [&](const QString &text)
                     {
                         result = text;
                         loop.quit(); });

    QObject::connect(&translator, &TencentTranslator::translationError,
                     &loop, [&](const QString &error)
                     {
                         err = error;
                         loop.quit(); });

    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]()
                     {
        err = QStringLiteral("Request timeout");
        loop.quit(); });
    timer.start(10000);

    translator.translate(text, targetLang, sourceLang);
    loop.exec();

    if (!err.isEmpty())
        *msg = QStringLiteral("Error: ") + err;
    else
        *msg = result;

    qDebug() << "[TencentTr]" << msg;
}