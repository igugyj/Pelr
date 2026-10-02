#include "voicevoxplugin.hpp"

#include "voicevox_core.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <mutex>

#include "componentpaths.hpp"

// ============ 全局 ONNX Runtime ============
static const VoicevoxOnnxruntime *g_onnx = nullptr;
static std::once_flag g_onnxInitFlag;

static bool ensureOnnxRuntime(const QString &onnxPath = QString())
{
    std::call_once(g_onnxInitFlag, [&]()
                   {
        // 官方包把 onnxruntime 放在 onnxruntime/lib/，与 voicevox_core.dll 不同目录。
        // voicevox_core.dll 的导入表并不含 voicevox_onnxruntime.dll（它是靠内嵌
        // 字符串动态 LoadLibrary 的），所以没有跨目录导入依赖，但必须把绝对路径
        // 写进 options.filename，否则 voicevox_core 只会去默认搜索路径里找。
        QString path = onnxPath;
        if (path.isEmpty())
        {
            const QStringList candidates = ComponentPaths::voicevoxOnnxDllCandidates();
            for (const QString &cand : candidates)
            {
                if (QFileInfo::exists(cand)) { path = cand; break; }
            }
            if (path.isEmpty())
            {
                qWarning().noquote() << "[VoicevoxTTS] voicevox_onnxruntime.dll not found, tried:"
                                     << candidates;
                return;
            }
        }
        if (!QFileInfo::exists(path))
        {
            qWarning() << "[VoicevoxTTS] ONNX Runtime library not found:" << path;
            return;
        }

        VoicevoxLoadOnnxruntimeOptions opts = voicevox_make_default_load_onnxruntime_options();
        const QByteArray onnxPathUtf8 = path.toUtf8();
        opts.filename = onnxPathUtf8.constData();

        VoicevoxResultCode rc = voicevox_onnxruntime_load_once(opts, &g_onnx);
        if (rc != VOICEVOX_RESULT_OK) {
            // 文件在却加载不动：两个 voicevox DLL 都由 MSVC 构建，依赖
            // MSVCP140/VCRUNTIME140，纯净 Windows 未装 VC++ Redistributable 时
            // 症状正是如此。
            qWarning() << "[VoicevoxTTS] Failed to load ONNX Runtime, error code:" << rc
                       << "- file exists at" << path
                       << ";on a clean machine this is usually a missing Microsoft"
                       << "Visual C++ Redistributable";
            g_onnx = nullptr;
        } else {
            qDebug() << "[VoicevoxTTS] ONNX Runtime initialized successfully" << path;
        } });
    return g_onnx != nullptr;
}

// ============ Pimpl ============
struct VoicevoxPlugin::Impl
{
    mutable QRecursiveMutex mutex;
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

VoicevoxPlugin::VoicevoxPlugin(QObject *parent)
    : QObject(parent), d(std::make_unique<Impl>())
{
}
VoicevoxPlugin::~VoicevoxPlugin() = default;

// ============ 内部初始化/加载 ============
bool VoicevoxPlugin::initialize(const QString &dictDir)
{
    QMutexLocker locker(&d->mutex);
    // 插件自包含：不再依赖宿主进程启动期显式调用。call_once 幂等，
    // 与 applyConfig() 内的调用重复触发也安全。
    if (!ensureOnnxRuntime())
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

bool VoicevoxPlugin::loadModel(const QString &modelPath)
{
    QMutexLocker locker(&d->mutex);
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

    VoicevoxLoadVoiceModelOptions loadOpts = voicevox_make_default_load_voice_model_options();
    // 可选：根据需求设置重复模型加载策略
    // loadOpts.on_existing = VOICEVOX_ON_EXISTING_VOICE_MODEL_ID_RELOAD; // 重新加载并释放旧内存
    // loadOpts.on_existing = VOICEVOX_ON_EXISTING_VOICE_MODEL_ID_SKIP;   // 跳过，不重复加载
    // loadOpts.on_existing = VOICEVOX_ON_EXISTING_VOICE_MODEL_ID_ERROR;  // 默认，报错
    rc = voicevox_synthesizer_load_voice_model(d->synthesizer, d->model, loadOpts);
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
bool VoicevoxPlugin::applyConfig(const TTSConfig &config)
{
    QMutexLocker locker(&d->mutex);
    if (config.provider != 2)
        return false; // 不是 VOICEVOX 配置，直接返回失败

    // 检查并初始化 ONNX Runtime（必须最先调用）
    if (!g_onnx)
    {
        // 尝试使用默认路径加载（可以在设置中指定 onnx 路径，这里暂时用空自动查找）
        if (!ensureOnnxRuntime())
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

QVector<int> VoicevoxPlugin::getStyleIds() const
{
    QMutexLocker locker(&d->mutex);
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

QVector<VoicevoxPlugin::SpeakerInfo> VoicevoxPlugin::getSpeakers() const
{
    QMutexLocker locker(&d->mutex);
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

QByteArray VoicevoxPlugin::synthesize(const QString &text, int styleId, double speed)
{
    QMutexLocker locker(&d->mutex);
    if (!d->ready)
    {
        qWarning() << "[VoicevoxTTS] TTS not ready (must initialize + loadModel)";
        return QByteArray();
    }

    QByteArray textUtf8 = text.toUtf8();

    // 生成 AudioQuery 并调整 speedScale（speed > 0 时生效）
    char *queryJson = nullptr;
    VoicevoxResultCode rc = voicevox_synthesizer_create_audio_query(
        d->synthesizer, textUtf8.constData(),
        static_cast<VoicevoxStyleId>(styleId), &queryJson);
    if (rc != VOICEVOX_RESULT_OK)
    {
        qWarning() << "[VoicevoxTTS] Failed to create audio query, error code:" << rc;
        return QByteArray();
    }
    QByteArray query(queryJson);
    voicevox_json_free(queryJson);

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(query, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning() << "[VoicevoxTTS] Failed to parse audio query:" << parseError.errorString();
        return QByteArray();
    }
    QJsonObject obj = doc.object();
    if (speed > 0)
    {
        obj["speedScale"] = speed;
    }
    QByteArray queryBytes = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    VoicevoxSynthesisOptions synOpts = voicevox_make_default_synthesis_options();
    uintptr_t wavSize = 0;
    uint8_t *wav = nullptr;
    rc = voicevox_synthesizer_synthesis(d->synthesizer, queryBytes.constData(),
                                        static_cast<VoicevoxStyleId>(styleId),
                                        synOpts, &wavSize, &wav);
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

bool VoicevoxPlugin::isReady() const
{
    QMutexLocker locker(&d->mutex);
    return d->ready;
}

void VoicevoxPlugin::unloadModel()
{
    QMutexLocker locker(&d->mutex);
    d->unloadModel();
}
