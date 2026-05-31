
#pragma once

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QStorageInfo>
#include <QDebug>

struct StorageInfoData
{
    // absolute paths
    QString resourcesPath;
    QString voicevoxCorePath;
    QString ttsCachePath;
    QString logPath;
    QString userDataPath;

    // sizes in bytes
    qint64 resourcesSize = 0;
    qint64 voicevoxCoreSize = 0;
    qint64 live2dSize = 0;
    qint64 ttsCacheSize = 0;
    qint64 logSize = 0;
    qint64 userDataSize = 0;
    qint64 totalAppSize = 0;
    qint64 runningDirSize = 0;   // total size of the app's directory

    // disk info
    qint64 diskTotal = 0;
    qint64 diskAvailable = 0;

    QString formatSize(qint64 bytes) const
    {
        if (bytes < 1024)
            return QString::number(bytes) + " B";
        if (bytes < 1024 * 1024)
            return QString::number(bytes / 1024.0, 'f', 1) + " KB";
        if (bytes < 1024LL * 1024 * 1024)
            return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
        return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }

    double usagePercent() const
    {
        if (diskTotal == 0) return 0.0;
        return 100.0 * runningDirSize / diskTotal;
    }
};

class StorageManager
{
public:
    static qint64 getDirSize(const QString &dirPath);
    static StorageInfoData getStorageInfo();
    static bool clearDirContents(const QString &dirPath);

private:
    StorageManager() = default;
};
