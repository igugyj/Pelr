
#include "ExtraFileManager.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>

ExtraFileManager::ExtraFileManager(QObject *parent)
    : QObject(parent)
{
}

ExtraFileManager::~ExtraFileManager()
{
    _extraExpressions.clear();
    _extraMotions.clear();
}

void ExtraFileManager::loadExtraFiles(const QString &modelDir)
{
    _modelHomeDir = modelDir;
    _extraExpressions.clear();
    _extraMotions.clear();

    QDir dir(modelDir);
    if (!dir.exists())
    {
        qWarning() << "[ExtraMotions] Model directory does not exist:" << modelDir;
        return;
    }

    scanDirectory(dir);
    qDebug() << "[ExtraMotions] Loaded" << _extraExpressions.size() << "extra expressions and"
             << _extraMotions.size() << "extra motions";
}

void ExtraFileManager::scanDirectory(const QDir &dir)
{
    qDebug() << "[ExtraMotions] === Scanning directory:" << dir.absolutePath() << "===";
    // 获取目录下所有文件和子目录
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    qDebug() << "[ExtraMotions] Found" << entries.size() << "entries in directory";

    for (const QFileInfo &entry : entries)
    {
        if (entry.isDir())
        {
            qDebug() << "[ExtraMotions] Entering subdirectory:" << entry.fileName();
            scanDirectory(QDir(entry.absoluteFilePath()));
        }
        else if (entry.isFile())
        {
            QString filePath = entry.absoluteFilePath();
            QString fileName = entry.fileName().toLower(); // 转换为小写进行比较

            // qDebug() << "Checking file:" << fileName;

            // 使用文件名匹配而不是后缀匹配
            if (fileName.endsWith(".exp3.json"))
            {
                // qDebug() << "Loading as expression file:" << fileName;
                loadExpressionFile(filePath);
            }
            else if (fileName.endsWith(".motion3.json"))
            {
                // qDebug() << "Loading as motion file:" << fileName;
                loadMotionFile(filePath);
            }
            else
            {
                qDebug() << "[ExtraMotions] Skipping file (not .exp3.json or .motion3.json):" << fileName;
            }
        }
    }
}

void ExtraFileManager::loadExpressionFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "[ExtraMotions] Failed to open expression file:" << filePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    // 使用文件名（不含扩展名）作为键
    QFileInfo fileInfo(filePath);
    QString expressionName = fileInfo.baseName();

    _extraExpressions[expressionName] = data;
    // qDebug() << "Loaded extra expression:" << expressionName;
}

void ExtraFileManager::loadMotionFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "[ExtraMotions] Failed to open motion file:" << filePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    // 使用文件名（不含扩展名）作为键
    QFileInfo fileInfo(filePath);
    QString motionName = fileInfo.baseName();

    _extraMotions[motionName] = data;
    // qDebug() << "Loaded extra motion:" << motionName;
}

QVector<QString> ExtraFileManager::getAvailableExpressions() const
{
    QVector<QString> expressions;
    for (auto it = _extraExpressions.begin(); it != _extraExpressions.end(); ++it)
    {
        expressions.append(it.key());
    }
    return expressions;
}

QVector<QString> ExtraFileManager::getAvailableMotions() const
{
    QVector<QString> motions;
    for (auto it = _extraMotions.begin(); it != _extraMotions.end(); ++it)
    {
        motions.append(it.key());
    }
    return motions;
}

QByteArray ExtraFileManager::getExpressionData(const QString &expressionName) const
{
    return _extraExpressions.value(expressionName);
}

QByteArray ExtraFileManager::getMotionData(const QString &motionName) const
{
    return _extraMotions.value(motionName);
}
