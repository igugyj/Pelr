#include "voicevox_tts.h"
#include "voicevox_core.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <mutex>
#include <QString>
#include <QtWidgets/QApplication>
#include <QCoreApplication>

// ============ 全局 ONNX Runtime ============
static const VoicevoxOnnxruntime *g_onnx = nullptr;
static std::once_flag g_onnxInitFlag;

bool VoicevoxTTS::initializeOnnxRuntime(const QString &onnxPath)
{
    std::call_once(g_onnxInitFlag, [&]()
                   {
        if (!onnxPath.isEmpty() && !QFileInfo::exists(onnxPath)) {
            qWarning() << "[VoicevoxTTS] ONNX Runtime library not found:" << onnxPath;
            return;
        }
        VoicevoxLoadOnnxruntimeOptions opts = voicevox_make_default_load_onnxruntime_options();
        if (!onnxPath.isEmpty()) opts.filename = onnxPath.toUtf8().constData();
        VoicevoxResultCode rc = voicevox_onnxruntime_load_once(opts, &g_onnx);
        if (rc != VOICEVOX_RESULT_OK) {
            qWarning() << "[VoicevoxTTS] Failed to load ONNX Runtime, error code:" << rc;
            g_onnx = nullptr;
        } else {
            qDebug() << "[VoicevoxTTS] ONNX Runtime initialized successfully";
        } });
    return g_onnx != nullptr;
}

// ============ 单例 ============
VoicevoxTTS &VoicevoxTTS::instance()
{
    static VoicevoxTTS s;
    return s;
}

// ============ Pimpl ============
struct VoicevoxTTS::Impl
{
    OpenJtalkRc *openJtalk = nullptr;
    VoicevoxSynthesizer *synthesizer = nullptr;
    VoicevoxVoiceModelFile *model = nullptr;
    bool ready = false;

    QString currentDictDir;
    QString currentModelPath;
    uint8_t currentModelId[16] = {0};
    bool hasModelId = false;

    ~Impl()
    {
        unloadModel();
        if (synthesizer)
        {
            voicevox_synthesizer_delete(synthesizer);
            synthesizer = nullptr;
        }
        if (openJtalk)
        {
            voicevox_open_jtalk_rc_delete(openJtalk);
            openJtalk = nullptr;
        }
    }

    void unloadModel()
    {
        if (synthesizer && hasModelId)
        {
            // 正确转换为 VoicevoxVoiceModelId (const uint8_t(*)[16])
            auto modelIdPtr = const_cast<const uint8_t (*)[16]>(&currentModelId);
            VoicevoxResultCode rc = voicevox_synthesizer_unload_voice_model(
                synthesizer,
                reinterpret_cast<VoicevoxVoiceModelId>(modelIdPtr));
            if (rc != VOICEVOX_RESULT_OK)
            {
                qWarning() << "[VoicevoxTTS] Failed to unload voice model, code:" << rc;
            }
            else
            {
                qDebug() << "[VoicevoxTTS] Unloaded previous model successfully";
            }
            hasModelId = false;
        }
        if (model)
        {
            voicevox_voice_model_file_delete(model);
            model = nullptr;
        }
        ready = false;
        currentModelPath.clear();
    }
};
VoicevoxTTS::VoicevoxTTS() : d(std::make_unique<Impl>()) {}
VoicevoxTTS::~VoicevoxTTS() = default;

