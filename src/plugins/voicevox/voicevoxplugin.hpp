#pragma once

#include <memory>

#include <QObject>

#include "voicevoxiface.hpp"

// 唯一接触 voicevox_core.h 的编译单元（实现见 .cpp）。
class VoicevoxPlugin : public QObject, public IVoicevoxTts
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.pelr.IVoicevoxTts/1")
    Q_INTERFACES(IVoicevoxTts)

public:
    explicit VoicevoxPlugin(QObject *parent = nullptr);
    ~VoicevoxPlugin() override;

    bool initialize(const QString &dictDir) override;
    bool loadModel(const QString &modelPath) override;
    bool applyConfig(const TTSConfig &config) override;
    QByteArray synthesize(const QString &text, int styleId, double speed) override;
    QVector<SpeakerInfo> getSpeakers() const override;
    QVector<int> getStyleIds() const override;
    bool isReady() const override;
    void unloadModel() override;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};
