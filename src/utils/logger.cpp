
#include "logger.hpp"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#include <QDir>
#include <QMutexLocker>

// 定义日志文件路径
#define QT_LOG_FILE "log/Pelr_qt.log"

// 定义全局变量
LogLevel g_logLevel = LogLevel::Debug;
QMutex g_logMutex;

// 输出到控制台的宏（如果启用）
#ifdef CONSOLE
#define LOG_TO_CONSOLE(txt)               \
    do                                    \
    {                                     \
        QMutexLocker locker(&g_logMutex); \
        QTextStream out(stdout);          \
        out << txt << Qt::endl;           \
    } while (0)
#else
#define LOG_TO_CONSOLE(txt)
#endif

// 初始化日志文件
void initLogFile()
{
    QDir().mkpath("log");

    QFileInfo fileInfo(QT_LOG_FILE);
    if (fileInfo.exists())
    {
        QFile::remove(QT_LOG_FILE);
    }
}

// 设置日志等级
void setLogLevel(LogLevel level)
{
    QMutexLocker locker(&g_logMutex);
    g_logLevel = level;
    // 测试输出，确认设置成功
    std::cout << "setLogLevel called: new level = " << static_cast<int>(level) << std::endl;
}

// 获取当前日志等级
LogLevel getLogLevel()
{
    QMutexLocker locker(&g_logMutex);
    return g_logLevel;
}

// 将QtMsgType转换为自定义LogLevel
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

// 自定义消息处理函数（核心）
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)

    // 1. 加锁保证线程安全
    QMutexLocker locker(&g_logMutex);

    // 2. 转换日志类型，判断是否需要记录
    LogLevel currentLevel = qtMsgTypeToLogLevel(type);
    LogLevel filterLevel = g_logLevel; // 先读取到局部变量

    // 3. 调试输出（测试时启用）
    // std::cout << "Filter: current=" << static_cast<int>(currentLevel)
    //           << ", filter=" << static_cast<int>(filterLevel) << std::endl;

    // 4. 判断是否需要记录（只有当前等级 >= 过滤等级才记录）
    if (static_cast<int>(currentLevel) < static_cast<int>(filterLevel))
    {
        return; // 不记录低于设置等级的日志
    }
    // 5. 拼接日志内容
    QString logTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logTypeStr;
    switch (type)
    {
    case QtDebugMsg:
        logTypeStr = "Debug";
        break;
    case QtInfoMsg:
        logTypeStr = "Info";
        break;
    case QtWarningMsg:
        logTypeStr = "Warning";
        break;
    case QtCriticalMsg:
        logTypeStr = "Critical";
        break;
    case QtFatalMsg:
        logTypeStr = "Fatal";
        break;
    default:
        logTypeStr = "Unknown";
        break;
    }
    QString txt = QString("[%1] [%2]: %3").arg(logTime).arg(logTypeStr).arg(msg);

    // 6. 输出到控制台（如果启用）
    LOG_TO_CONSOLE(txt);

    // 7. 写入日志文件
    QFile logFile(QT_LOG_FILE);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QTextStream textStream(&logFile);

        textStream << txt << Qt::endl;
        logFile.close();
    }
}
