#pragma once

#include <QString>
#include <QMutex>
#include <QDataStream>
#include <QFile>
#include <QMessageBox>
#include <QUrl>
#include <QDebug>

#define LOG_LEVEL_FILE "user/logLevel.dat"

enum class LogLevel
{
    Debug = 0,
    Info = 1,
    Warning = 2,
    Critical = 3,
    Fatal = 4
};

extern LogLevel g_logLevel;
extern QMutex g_logMutex;

void initLogFile();
void setLogLevel(LogLevel level);
LogLevel getLogLevel();
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#ifdef CONSOLE
#define LOG_TO_CONSOLE(txt)                      \
    do                                           \
    {                                            \
        QMutexLocker locker(&g_logMutex);        \
        QTextStream out(stdout);                 \
        out.setEncoding(QStringConverter::Utf8); \
        out << txt << Qt::endl;                  \
    } while (0)
#else
#define LOG_TO_CONSOLE(txt) ((void)0)
#endif

inline LogLevel defaultLogLevel()
{
#ifdef CONSOLE
    return LogLevel::Debug;
#else
    return LogLevel::Warning;
#endif
}

inline LogLevel read_log_level()
{
    QFile file(LOG_LEVEL_FILE);
    if (!file.open(QIODevice::ReadOnly))
        return defaultLogLevel();

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    LogLevel level = defaultLogLevel();
    in >> level;
    file.close();

    if (in.status() != QDataStream::Ok)
        return defaultLogLevel();

    return level;
}

inline void write_log_level(const LogLevel level)
{
    setLogLevel(level);

    QFile file(LOG_LEVEL_FILE);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(nullptr, "Error", "Failed to write data!");
        qCritical() << "[Log] Write data failed: can not open file" << LOG_LEVEL_FILE;
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);
    out << level;
    file.close();
}

inline QString maskUrl(const QString &urlStr)
{
    QUrl u(urlStr);
    if (!u.isValid())
        return urlStr.left(40);
    return u.toString(QUrl::RemoveUserInfo | QUrl::RemoveQuery);
}