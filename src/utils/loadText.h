#pragma once
/*
PROJECT_NAME Pelr
PRODUCT_NAME CLion
NAME loadText
AUTHOR Pfolg
TIME 2026/3/20 21:56
*/

#include <QFile>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDir>
#include <QRandomGenerator>
#include <QFileInfo>
#include "launcher.hpp"
#include "data.hpp"

// 辅助函数：检查文件是否存在
static bool checkFileExists(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists())
    {
        qWarning() << "[TextLoader] File not found:" << filePath;
        return false;
    }
    return true;
}

// 辅助函数：从指定文件中按 key 获取随机字符串
static QString getRandomTextFromFile(const QString &filePath, const QString &keyName)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "[TextLoader] Cannot open file:" << filePath << "-" << file.errorString();
        return QString();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull())
    {
        qWarning() << "[TextLoader] JSON parse failed for file:" << filePath;
        return QString();
    }

    QJsonObject root = doc.object();
    if (!root.contains(keyName) || !root[keyName].isArray())
    {
        qWarning() << "[TextLoader] JSON format error or missing key '" << keyName << "' in file:" << filePath;
        return QString();
    }

    QJsonArray targetArray = root[keyName].toArray();
    if (targetArray.isEmpty())
    {
        qWarning() << "[TextLoader] Array for key '" << keyName << "' is empty in file:" << filePath;
        return QString();
    }

    // Qt5 随机数生成器
    const int index = QRandomGenerator::global()->bounded(targetArray.size());
    return targetArray[index].toString();
}

// 主要加载函数：优先使用 user 文件，其次默认文件，最后硬编码默认语句
static QString loadText(const QString &keyName)
{
    const QString fallbackText = "Hello World!";
    QString _auser = FilePaths.userTextFile;
    QString _bdefault = FilePaths.defaultTextFile;
    // 1. 优先尝试 user 文件
    if (checkFileExists(_auser))
    {
        QString text = getRandomTextFromFile(_auser, keyName);
        if (!text.isEmpty())
        {
            return text;
        }
        qWarning() << "[TextLoader] User file is invalid, fallback to default file.";
    }

    // 2. 尝试默认文件
    if (checkFileExists(_bdefault))
    {
        QString text = getRandomTextFromFile(_bdefault, keyName);
        if (!text.isEmpty())
        {
            return text;
        }
        qWarning() << "[TextLoader] Default file is invalid or missing key.";
    }

    // 3. 都失败则返回硬编码默认语句
    qWarning() << "[TextLoader] No valid text source for key '" << keyName << "', using fallback.";
    return fallbackText;
}

// 初始化用户文件：若文件已存在则直接打开，否则复制默认文件后打开
// 注意：openFileForEditing 是外部已实现的打开文件的函数，需要用户自行提供
static void initUserTextFile()
{

    QString _auser = FilePaths.userTextFile;
    QString _bdefault = FilePaths.defaultTextFile;
    // 确保 user 目录存在
    QDir dir;
    if (!dir.exists("user"))
    {
        if (!dir.mkpath("user"))
        {
            qCritical() << "[TextLoader] Failed to create user directory.";
            return;
        }
    }

    // 检查 user 文件是否存在
    bool fileExists = QFile::exists(_auser);

    if (!fileExists)
    {
        // 复制默认文件
        if (QFile::exists(_bdefault))
        {
            if (!QFile::copy(_bdefault, _auser))
            {
                qCritical() << "[TextLoader] Failed to copy default file to user directory.";
                return;
            }
            qDebug() << "[TextLoader] User text file created at" << _auser;
        }
        else
        {
            qCritical() << "[TextLoader] Default text file not found, cannot initialize user file.";
            return;
        }
    }
    else
    {
        qDebug() << "[TextLoader] User text file already exists, opening directly.";
    }

    // 调用外部打开文件的模块
    launchByPath(_auser);
    qDebug() << "[TextLoader] Please open file for editing:" << _auser;
}
