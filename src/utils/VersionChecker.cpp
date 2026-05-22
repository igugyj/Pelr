
#include "VersionChecker.h"
#include "data.hpp"
#include <QDebug>
#include <QSslSocket>
#include <QJsonArray>
#include <QRegularExpression>

void checkSslSupport()
{
    qDebug() << "[UpdateCheck] SSL support:" << QSslSocket::supportsSsl();
    qDebug() << "[UpdateCheck] SSL version:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "[UpdateCheck] SSL build version:" << QSslSocket::sslLibraryBuildVersionString();
}

// 提取主版本号（忽略前导日期和后缀，只取第一个数字段）
static int extractMajorVersion(const QString &version)
{
    QString trimmed = version.trimmed();
    // 去除前导 'v' 或 'V'
    if (trimmed.startsWith('v', Qt::CaseInsensitive))
        trimmed = trimmed.mid(1);

    // 正则：匹配可选的8位日期前缀和点，然后捕获第一个数字（主版本号）
    QRegularExpression re(R"(^(?:\d{8}\.)?(\d+))");
    QRegularExpressionMatch match = re.match(trimmed);
    if (match.hasMatch())
    {
        return match.captured(1).toInt();
    }
    return -1; // 解析失败
}

int VersionChecker::compareVersions(const QString &v1, const QString &v2)
{
    int major1 = extractMajorVersion(v1);
    int major2 = extractMajorVersion(v2);
    if (major1 < 0 || major2 < 0)
    {
        // 解析失败时回退到字符串比较
        return QString::compare(v1, v2);
    }
    if (major1 < major2)
        return -1;
    if (major1 > major2)
        return 1;
    return 0;
}

VersionChecker::VersionChecker(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
    checkSslSupport();
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &VersionChecker::onReplyFinished);
    connect(this, &VersionChecker::sourceError,
            this, [](const QString &source, const QString &error)
             { qDebug() << "[UpdateCheck] Source:" << source << "Error:" << error; });
}

VersionChecker::~VersionChecker()
{
    for (auto reply : m_pendingReplies)
    {
        if (reply)
        {
            reply->abort();
            reply->deleteLater();
        }
    }
    for (auto timer : m_timeoutTimers)
    {
        if (timer)
            timer->deleteLater();
    }
}

void VersionChecker::checkVersionMatch(const QString &localVersion)
{
    m_localVersion = localVersion;
    m_pendingReplies.clear();
    m_timeoutTimers.clear();
    m_results.clear();

    checkByGitee(DataManager::instance().const_config_data.Gitee_repo_owner.toLower(),
                 DataManager::instance().const_config_data.Gitee_repo_name.toLower());
    checkByGithub(DataManager::instance().const_config_data.Github_repo_owner,
                  DataManager::instance().const_config_data.Github_repo_name);
    checkByPelr(); // 自建 API (占位)
}

void VersionChecker::checkByGitee(const QString &owner, const QString &repo)
{
    SourceCheckResult result;
    result.sourceName = "Gitee";
    result.url = QString("https://gitee.com/api/v5/repos/%1/%2/releases/latest")
                     .arg(owner, repo);
    m_results["Gitee"] = result;

    QNetworkRequest request;
    request.setUrl(QUrl(result.url));
    QString userAgent = "APP" + DataManager::instance().const_config_data.Gitee_repo_name + "_" +
                        DataManager::instance().const_config_data.version;
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);

    QNetworkReply *reply = m_networkManager->get(request);
    m_pendingReplies.insert(reply);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(10000);
    m_timeoutTimers[reply] = timer;
    connect(timer, &QTimer::timeout, this, &VersionChecker::onTimeout);
}

void VersionChecker::checkByGithub(const QString &owner, const QString &repo)
{
    SourceCheckResult result;
    result.sourceName = "GitHub";
    result.url = QString("https://api.github.com/repos/%1/%2/releases/latest")
                     .arg(owner, repo);
    m_results["GitHub"] = result;

    QNetworkRequest request;
    request.setUrl(QUrl(result.url));
    QString userAgent = "APP" + DataManager::instance().const_config_data.Github_repo_name + "_" +
                        DataManager::instance().const_config_data.version;
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);

    QNetworkReply *reply = m_networkManager->get(request);
    m_pendingReplies.insert(reply);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(10000);
    m_timeoutTimers[reply] = timer;
    connect(timer, &QTimer::timeout, this, &VersionChecker::onTimeout);
}

void VersionChecker::checkByPelr()
{
    SourceCheckResult result;
    result.sourceName = "Pelr";
    result.url = "https://pg25-lsae.eu.org/api/pelr/latest.json";
    m_results["Pelr"] = result;
    /*
    // 模拟失败，避免阻塞
    QTimer::singleShot(0, this, [this]() {
        SourceCheckResult &res = m_results["Pelr"];
        res.success = false;
        res.errorString = "Pelr API not implemented yet";
        emit sourceError("Pelr", res.errorString);
        finalizeIfAllFinished();
    });
    */

    // 实际启用时使用如下代码
    QNetworkRequest request;
    request.setUrl(QUrl(result.url));
    request.setHeader(QNetworkRequest::UserAgentHeader, "PelrApp");
    QNetworkReply *reply = m_networkManager->get(request);
    m_pendingReplies.insert(reply);
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(10000);
    m_timeoutTimers[reply] = timer;
    connect(timer, &QTimer::timeout, this, &VersionChecker::onTimeout);
}

