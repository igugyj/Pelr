#include "voicevox_tts.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QMutex>

#include "data.hpp"
#include "voicevoxhost.hpp"

namespace
{
IVoicevoxTts *backend()
{
    return VoicevoxHost::acquire();
}
} // namespace

VoicevoxTTS &VoicevoxTTS::instance()
{
    static VoicevoxTTS s;
    return s;
}

bool VoicevoxTTS::initialize(const QString &dictDir)
{
    auto *p = backend();
    if (!p)
    {
        qWarning() << "[VoicevoxTTS] voicevox plugin unavailable";
        return false;
    }
    return p->initialize(dictDir);
}

bool VoicevoxTTS::loadModel(const QString &modelPath)
{
    auto *p = backend();
    if (!p)
    {
        qWarning() << "[VoicevoxTTS] voicevox plugin unavailable";
        return false;
    }
    return p->loadModel(modelPath);
}

bool VoicevoxTTS::applyConfig(const TTSConfig &config)
{
    auto *p = backend();
    if (!p)
    {
        qWarning() << "[VoicevoxTTS] voicevox plugin unavailable";
        return false;
    }
    return p->applyConfig(config);
}

QVector<VoicevoxTTS::SpeakerInfo> VoicevoxTTS::getSpeakers() const
{
    auto *p = backend();
    return p ? p->getSpeakers() : QVector<SpeakerInfo>{};
}

QVector<int> VoicevoxTTS::getStyleIds() const
{
    auto *p = backend();
    return p ? p->getStyleIds() : QVector<int>{};
}

QByteArray VoicevoxTTS::synthesis(const QString &text, int styleId, double speed)
{
    auto *p = backend();
    if (!p)
    {
        qWarning() << "[VoicevoxTTS] voicevox plugin unavailable";
        return QByteArray();
    }
    return p->synthesize(text, styleId, speed);
}

bool VoicevoxTTS::isReady() const
{
    auto *p = backend();
    return p && p->isReady();
}

void VoicevoxTTS::unloadModel()
{
    if (auto *p = backend())
        p->unloadModel();
}

QString VoicevoxTTS::synthesizeToFile(const TTSConfig &config, const QString &text,
                                      int styleId, double speed)
{
    // 插件化前这里锁的是与 Impl 共享的同一把递归锁，串行了整个文件写入。
    // 插件有自己的锁覆盖其状态，这把锁只保留文件 I/O 的串行语义。
    QMutexLocker locker(&m_mutex);

    // 确保配置已生效
    if (!applyConfig(config))
    {
        qWarning() << "[VoicevoxTTS] synthesizeToFile: applyConfig failed";
        return {};
    }

    // 构建哈希文件名：模型路径 + 文本 + speed + styleId
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(config.voicevox_model.toUtf8());
    hash.addData(text.toUtf8());
    hash.addData(QByteArray::number(speed, 'f', 2));
    hash.addData(QByteArray::number(styleId));
    QString hashName = hash.result().toHex().left(16);

    QString dirPath = DataManager::instance().const_config_data.VoiceFolder;
    QString filePath = dirPath + "/" + hashName + ".wav";

    // 缓存命中则直接返回已有文件
    if (QFile::exists(filePath))
    {
        qDebug() << "[VoicevoxTTS] Reusing cached:" << filePath;
        return filePath;
    }

    QByteArray wav = synthesis(text, styleId, speed);
    if (wav.isEmpty())
    {
        qWarning() << "[VoicevoxTTS] synthesizeToFile: synthesis empty";
        return {};
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "[VoicevoxTTS] synthesizeToFile: cannot write" << filePath;
        return {};
    }
    file.write(wav);
    file.close();
    return filePath;
}
