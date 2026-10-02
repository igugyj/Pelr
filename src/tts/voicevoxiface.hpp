#pragma once

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QtPlugin>

#include "ttsconfig.hpp"

// 主 exe 与 voicevox 插件之间的纯 Qt 边界。
// 这里绝不能出现 voicevox_core.h 或任何 voicevox 类型。
class IVoicevoxTts
{
public:
    struct StyleInfo
    {
        int id = 0;
        QString name;
    };
    struct SpeakerInfo
    {
        QString name;
        QString uuid;
        QString version;
        QVector<StyleInfo> styles;
    };

    virtual ~IVoicevoxTts() = default;

    virtual bool initialize(const QString &dictDir) = 0;
    virtual bool loadModel(const QString &modelPath) = 0;
    virtual bool applyConfig(const TTSConfig &config) = 0;
    virtual QByteArray synthesize(const QString &text, int styleId, double speed) = 0;
    virtual QVector<SpeakerInfo> getSpeakers() const = 0;
    virtual QVector<int> getStyleIds() const = 0;
    virtual bool isReady() const = 0;
    virtual void unloadModel() = 0;
};

Q_DECLARE_INTERFACE(IVoicevoxTts, "com.pelr.IVoicevoxTts/1")