void VersionChecker::onTimeout()
{
    QTimer *timer = qobject_cast<QTimer *>(sender());
    if (!timer)
        return;

    // 找到对应的 reply
    QNetworkReply *reply = nullptr;
    for (auto it = m_timeoutTimers.begin(); it != m_timeoutTimers.end(); ++it)
    {
        if (it.value() == timer)
        {
            reply = it.key();
            // 从容器中移除，防止 finished 槽再次处理
            it.value()->disconnect(this); // 断开定时器信号
            m_timeoutTimers.erase(it);
            break;
        }
    }

    if (reply)
    {
        if (reply->isRunning())
        {
            reply->abort();
        }
        // 如果 reply 还未从 pending 中移除，手动移除
        m_pendingReplies.remove(reply);
    }
    timer->deleteLater();
}

void VersionChecker::onReplyFinished(QNetworkReply *reply)
{
    if (m_timeoutTimers.contains(reply))
    {
        m_timeoutTimers[reply]->deleteLater();
        m_timeoutTimers.remove(reply);
    }

    m_pendingReplies.remove(reply);
    reply->deleteLater();

    QString url = reply->url().toString();
    QString sourceName;
    if (url.contains("gitee.com"))
    {
        sourceName = "Gitee";
    }
    else if (url.contains("api.github.com"))
    {
        sourceName = "GitHub";
    }
    else
    {
        sourceName = "Pelr";
    }

    processReply(reply, sourceName);
    finalizeIfAllFinished();
}

void VersionChecker::processReply(QNetworkReply *reply, const QString &sourceName)
{
    if (!m_results.contains(sourceName))
    {
        qWarning() << "[UpdateCheck] Unknown source name:" << sourceName;
        return;
    }

    SourceCheckResult &result = m_results[sourceName];

    if (reply->error() != QNetworkReply::NoError)
    {
        result.success = false;
        if (reply->error() == QNetworkReply::OperationCanceledError)
            result.errorString = "Request timeout or canceled";
        else
            result.errorString = reply->errorString();
        emit sourceError(sourceName, result.errorString);
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        result.success = false;
        result.errorString = QString("JSON parse error: %1").arg(parseError.errorString());
        emit sourceError(sourceName, result.errorString);
        return;
    }

    if (!jsonDoc.isObject())
    {
        result.success = false;
        result.errorString = "Response is not a JSON object";
        emit sourceError(sourceName, result.errorString);
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    if (!jsonObj.contains("tag_name"))
    {
        result.success = false;
        result.errorString = "Missing 'tag_name' field";
        emit sourceError(sourceName, result.errorString);
        return;
    }
    result.latestVersion = jsonObj["tag_name"].toString();

    if (jsonObj.contains("body"))
    {
        result.body = jsonObj["body"].toString();
    }

    if (sourceName == "GitHub" || sourceName == "Gitee")
    {
        if (jsonObj.contains("assets") && jsonObj["assets"].isArray())
        {
            QJsonArray assets = jsonObj["assets"].toArray();
            if (!assets.isEmpty())
            {
                QJsonObject firstAsset = assets[0].toObject();
                result.downloadUrl = firstAsset["browser_download_url"].toString();
            }
        }
    }
    else if (sourceName == "Pelr")
    {
        if (jsonObj.contains("download_url"))
        {
            result.downloadUrl = jsonObj["download_url"].toString();
        }
    }

    QString dateField = (sourceName == "Gitee") ? "created_at" : "published_at";
    if (jsonObj.contains(dateField))
    {
        result.publishedAt = jsonObj[dateField].toString();
    }

    result.success = true;
    qDebug() << "[UpdateCheck]" << result.success << result.sourceName << result.latestVersion << result.publishedAt;
}

void VersionChecker::finalizeIfAllFinished()
{
    if (!m_pendingReplies.isEmpty())
        return;

    VersionCheckSummary summary;
    summary.localVersion = m_localVersion;
    summary.results = m_results.values();
    summary.anySuccess = false;
    summary.anyNewerVersion = false;
    summary.newestRemoteVersion.clear();

    // 找出所有成功源中的最高远程版本（按主版本号比较）
    for (const auto &res : summary.results)
    {
        if (res.success)
        {
            summary.anySuccess = true;
            if (summary.newestRemoteVersion.isEmpty() ||
                compareVersions(res.latestVersion, summary.newestRemoteVersion) > 0)
            {
                summary.newestRemoteVersion = res.latestVersion;
            }
        }
    }

    // 判断是否有比本地更新的版本（基于主版本号）
    if (summary.anySuccess && !summary.newestRemoteVersion.isEmpty())
    {
        summary.anyNewerVersion = (compareVersions(summary.newestRemoteVersion, m_localVersion) > 0);
    }

    emit versionCheckResult(summary);

    QString msg;
    if (summary.anyNewerVersion)
    {
        msg = QString("[UPDATE] New version available. Latest: %1 (Local: %2)")
                  .arg(summary.newestRemoteVersion, m_localVersion);
        emit versionCheckCompleted(false, msg);
    }
    else if (summary.anySuccess)
    {
        msg = QString("[OK] Version %1 is up to date.").arg(m_localVersion);
        emit versionCheckCompleted(true, msg);
    }
    else
    {
        msg = "[FAIL] Unable to check for updates.";
        emit versionCheckCompleted(false, msg);
        emit errorOccurred(msg);
    }
    qDebug() << "[UpdateCheck]" << msg;
}