// ============ 内部初始化/加载 ============
bool VoicevoxTTS::initialize(const QString &dictDir)
{
    if (!g_onnx)
    {
        qWarning() << "[VoicevoxTTS] ONNX Runtime not initialized";
        return false;
    }
    if (dictDir.isEmpty() || !QDir(dictDir).exists())
    {
        qWarning() << "[VoicevoxTTS] Dictionary directory not found:" << dictDir;
        return false;
    }
    // 清理旧对象
    d->unloadModel();
    if (d->synthesizer)
    {
        voicevox_synthesizer_delete(d->synthesizer);
        d->synthesizer = nullptr;
    }
    if (d->openJtalk)
    {
        voicevox_open_jtalk_rc_delete(d->openJtalk);
        d->openJtalk = nullptr;
    }

    QByteArray dictDirBytes = dictDir.toUtf8();
    VoicevoxResultCode rc = voicevox_open_jtalk_rc_new(dictDirBytes.constData(), &d->openJtalk);
    if (rc != VOICEVOX_RESULT_OK)
    {
        qWarning() << "[VoicevoxTTS] OpenJTalk init failed, code:" << rc;
        return false;
    }
    VoicevoxInitializeOptions initOpts = voicevox_make_default_initialize_options();
    rc = voicevox_synthesizer_new(g_onnx, d->openJtalk, initOpts, &d->synthesizer);
    if (rc != VOICEVOX_RESULT_OK)
    {
        qWarning() << "[VoicevoxTTS] Synthesizer create failed, code:" << rc;
        return false;
    }
    d->currentDictDir = dictDir;
    qDebug() << "[VoicevoxTTS] Synthesizer created with dict:" << dictDir;
    return true;
}
bool VoicevoxTTS::loadModel(const QString &modelPath)
{
    if (!d->synthesizer)
    {
        qWarning() << "[VoicevoxTTS] Synthesizer not ready, call initialize() first";
        return false;
    }
    if (modelPath.isEmpty() || !QFileInfo::exists(modelPath))
    {
        qWarning() << "[VoicevoxTTS] Model file not found:" << modelPath;
        return false;
    }

    // 如果已加载相同路径的模型，直接成功（但可加选项强制重载，此处保持原行为）
    if (d->ready && d->currentModelPath == modelPath)
    {
        qDebug() << "[VoicevoxTTS] Model already loaded:" << modelPath;
        return true;
    }

    qDebug() << "[VoicevoxTTS] Unloading current model (if any) before loading new one";
    d->unloadModel(); // 卸载旧模型（包括从合成器中移除）

    QByteArray modelPathBytes = modelPath.toUtf8();
    VoicevoxResultCode rc = voicevox_voice_model_file_open(modelPathBytes.constData(), &d->model);
    if (rc != VOICEVOX_RESULT_OK || !d->model)
    {
        qWarning() << "[VoicevoxTTS] Open model failed, code:" << rc;
        return false;
    }

    // 获取新模型的唯一ID（用于将来卸载）
    voicevox_voice_model_file_id(d->model, &d->currentModelId);
    d->hasModelId = true;
    qDebug() << "[VoicevoxTTS] New model ID obtained";

    rc = voicevox_synthesizer_load_voice_model(d->synthesizer, d->model);
    if (rc != VOICEVOX_RESULT_OK)
    {
        qWarning() << "[VoicevoxTTS] Load voice model failed, code:" << rc;
        voicevox_voice_model_file_delete(d->model);
        d->model = nullptr;
        d->hasModelId = false;
        return false;
    }

    d->ready = true;
    d->currentModelPath = modelPath;
    qDebug() << "[VoicevoxTTS] Model loaded successfully:" << modelPath;
    return true;
}

// ============ 公共配置应用 ============
bool VoicevoxTTS::applyConfig(const TTSConfig &config)
{
    if (config.provider != 2)
        return false; // 不是 VOICEVOX 配置，直接返回失败

    // 检查并初始化 ONNX Runtime（必须最先调用）
    if (!g_onnx)
    {
        // 尝试使用默认路径加载（可以在设置中指定 onnx 路径，这里暂时用空自动查找）
        if (!initializeOnnxRuntime())
        {
            qWarning() << "[VoicevoxTTS] ONNX Runtime unavailable";
            return false;
        }
    }

    // 检查辞书路径是否变化，若变化则重新初始化
    if (d->currentDictDir != config.voicevox_dict_dir)
    {
        qDebug() << "[VoicevoxTTS] Dict dir changed, reinitializing...";
        if (!initialize(config.voicevox_dict_dir))
        {
            qWarning() << "[VoicevoxTTS] Failed to initialize with new dict";
            return false;
        }
        // 初始化后必须重新加载模型（因为合成器已重建）
        if (!config.voicevox_model.isEmpty())
        {
            if (!loadModel(config.voicevox_model))
            {
                qWarning() << "[VoicevoxTTS] Failed to load model after init";
                return false;
            }
        }
    }
    else
    {
        // 辞书相同，只需检查模型路径
        if (d->currentModelPath != config.voicevox_model)
        {
            if (!config.voicevox_model.isEmpty())
            {
                if (!loadModel(config.voicevox_model))
                {
                    qWarning() << "[VoicevoxTTS] Failed to load new model";
                    return false;
                }
            }
        }
    }
    // 如果模型尚未加载，尝试加载
    if (!d->ready && !config.voicevox_model.isEmpty())
    {
        if (!loadModel(config.voicevox_model))
        {
            return false;
        }
    }
    return isReady();
}

