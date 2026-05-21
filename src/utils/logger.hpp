
#pragma once
#include <QString>
#include <QMutex>
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QMessageBox>
#include <QString>
#define LOG_LEVEL_FILE "user/logLevel.dat"

// 定义日志等级枚举
enum class LogLevel
{
    Debug = 0,
    Info = 1,
    Warning = 2,
    Critical = 3,
    Fatal = 4
};

// 声明外部变量
extern LogLevel g_logLevel;
extern QMutex g_logMutex;

// 声明函数
void initLogFile();

void setLogLevel(LogLevel level);

LogLevel getLogLevel();

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

inline LogLevel read_log_level()
{
    QFile file(LOG_LEVEL_FILE);
    LogLevel level;
    if (!file.open(QIODevice::ReadOnly))
    {
        // 返回默认日志等级
        return LogLevel::Debug;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0); // 设置流版本以确保兼容性
    in >> level;
    file.close();
    return level;
}

inline void write_log_level(const LogLevel level)
{
    setLogLevel(level);
    QFile file(LOG_LEVEL_FILE);
    if (!file.open(QIODevice::WriteOnly))
    {
        // 无法打开文件进行写入
        QMessageBox::critical(nullptr, "Error", "写入数据失败！");
        qCritical() << "write data failed: can not open file" << LOG_LEVEL_FILE;
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_15); // 设置流版本以确保兼容性

    // 写入数据
    out << level;
    file.close();
}
