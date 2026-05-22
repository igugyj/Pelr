#include "llamaclient.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCoreApplication>
#include <QEventLoop>
#include <QMutexLocker>
#include <QMutex>

// 静态成员初始化
LlamaClient *LlamaClient::m_instance = nullptr;
static QMutex mutex;

LlamaClient::LlamaClient(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)), m_model("gpt-3.5-turbo"), m_maxContextMessages(-1)
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &LlamaClient::onReplyFinished);
}

LlamaClient::~LlamaClient()
{
    m_manager->deleteLater();
}

// 获取单例（线程安全）
LlamaClient *LlamaClient::instance()
{
    if (!m_instance)
    {
        QMutexLocker locker(&mutex);
        if (!m_instance)
        {
            m_instance = new LlamaClient();

            // 应用程序退出时自动清理实例
            connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                    []()
                    {
                        QMutexLocker lock(&mutex);
                        delete m_instance;
                        m_instance = nullptr;
                    });
        }
    }
    return m_instance;
}

// ---------- 一键配置 ----------
void LlamaClient::configure(const QString &baseUrl, const QString &apiKey,
                            const QString &model, const QString &systemPrompt,
                            int maxContextMessages)
{
    if (!baseUrl.isEmpty())
    {
        m_baseUrl = baseUrl;
        if (m_baseUrl.endsWith('/'))
            m_baseUrl.chop(1);
    }
    if (!apiKey.isEmpty())
        m_apiKey = apiKey;
    if (!model.isEmpty())
        m_model = model;
    if (!systemPrompt.isEmpty())
        setSystemPrompt(systemPrompt);
    if (maxContextMessages >= 0)
        m_maxContextMessages = maxContextMessages;
}

// ---------- 分离设置项 ----------
void LlamaClient::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
    if (m_baseUrl.endsWith('/'))
        m_baseUrl.chop(1);
}

void LlamaClient::setApiKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void LlamaClient::setModel(const QString &model)
{
    m_model = model;
}

void LlamaClient::setSystemPrompt(const QString &prompt)
{
    if (prompt.isEmpty())
        return;
    bool found = false;
    for (int i = 0; i < m_messages.size(); ++i)
    {
        QJsonObject msg = m_messages[i].toObject();
        if (msg["role"].toString() == "system")
        {
            msg["content"] = prompt;
            m_messages[i] = msg;
            found = true;
            break;
        }
    }
    if (!found)
    {
        QJsonObject sysMsg;
        sysMsg["role"] = "system";
        sysMsg["content"] = prompt;
        m_messages.prepend(sysMsg);
    }
}

bool LlamaClient::loadSystemPromptFromFile(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;
    QFile file(filePath);
    if (!file.exists())
    {
        qDebug() << "[AI] File does not exist:" << filePath;
        return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "[AI] Cannot open file:" << filePath;
        return false;
    }
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    if (content.isEmpty())
        return false;
    setSystemPrompt(content);
    return true;
}

void LlamaClient::setMaxContextMessages(int count)
{
    m_maxContextMessages = count;
    applyContextCompression();
}

void LlamaClient::clearHistory()
{
    QJsonArray newArray;
    for (const QJsonValue &val : m_messages)
    {
        QJsonObject msg = val.toObject();
        if (msg["role"].toString() == "system")
        {
            newArray.append(msg);
            break;
        }
    }
    m_messages = newArray;
}

void LlamaClient::applyContextCompression()
{
    if (m_maxContextMessages < 0)
        return;

    QJsonArray compressed;
    for (const QJsonValue &val : m_messages)
    {
        QJsonObject msg = val.toObject();
        if (msg["role"].toString() == "system")
        {
            compressed.append(msg);
            break;
        }
    }

    QJsonArray history;
    for (const QJsonValue &val : m_messages)
    {
        QJsonObject msg = val.toObject();
        if (msg["role"].toString() != "system")
            history.append(msg);
    }

    if (history.isEmpty())
    {
        m_messages = compressed;
        return;
    }

    int keep = m_maxContextMessages * 2;
    if (history.size() > keep)
    {
        QJsonArray recent;
        for (int i = history.size() - keep; i < history.size(); ++i)
            recent.append(history[i]);
        history = recent;
    }

    for (const QJsonValue &val : history)
        compressed.append(val);
    m_messages = compressed;
}

void LlamaClient::doGenerate(const QString &prompt, bool stream)
{
    if (m_baseUrl.isEmpty())
    {
        emit errorOccurred("Base URL not configured. Call configure() first.", m_id);
        return;
    }

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = prompt;
    m_messages.append(userMsg);
    applyContextCompression();

    QNetworkRequest request(QUrl(m_baseUrl + "/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_apiKey.isEmpty())
        request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    QJsonObject jsonBody;
    jsonBody["model"] = m_model;
    jsonBody["messages"] = m_messages;
    jsonBody["stream"] = stream;

    m_manager->post(request, QJsonDocument(jsonBody).toJson());
}

QString LlamaClient::generateText(const QString &prompt, const int &id, bool stream)
{
    m_id = id;
    if (m_baseUrl.isEmpty())
        return "Error: Base URL not configured.";

    QJsonArray tempMessages = m_messages;
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = prompt;
    tempMessages.append(userMsg);

    if (m_maxContextMessages >= 0)
    {
        QJsonArray compressed;
        for (const auto &v : tempMessages)
        {
            if (v.toObject()["role"].toString() == "system")
            {
                compressed.append(v);
                break;
            }
        }
        QJsonArray history;
        for (const auto &v : tempMessages)
        {
            if (v.toObject()["role"].toString() != "system")
                history.append(v);
        }
        int keep = m_maxContextMessages * 2;
        if (history.size() > keep)
        {
            QJsonArray recent;
            for (int i = history.size() - keep; i < history.size(); ++i)
                recent.append(history[i]);
            history = recent;
        }
        for (const auto &v : history)
            compressed.append(v);
        tempMessages = compressed;
    }

    QNetworkRequest request(QUrl(m_baseUrl + "/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_apiKey.isEmpty())
        request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    QJsonObject jsonBody;
    jsonBody["model"] = m_model;
    jsonBody["messages"] = tempMessages;
    jsonBody["stream"] = stream;

    QNetworkReply *reply = m_manager->post(request, QJsonDocument(jsonBody).toJson());
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        QString err = QString("Network error: %1").arg(reply->errorString());
        reply->deleteLater();
        return err;
    }

    QByteArray resp = reply->readAll();
    reply->deleteLater();
    QJsonObject obj = QJsonDocument::fromJson(resp).object();
    QJsonArray choices = obj["choices"].toArray();
    if (choices.isEmpty())
        return "Error: No choices in response";
    return choices.first().toObject()["message"].toObject()["content"].toString();
}

void LlamaClient::generateTextAsync(const QString &prompt, const int &id, bool stream)
{
    m_id = id;
    doGenerate(prompt, stream);
}

void LlamaClient::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        emit errorOccurred(QString("Network error: %1").arg(reply->errorString()), m_id);
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    QJsonArray choices = obj["choices"].toArray();
    if (choices.isEmpty())
    {
        emit errorOccurred("No choices in response", m_id);
        return;
    }

    QJsonObject message = choices.first().toObject()["message"].toObject();
    QString content = message["content"].toString();
    if (content.isEmpty())
    {
        emit errorOccurred("Empty content from AI", m_id);
        return;
    }

    QJsonObject assistantMsg;
    assistantMsg["role"] = "assistant";
    assistantMsg["content"] = content;
    m_messages.append(assistantMsg);
    applyContextCompression();

    emit textGenerated(content, m_id);
}