// 在 synthesizeToFile 实现之后追加
QVector<int> VoicevoxTTS::getStyleIds() const
{
    QVector<int> ids;
    const auto speakers = getSpeakers();
    for (const auto &speaker : speakers)
    {
        for (const auto &style : speaker.styles)
        {
            ids.append(style.id);
        }
    }
    return ids;
}

// 原有公共方法保持不变（synthesis, getSpeakers 等）

QVector<VoicevoxTTS::SpeakerInfo> VoicevoxTTS::getSpeakers() const
{
    QVector<SpeakerInfo> result;
    if (!d->synthesizer)
        return result;

    char *json = voicevox_synthesizer_create_metas_json(d->synthesizer);
    if (!json)
    {
        qWarning() << "[VoicevoxTTS] Failed to get speaker metas JSON";
        return result;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    voicevox_json_free(json);

    if (error.error != QJsonParseError::NoError || !doc.isArray())
    {
        qWarning() << "[VoicevoxTTS] Failed to parse metas JSON:" << error.errorString();
        return result;
    }

    const QJsonArray arr = doc.array();
    for (const QJsonValue &val : arr)
    {
        if (!val.isObject())
            continue;
        QJsonObject obj = val.toObject();
        SpeakerInfo spk;
        spk.name = obj["name"].toString();
        spk.uuid = obj["speaker_uuid"].toString();
        spk.version = obj["version"].toString();

        const QJsonArray stylesArr = obj["styles"].toArray();
        for (const QJsonValue &sVal : stylesArr)
        {
            if (!sVal.isObject())
                continue;
            QJsonObject sObj = sVal.toObject();
            StyleInfo st;
            st.id = sObj["id"].toInt();
            st.name = sObj["name"].toString();
            spk.styles.append(st);
        }
        result.append(spk);
    }
    return result;
}

QByteArray VoicevoxTTS::synthesis(const QString &text, int styleId)
{
    if (!d->ready)
    {
        qWarning() << "[VoicevoxTTS] TTS not ready (must initialize + loadModel)";
        return QByteArray();
    }

    QByteArray textUtf8 = text.toUtf8();
    VoicevoxTtsOptions ttsOpts = voicevox_make_default_tts_options();

    uintptr_t wavSize = 0;
    uint8_t *wav = nullptr;
    VoicevoxResultCode rc = voicevox_synthesizer_tts(d->synthesizer, textUtf8.constData(),
                                                     static_cast<VoicevoxStyleId>(styleId),
                                                     ttsOpts, &wavSize, &wav);
    if (rc != VOICEVOX_RESULT_OK)
    {
        qWarning() << "[VoicevoxTTS] Speech synthesis failed, error code:" << rc;
        return QByteArray();
    }

    QByteArray data(reinterpret_cast<const char *>(wav), static_cast<int>(wavSize));
    voicevox_wav_free(wav);
    qDebug() << "[VoicevoxTTS] Synthesis completed, WAV size:" << wavSize << "bytes";
    return data;
}

bool VoicevoxTTS::isReady() const { return d->ready; }
void VoicevoxTTS::unloadModel() { d->unloadModel(); }

QByteArray VoicevoxTTS::testSynthesis(const TTSConfig &config)
{
    // 应用配置（如果需要切换模型/辞书）
    if (!applyConfig(config))
    {
        qWarning() << "[VoicevoxTTS] testSynthesis: applyConfig failed";
        return {};
    }
    const QString testText = QString::fromUtf8(u8"こんにちは、テストです。");
    int styleId = config.voicevox_style_id; // 从配置读取
    return synthesis(testText, styleId);
}

QByteArray VoicevoxTTS::testSynthesis()
{
    // 无参版本：使用内部存储的配置？这里简单回退到 style 0
    if (!isReady())
    {
        qWarning() << "[VoicevoxTTS] testSynthesis: not ready";
        return {};
    }
    return synthesis(QString::fromUtf8(u8"こんにちは、テストです。"), 0);
}

QString VoicevoxTTS::synthesizeToFile(const TTSConfig &config, const QString &text,
                                      int styleId, double speed)
{
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

    QByteArray wav = synthesis(text, styleId);
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