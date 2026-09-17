#include "logger.hpp"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QMutexLocker>

#include <cstdio>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#define QT_LOG_FILE "log/Pelr_qt.log"

LogLevel g_logLevel = defaultLogLevel();
QMutex g_logMutex;

void initLogFile()
{
    QDir().mkpath("log");

    QFileInfo fileInfo(QT_LOG_FILE);
    if (fileInfo.exists())
        QFile::remove(QT_LOG_FILE);
}

void setLogLevel(LogLevel level)
{
    QMutexLocker locker(&g_logMutex);
    g_logLevel = level;
}

LogLevel getLogLevel()
{
    QMutexLocker locker(&g_logMutex);
    return g_logLevel;
}

static LogLevel qtMsgTypeToLogLevel(QtMsgType type)
{
    switch (type)
    {
    case QtDebugMsg:
        return LogLevel::Debug;
    case QtInfoMsg:
        return LogLevel::Info;
    case QtWarningMsg:
        return LogLevel::Warning;
    case QtCriticalMsg:
        return LogLevel::Critical;
    case QtFatalMsg:
        return LogLevel::Fatal;
    default:
        return LogLevel::Info;
    }
}

static const char *logLevelName(QtMsgType type)
{
    switch (type)
    {
    case QtDebugMsg:
        return "Debug";
    case QtInfoMsg:
        return "Info";
    case QtWarningMsg:
        return "Warning";
    case QtCriticalMsg:
        return "Critical";
    case QtFatalMsg:
        return "Fatal";
    default:
        return "Unknown";
    }
}

static void writeToConsole(const QString &txt)
{
#ifdef Q_OS_WIN
    HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
    DWORD mode = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode))
    {
        DWORD written = 0;
        WriteConsoleW(h,
                      reinterpret_cast<const wchar_t *>(txt.utf16()),
                      static_cast<DWORD>(txt.size()),
                      &written,
                      nullptr);
        WriteConsoleW(h, L"\n", 1, &written, nullptr);
        return;
    }
#endif
    const QByteArray u = txt.toUtf8();
    std::fwrite(u.constData(), 1, static_cast<size_t>(u.size()), stderr);
    std::fputc('\n', stderr);
    std::fflush(stderr);
}

static void writeToFile(const QString &txt)
{
    QFile logFile(QT_LOG_FILE);
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream textStream(&logFile);
    textStream.setEncoding(QStringConverter::Utf8);
    textStream << txt << Qt::endl;
}

void messageHandler(QtMsgType type,
                    const QMessageLogContext &context,
                    const QString &msg)
{
    Q_UNUSED(context)

    QMutexLocker locker(&g_logMutex);

    if (static_cast<int>(qtMsgTypeToLogLevel(type)) < static_cast<int>(g_logLevel))
        return;

    const QString logTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    const QString txt = QString("[%1] [%2]: %3")
                            .arg(logTime)
                            .arg(QString::fromLatin1(logLevelName(type)))
                            .arg(msg);

#ifdef CONSOLE
    writeToConsole(txt);
#endif

    writeToFile(txt);
}