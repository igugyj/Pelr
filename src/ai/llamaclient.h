#ifndef LLAMACLIENT_H
#define LLAMACLIENT_H

#include <QObject>
#include <QString>
#include <QJsonArray>

class QNetworkAccessManager;
class QNetworkReply;

class LlamaClient : public QObject
{
    Q_OBJECT

public:
    // 获取单例实例
    static LlamaClient *instance();

    // 一键配置全部参数（首次使用前必须调用一次）
    void configure(const QString &baseUrl = QString(),
                   const QString &apiKey = QString(),
                   const QString &model = QString(),
                   const QString &systemPrompt = QString(),
                   int maxContextMessages = -1);

    // 单独设置项
    void setBaseUrl(const QString &url);
    void setApiKey(const QString &apiKey);
    void setModel(const QString &model);
    void setSystemPrompt(const QString &prompt);
    bool loadSystemPromptFromFile(const QString &filePath);
    void setMaxContextMessages(int count);
    void clearHistory();

    // 同步生成（保留历史）
    QString generateText(const QString &prompt, const int &id, bool stream = false);
    // 异步生成（使用历史，推荐）
    void generateTextAsync(const QString &prompt, const int &id, bool stream = false);

    bool isConfigured() const;
    void generateRandomAsync(const QString &prompt, const int &id);
    void clearRandomHistory();

signals:
    void textGenerated(const QString &text, int index);
    void errorOccurred(const QString &error, int index);

private:
    // 私有构造与析构（禁止外部创建）
    explicit LlamaClient(QObject *parent = nullptr);
    ~LlamaClient();
    LlamaClient(const LlamaClient &) = delete;
    LlamaClient &operator=(const LlamaClient &) = delete;

    // 单例指针
    static LlamaClient *m_instance;
    int m_id = 1; // 1 box 2 window
    QNetworkAccessManager *m_manager;
    QString m_baseUrl;
    QString m_apiKey;
    QString m_model;
    QJsonArray m_messages;    // 完整消息历史（包含system）
    QJsonArray m_randomMessages; // 随机语录独立历史
    int m_maxContextMessages; // 最大保留轮数，-1不限

    void applyContextCompression();
    void doGenerate(const QString &prompt, bool stream);

private slots:
    void onReplyFinished(QNetworkReply *reply);
};

#endif // LLAMACLIENT_H