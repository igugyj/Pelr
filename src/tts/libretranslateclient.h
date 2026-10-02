#ifndef LIBRETRANSLATECLIENT_H
#define LIBRETRANSLATECLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

/**
 * @brief 调用本地 LibreTranslate 服务（默认端口 5000）的客户端
 */
class LibreTranslateClient : public QObject
{
    Q_OBJECT

public:
    explicit LibreTranslateClient(QObject *parent = nullptr);
    ~LibreTranslateClient() override = default;

    void setPort(int port);              // 设置服务端口，默认 5000
    void setBaseUrl(const QString &url); // 直接设置完整基础 URL（覆盖 port 设置）

    /**
     * @brief 异步翻译，源语言自动检测（auto）
     * @param text       待翻译文本
     * @param targetLang 目标语言代码，如 "ja"（日语）、"zh"（中文）
     */
    void translate(const QString &text, const QString &targetLang);

signals:
    void translationFinished(const QString &translatedText);
    void translationError(const QString &errorMessage);

private:
    void handleReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    QString m_baseUrl; // 完整基础 URL，如 "http://localhost:5000"

    QNetworkReply *m_currentReply = nullptr;

    void postJson(const QString &endpoint, const QByteArray &data);
};

#endif // LIBRETRANSLATECLIENT_H