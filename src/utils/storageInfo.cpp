#include "storageInfo.hpp"
#include "data.hpp"
#include <QCoreApplication>

qint64 StorageManager::getDirSize(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
    {
        qDebug() << "[StorageManager] Directory does not exist:" << dirPath;
        return 0;
    }

    qint64 total = 0;
    QDirIterator it(dirPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        total += it.fileInfo().size();
    }
    return total;
}

StorageInfoData StorageManager::getStorageInfo()
{
    StorageInfoData data;
    const auto &constData = DataManager::instance().const_config_data;
    QString basePath = QCoreApplication::applicationDirPath();

    // ── resolve absolute paths ──
    data.resourcesPath = QDir(basePath).absoluteFilePath("Resources");
    data.ttsCachePath = QDir(basePath).absoluteFilePath(constData.VoiceFolder);
    data.logPath = QDir(basePath).absoluteFilePath(constData.logFolder);
    data.userDataPath = QDir(basePath).absoluteFilePath(constData.userFolder);

    // voicevox_core model directory: parent of the configured .vvm file
    TTSConfig tts = DataManager::instance().getTTSConfig();
    if (!tts.voicevox_model.isEmpty())
    {
        QFileInfo vvmFile(tts.voicevox_model);
        if (vvmFile.exists())
            data.voicevoxCorePath = vvmFile.absolutePath();
        else
            qWarning() << "[StorageManager] voicevox_model file not found:" << tts.voicevox_model;
    }

    // ── calculate sizes ──
    data.resourcesSize = getDirSize(data.resourcesPath);
    data.voicevoxCoreSize = data.voicevoxCorePath.isEmpty() ? 0 : getDirSize(data.voicevoxCorePath);
    data.ttsCacheSize = getDirSize(data.ttsCachePath);
    data.logSize = getDirSize(data.logPath);
    data.userDataSize = getDirSize(data.userDataPath);

    // Resources contains only Live2D; voicevox_core is a sibling directory
    data.live2dSize = data.resourcesSize;

    data.runningDirSize = getDirSize(basePath);
    data.totalAppSize = data.runningDirSize;

    // ── disk info ──
    QStorageInfo storage(basePath);
    data.diskTotal = storage.bytesTotal();
    data.diskAvailable = storage.bytesAvailable();

    /*
    qDebug() << "[StorageManager]"
             << "Resources=" << data.resourcesSize
             << "Live2D=" << data.live2dSize
             << "voicevox_core=" << data.voicevoxCoreSize
             << "TTS cache=" << data.ttsCacheSize
             << "Logs=" << data.logSize
             << "User=" << data.userDataSize
             << "Total=" << data.totalAppSize
             << "Disk=" << data.diskTotal
             << "Available=" << data.diskAvailable;
    */
    return data;
}

bool StorageManager::clearDirContents(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
    {
        qWarning() << "[StorageManager] Cannot clear, directory not found:" << dirPath;
        return false;
    }

    qDebug() << "[StorageManager] Clearing directory:" << dirPath;
    if (!dir.removeRecursively())
    {
        qWarning() << "[StorageManager] Failed to remove:" << dirPath;
        return false;
    }

    if (!dir.mkpath("."))
    {
        qWarning() << "[StorageManager] Failed to recreate:" << dirPath;
        return false;
    }

    qDebug() << "[StorageManager] Cleared:" << dirPath;
    return true;
}
