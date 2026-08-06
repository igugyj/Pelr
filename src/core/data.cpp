#include "data.hpp"
#include <QSaveFile>
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QJsonValue>
#include <QMap>
#include <QStringList>
#include <QFileInfo>
#include <QSysInfo>
#include <QtConcurrent>
#include <atomic>

bool DataManager::writeJsonFile(const QString &filePath, const QJsonDocument &doc)
{
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << "[Data] Cannot open file for writing:" << filePath;
        return false;
    }
    file.write(doc.toJson());
    if (!file.commit())
    {
        qCritical() << "[Data] Failed to commit file:" << filePath;
        return false;
    }
    return true;
}

// ============================================================================
// H15: 菜单数据 HMAC 签名 / 验签
// 单文件 user/.menuSig（明文 JSON）内含 jsonSalt、json 摘要（HMAC-SHA256，
// 密钥由机器标识派生，无定值）、各条目 salt + SHA-256 文件摘要（网页链接与文件夹跳过）。
// 防护级别：防"随手改文件"；密钥可被本机进程重建（纯 Qt 路线的固有边界）。
// ============================================================================

namespace
{
bool constantTimeEqual(const QByteArray &a, const QByteArray &b)
{
    if (a.size() != b.size())
        return false;
    bool ok = true;
    for (int i = 0; i < a.size(); ++i)
        ok = ok && (a.at(i) == b.at(i));
    return ok;
}

bool isUrlPath(const QString &path)
{
    return path.startsWith("http://") || path.startsWith("https://") ||
           path.startsWith("ftp://") || path.startsWith("file://");
}

// 跨版本稳定契约：菜单签名密钥派生规则（机器标识），改动需 bump payload v + 一次重签过渡
QByteArray menuHmacKey()
{
    static const QByteArray key = []() {
        // 每台机器唯一：MachineGuid（Windows 注册表）；空（极端情况）回退主机名
        QString seed = QSysInfo::machineUniqueId();
        if (seed.isEmpty())
            seed = QSysInfo::machineHostName();
        return QCryptographicHash::hash(QByteArrayLiteral("Pelr-MenuSig-v1|") + seed.toUtf8(),
                                        QCryptographicHash::Sha256);
    }();
    return key;
}

// 签名代际计数：快速连续保存时只落最新一份，防止旧任务覆盖新签名
std::atomic<int> g_sigGeneration{0};
} // namespace

void DataManager::signMenuData(const QList<MenuData> &items, int gen)
{
    QByteArray jsonBytes;
    {
        QFile f(FilePaths.menuDataFile); // 块作用域：读完即关，避免句柄占用影响后续写入
        if (!f.open(QIODevice::ReadOnly))
        {
            qCritical() << "[Data] signMenuData: cannot read" << FilePaths.menuDataFile;
            return;
        }
        jsonBytes = f.readAll();
    }
    const QByteArray key = menuHmacKey();

    QByteArray jsonSalt;
    {
        QFile sigFile(FilePaths.menuSigFile); // 块作用域：读完即关，否则 QSaveFile 覆盖会被只读句柄阻塞
        if (sigFile.open(QIODevice::ReadOnly))
        {
            QJsonObject obj = QJsonDocument::fromJson(sigFile.readAll()).object();
            jsonSalt = QByteArray::fromBase64(obj.value("jsonSalt").toString().toUtf8());
        }
    }
    if (jsonSalt.size() != 16)
    {
        jsonSalt.resize(16);
        for (int i = 0; i < 16; ++i)
            jsonSalt[i] = static_cast<char>(QRandomGenerator::system()->bounded(256));
    }

    const QByteArray jsonHmac = QMessageAuthenticationCode::hash(jsonSalt + jsonBytes,
                                                                 key, QCryptographicHash::Sha256);

    QJsonArray filesArr;
    for (const MenuData &item : items)
    {
        if (item.path.isEmpty() || isUrlPath(item.path))
            continue;
        QFileInfo fi(item.path);
        if (fi.isDir())
            continue; // 文件夹不校验（与网页链接同待遇）

        QByteArray salt(16, '\0');
        QByteArray hash;
        QFile itemFile(item.path);
        if (itemFile.open(QIODevice::ReadOnly))
        {
            for (int i = 0; i < 16; ++i)
                salt[i] = static_cast<char>(QRandomGenerator::system()->bounded(256));
            hash = QCryptographicHash::hash(salt + itemFile.readAll(), QCryptographicHash::Sha256);
            itemFile.close();
        }
        else
        {
            qWarning() << "[Data] signMenuData: cannot read file for signing:" << item.path;
        }
        QJsonObject entry;
        entry["path"] = item.path;
        entry["salt"] = QString::fromLatin1(salt.toBase64());
        entry["hash"] = QString::fromLatin1(hash.toBase64());
        filesArr.append(entry);
    }

    QJsonObject payloadObj;
    payloadObj["v"] = 1;
    payloadObj["jsonSalt"] = QString::fromLatin1(jsonSalt.toBase64());
    payloadObj["json"] = QString::fromLatin1(jsonHmac.toBase64());
    payloadObj["files"] = filesArr;

    if (gen != g_sigGeneration)
        return; // 陈旧任务：已有更新的保存，丢弃，避免旧签名覆盖新签名

    const QByteArray signedBytes = QJsonDocument(payloadObj).toJson();
    QSaveFile sf(FilePaths.menuSigFile);
    if (!sf.open(QIODevice::WriteOnly) || sf.write(signedBytes) < 0 || !sf.commit())
    {
        qCritical() << "[Data] signMenuData: failed to write signature file" << FilePaths.menuSigFile;
        return;
    }
    qDebug() << "[Data] signMenuData: rewrote signature file"
             << QFileInfo(FilePaths.menuSigFile).absoluteFilePath() << ","
             << signedBytes.size() << "bytes," << filesArr.size() << "files signed";
}

// 后台异步写签：保存路径调用，避免哈希大文件阻塞 UI 线程
void DataManager::scheduleMenuResign(const QList<MenuData> &items)
{
    const int gen = ++g_sigGeneration;
    QtConcurrent::run([items, gen]() {
        DataManager::instance().signMenuData(items, gen);
    });
}

bool DataManager::verifyMenuData(bool *jsonOk, QStringList *failedFiles)
{
    if (jsonOk)
        *jsonOk = false;
    if (failedFiles)
        failedFiles->clear();

    QFile f(FilePaths.menuDataFile);
    if (!f.open(QIODevice::ReadOnly))
    {
        qWarning() << "[Data] verifyMenuData: cannot read" << FilePaths.menuDataFile;
        return false;
    }
    const QByteArray jsonBytes = f.readAll();

    QFile sigFile(FilePaths.menuSigFile);
    if (!sigFile.open(QIODevice::ReadOnly))
    {
        // TOFU：升级后首次启动无签名文件 → 生成机器密钥签名（信任当前菜单）
        qDebug() << "[Data] verifyMenuData: no signature file, signing current menu (TOFU)";
        signMenuData(getMenuData(), ++g_sigGeneration);
        if (jsonOk)
            *jsonOk = true;
        return true;
    }

    QJsonDocument doc = QJsonDocument::fromJson(sigFile.readAll());
    if (!doc.isObject())
    {
        qWarning() << "[Data] verifyMenuData: invalid signature file";
        return false;
    }
    QJsonObject obj = doc.object();
    QByteArray jsonSalt = QByteArray::fromBase64(obj.value("jsonSalt").toString().toUtf8());
    QByteArray storedJson = QByteArray::fromBase64(obj.value("json").toString().toUtf8());
    if (jsonSalt.size() != 16)
    {
        qWarning() << "[Data] verifyMenuData: invalid jsonSalt in signature";
        return false;
    }

    const QByteArray calcJson = QMessageAuthenticationCode::hash(jsonSalt + jsonBytes,
                                                                 menuHmacKey(), QCryptographicHash::Sha256);
    if (!constantTimeEqual(storedJson, calcJson))
    {
        qWarning() << "[Data] verifyMenuData: menuData.json HMAC mismatch (tampered?)";
        return false; // jsonOk 保持 false → 整批阻止自动启动
    }
    if (jsonOk)
        *jsonOk = true;

    // 逐条目文件校验
    QMap<QString, QPair<QByteArray, QByteArray>> fileMap; // path -> (salt, hash)
    const QJsonArray filesArr = obj.value("files").toArray();
    for (const QJsonValue &v : filesArr)
    {
        QJsonObject e = v.toObject();
        fileMap[e.value("path").toString()] = qMakePair(
            QByteArray::fromBase64(e.value("salt").toString().toUtf8()),
            QByteArray::fromBase64(e.value("hash").toString().toUtf8()));
    }

    bool allOk = true;
    const QList<MenuData> items = getMenuData();
    for (const MenuData &item : items)
    {
        if (item.path.isEmpty() || isUrlPath(item.path))
            continue;
        if (QFileInfo(item.path).isDir())
            continue;

        QByteArray salt, storedHash;
        const auto it = fileMap.constFind(item.path);
        if (it != fileMap.constEnd())
        {
            salt = it.value().first;
            storedHash = it.value().second;
        }
        bool ok = !storedHash.isEmpty();
        if (ok)
        {
            QFile itemFile(item.path);
            if (itemFile.open(QIODevice::ReadOnly))
                ok = constantTimeEqual(storedHash, QCryptographicHash::hash(
                                                         salt + itemFile.readAll(), QCryptographicHash::Sha256));
            else
                ok = false;
        }
        if (!ok)
        {
            qWarning() << "[Data] verifyMenuData: file changed or unreadable:" << item.path;
            if (failedFiles)
                failedFiles->append(item.path);
            allOk = false;
        }
    }
    if (!allOk && failedFiles && !failedFiles->isEmpty())
        qWarning() << "[Data] verifyMenuData: changed files:" << failedFiles->join(", ");
    return allOk;
}

void DataManager::writeOpenWeatherData(const OpenWeatherData &opwdt)
{
    QWriteLocker wl(&DataManager::instance().rwlock);
    DataManager::instance().openWeather_data = opwdt;
    QJsonObject obj;
    obj["city"] = opwdt.city;
    obj["api_key"] = opwdt.api_key;
    writeJsonFile(FilePaths.openWeatherFile, QJsonDocument(obj));
}

void DataManager::writeLlamaData(const LlamaData &llm)
{
    QWriteLocker wl(&DataManager::instance().rwlock);
    DataManager::instance().llama_data = llm;
    QJsonObject obj;
    obj["maxContextMessages"] = llm.maxContextMessages;
    obj["model"] = llm.model;
    obj["systemPrompt"] = llm.systemPrompt;
    obj["baseUrl"] = llm.baseUrl;
    obj["apiKey"] = llm.apiKey;
    obj["promptFilePath"] = llm.promptFilePath;
    writeJsonFile(FilePaths.llmConfigFile, QJsonDocument(obj));
}

void DataManager::writeTTSConfig(const TTSConfig &ttsc)
{
    QWriteLocker wl(&DataManager::instance().rwlock);
    DataManager::instance().tts_config = ttsc;
    QJsonObject obj;
    obj["provider"] = ttsc.provider;
    obj["speaker_openai_edge_tts"] = ttsc.speaker_openai_edge_tts;
    obj["speed_openai_edge_tts"] = ttsc.speed_openai_edge_tts;
    obj["openai_endpoint"] = ttsc.openai_endpoint;
    obj["openai_apiKey"] = ttsc.openai_apiKey;
    obj["openai_model"] = ttsc.openai_model;
    obj["openai_voice"] = ttsc.openai_voice;
    obj["openai_speed"] = ttsc.openai_speed;
    obj["iFlytek_APPID"] = ttsc.iFlytek_APPID;
    obj["iFlytek_APISecret"] = ttsc.iFlytek_APISecret;
    obj["iFlytek_APIKey"] = ttsc.iFlytek_APIKey;
    obj["iFlytek_speaker"] = ttsc.iFlytek_speaker;
    obj["voicevox_dict_dir"] = ttsc.voicevox_dict_dir;
    obj["voicevox_model"] = ttsc.voicevox_model;
    obj["voicevox_style_id"] = ttsc.voicevox_style_id;
    obj["voicevox_speed"] = ttsc.voicevox_speed;
    obj["tr_point"] = ttsc.tr_point;
    obj["tr_libretranslate_port"] = ttsc.tr_libretranslate_port;
    obj["tr_provider"] = ttsc.tr_provider;
    obj["tr_lang_translators"] = ttsc.tr_lang_translators;
    obj["tr_lang_libretranslate"] = ttsc.tr_lang_libretranslate;
    obj["tr_tx_secret_id"] = ttsc.tr_tx_secret_id;
    obj["tr_tx_secret_key"] = ttsc.tr_tx_secret_key;
    obj["tr_tx_region"] = ttsc.tr_tx_region;
    obj["tr_tx_project_id"] = ttsc.tr_tx_project_id;
    obj["tr_tx_source_lang"] = ttsc.tr_tx_source_lang;
    obj["tr_tx_target_lang"] = ttsc.tr_tx_target_lang;
    writeJsonFile(FilePaths.ttsConfigFile, QJsonDocument(obj));
}

QJsonDocument DataManager::readJsonFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty())
        return {};

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError)
    {
        qCritical() << "[Data] JSON parse error in" << filePath << ":" << err.errorString();
        return {};
    }
    return doc;
}

// ── ConfigData serialization ──

QJsonObject DataManager::serializeConfig(const ConfigData &d)
{
    QJsonObject obj;
    obj["model_path"] = d.model_path;
    obj["model_size"] = d.model_size;
    obj["FPS"] = d.FPS;
    obj["volume"] = d.volume;

    QJsonObject bubble;
    bubble["forground"] = d.color_bubble.forground;
    bubble["background"] = d.color_bubble.background;
    obj["color_bubble"] = bubble;

    QJsonObject tray;
    tray["forground"] = d.color_tray.forground;
    tray["background"] = d.color_tray.background;
    obj["color_tray"] = tray;

    QJsonObject interval;
    interval["min"] = d.RandomInterval.first;
    interval["max"] = d.RandomInterval.second;
    obj["RandomInterval"] = interval;

    obj["music_tray_symbol"] = d.music_tray_symbol;
    obj["isStartUp"] = d.isStartUp;
    obj["isListening"] = d.isListening;
    obj["isLookingMouse"] = d.isLookingMouse;
    obj["LookingMouseStrength"] = d.LookingMouseStrength;
    obj["isStartStar"] = d.isStartStar;
    obj["StarCheckTime"] = d.StarCheckTime;
    obj["StarRunTimeout"] = d.StarRunTimeout;
    obj["isRandomSpeech"] = d.isRandomSpeech;
    obj["isSaying"] = d.isSaying;
    obj["isHourAlarm"] = d.isHourAlarm;
    obj["isTop"] = d.isTop;
    obj["isTrayHourAlarm"] = d.isTrayHourAlarm;
    obj["isSilentBoot"] = d.isSilentBoot;
    obj["isRecordWindowLocation"] = d.isRecordWindowLocation;
    obj["trayIconMode"] = d.trayIconMode;
    obj["trayGifPath"] = d.trayGifPath;
    obj["ShowLaunchMenuinTrayMenu"] = d.ShowLaunchMenuinTrayMenu;
    obj["isShowThinkingBubble"] = d.isShowThinkingBubble;
    obj["isLLMGreeting"] = d.isLLMGreeting;
    obj["language"] = d.language;
    obj["theme"] = d.theme;
    return obj;
}

ConfigData DataManager::deserializeConfig(const QJsonObject &obj)
{
    ConfigData d;
    d.model_path = obj["model_path"].toString().trimmed();
    d.model_size = obj["model_size"].toInt(150);
    d.FPS = obj["FPS"].toInt(30);
    d.volume = obj["volume"].toInt(50);

    QJsonObject bubble = obj["color_bubble"].toObject();
    d.color_bubble.forground = bubble["forground"].toString("#ffffffff").trimmed();
    d.color_bubble.background = bubble["background"].toString("#ff00ffff").trimmed();

    QJsonObject tray = obj["color_tray"].toObject();
    d.color_tray.forground = tray["forground"].toString("#ff00ff").trimmed();
    d.color_tray.background = tray["background"].toString("#ff0000").trimmed();

    QJsonObject interval = obj["RandomInterval"].toObject();
    d.RandomInterval.first = interval["min"].toInt(10);
    d.RandomInterval.second = interval["max"].toInt(25);

    d.music_tray_symbol = obj["music_tray_symbol"].toString("\u266B").trimmed();
    d.isStartUp = obj["isStartUp"].toBool();
    d.isListening = obj["isListening"].toBool();
    d.isLookingMouse = obj["isLookingMouse"].toBool(true);
    d.LookingMouseStrength = obj["LookingMouseStrength"].toDouble(1);
    d.isStartStar = obj["isStartStar"].toBool();
    d.StarCheckTime = obj["StarCheckTime"].toInt(20);
    d.StarRunTimeout = obj["StarRunTimeout"].toInt(1);
    d.isRandomSpeech = obj["isRandomSpeech"].toBool(true);
    d.isSaying = obj["isSaying"].toBool(true);
    d.isHourAlarm = obj["isHourAlarm"].toBool(true);
    d.isTop = obj["isTop"].toBool();
    d.isTrayHourAlarm = obj["isTrayHourAlarm"].toBool();
    d.isSilentBoot = obj["isSilentBoot"].toBool();
    d.isRecordWindowLocation = obj["isRecordWindowLocation"].toBool();
    d.trayIconMode = obj["trayIconMode"].toInt(TrayIcon_Static);
    d.trayGifPath = obj["trayGifPath"].toString().trimmed();
    d.ShowLaunchMenuinTrayMenu = obj["ShowLaunchMenuinTrayMenu"].toBool(true);
    d.isShowThinkingBubble = obj["isShowThinkingBubble"].toBool();
    d.isLLMGreeting = obj["isLLMGreeting"].toBool();
    d.language = obj["language"].toString().trimmed();
    d.theme = obj["theme"].toString("Fusion");
    return d;
}

// ── MenuData list serialization ──

QJsonArray DataManager::serializeMenuList(const QList<MenuData> &list)
{
    QJsonArray arr;
    for (const auto &m : list)
    {
        QJsonObject obj;
        obj["category"] = m.category;
        obj["name"] = m.name;
        obj["path"] = m.path;
        obj["icon"] = m.icon;
        obj["description"] = m.description;
        arr.append(obj);
    }
    return arr;
}

QList<MenuData> DataManager::deserializeMenuList(const QJsonArray &arr)
{
    QList<MenuData> list;
    for (const auto &v : arr)
    {
        QJsonObject obj = v.toObject();
        MenuData m;
        m.category = obj["category"].toString().trimmed();
        m.name = obj["name"].toString().trimmed();
        m.path = obj["path"].toString().trimmed();
        m.icon = obj["icon"].toString().trimmed();
        m.description = obj["description"].toString().trimmed();
        list.append(m);
    }
    return list;
}

// ── TodoData list serialization ──

QJsonArray DataManager::serializeTodoList(const QList<TodoData> &list)
{
    QJsonArray arr;
    for (const auto &t : list)
    {
        QJsonObject obj;
        obj["category"] = t.category;
        obj["title"] = t.title;
        obj["content"] = t.content;
        obj["deadline"] = t.deadline;
        obj["remarks"] = t.remarks;
        obj["isNotify"] = t.isNotify;
        arr.append(obj);
    }
    return arr;
}

QList<TodoData> DataManager::deserializeTodoList(const QJsonArray &arr)
{
    QList<TodoData> list;
    for (const auto &v : arr)
    {
        QJsonObject obj = v.toObject();
        TodoData t;
        t.category = obj["category"].toInt();
        t.title = obj["title"].toString().trimmed();
        t.content = obj["content"].toString().trimmed();
        t.deadline = obj["deadline"].toString().trimmed();
        t.remarks = obj["remarks"].toString().trimmed();
        t.isNotify = obj["isNotify"].toBool();
        list.append(t);
    }
    return list;
}

// ── Read / write implementations ──

void DataManager::writeData(ToDoSettingData setting)
{
    QWriteLocker wl(&rwlock);
    QJsonObject obj;
    obj["is_show_todo"] = setting.is_show_todo;
    obj["is_notify_tray"] = setting.is_notify_tray;
    writeJsonFile(FilePaths.todoNotifyFile, QJsonDocument(obj));
    todo_setting_data = setting;
}

void DataManager::readMenuData()
{
    QJsonDocument doc = readJsonFile(FilePaths.menuDataFile);
    if (!doc.isNull())
    {
        cached_menu_data = deserializeMenuList(doc.array());
    }
}

void DataManager::readBasicData()
{
    QJsonDocument doc = readJsonFile(FilePaths.configDataFile);
    if (doc.isObject())
    {
        basic_data = deserializeConfig(doc.object());
    }
}

void DataManager::readTodoData()
{
    QJsonDocument doc = readJsonFile(FilePaths.todoDataFile);
    if (!doc.isNull())
    {
        todo_data = deserializeTodoList(doc.array());
    }
}

void DataManager::readTodoNotify()
{
    QJsonDocument doc = readJsonFile(FilePaths.todoNotifyFile);
    if (doc.isObject())
    {
        QJsonObject obj = doc.object();
        todo_setting_data.is_show_todo = obj["is_show_todo"].toBool(true);
        todo_setting_data.is_notify_tray = obj["is_notify_tray"].toBool(true);
    }
}

void DataManager::readTTSConfig()
{
    QJsonDocument doc = readJsonFile(FilePaths.ttsConfigFile);
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();
    if (obj.isEmpty()) return;

    tts_config.provider = obj["provider"].toInt(0);
    if (tts_config.provider < 0 || tts_config.provider >= TTSProviderList.length())
    {
        qDebug() << "[Data] Invalid TTS provider:" << tts_config.provider << ", reset to 0";
        tts_config.provider = 0;
    }
    tts_config.speaker_openai_edge_tts = obj["speaker_openai_edge_tts"].toString("zh-CN-XiaoxiaoNeural").trimmed();
    tts_config.speed_openai_edge_tts = obj["speed_openai_edge_tts"].toDouble(1.0);
    tts_config.openai_endpoint = obj["openai_endpoint"].toString().trimmed();
    tts_config.openai_apiKey = obj["openai_apiKey"].toString().trimmed();
    tts_config.openai_model = obj["openai_model"].toString().trimmed();
    tts_config.openai_voice = obj["openai_voice"].toString("alloy").trimmed();
    tts_config.openai_speed = obj["openai_speed"].toDouble(1.0);
    tts_config.iFlytek_APPID = obj["iFlytek_APPID"].toString().trimmed();
    tts_config.iFlytek_APISecret = obj["iFlytek_APISecret"].toString().trimmed();
    tts_config.iFlytek_APIKey = obj["iFlytek_APIKey"].toString().trimmed();
    tts_config.iFlytek_speaker = obj["iFlytek_speaker"].toString("x4_yezi").trimmed();

    if (tts_config.provider == 1 && (tts_config.iFlytek_APPID.isEmpty() || tts_config.iFlytek_APISecret.isEmpty() || tts_config.iFlytek_APIKey.isEmpty()))
    {
        qDebug() << "[Data] iFlytek credentials missing, fallback to OpenAI TTS";
        tts_config.provider = 0;
    }
    tts_config.voicevox_dict_dir = obj["voicevox_dict_dir"].toString().trimmed();
    tts_config.voicevox_model = obj["voicevox_model"].toString().trimmed();
    tts_config.voicevox_style_id = obj["voicevox_style_id"].toInt(0);
    tts_config.voicevox_speed = obj["voicevox_speed"].toDouble(1.0);
    tts_config.tr_point = obj["tr_point"].toInt(1);
    tts_config.tr_lang_libretranslate = obj["tr_lang_libretranslate"].toString().trimmed();
    tts_config.tr_libretranslate_port = obj["tr_libretranslate_port"].toString().trimmed();
    tts_config.tr_provider = obj["tr_provider"].toString().trimmed();
    tts_config.tr_lang_translators = obj["tr_lang_translators"].toString().trimmed();
    tts_config.tr_tx_secret_id = obj["tr_tx_secret_id"].toString().trimmed();
    tts_config.tr_tx_secret_key = obj["tr_tx_secret_key"].toString().trimmed();
    tts_config.tr_tx_region = obj["tr_tx_region"].toString().trimmed();
    tts_config.tr_tx_project_id = obj["tr_tx_project_id"].toInt(0);
    tts_config.tr_tx_source_lang = obj["tr_tx_source_lang"].toString("auto").trimmed();
    tts_config.tr_tx_target_lang = obj["tr_tx_target_lang"].toString().trimmed();
}

void DataManager::readLlamaData()
{
    QJsonDocument doc = readJsonFile(FilePaths.llmConfigFile);
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();

    llama_data.maxContextMessages = obj["maxContextMessages"].toInt(10);
    llama_data.model = obj["model"].toString().trimmed();
    llama_data.systemPrompt = obj["systemPrompt"].toString("You are a friendly AI assistant.").trimmed();
    llama_data.baseUrl = obj["baseUrl"].toString().trimmed();
    llama_data.apiKey = obj["apiKey"].toString().trimmed();
    llama_data.promptFilePath = obj["promptFilePath"].toString().trimmed();
}

void DataManager::readOpenWeatherData()
{
    QJsonDocument doc = readJsonFile(FilePaths.openWeatherFile);
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();

    openWeather_data.api_key = obj["api_key"].toString().trimmed();
    if (openWeather_data.api_key.isEmpty())
    {
        qWarning() << "[Data] api_key in config file is empty or does not exist";
    }
    openWeather_data.city = obj["city"].toString().trimmed();
}

QFont DataManager::loadFont()
{
    QString path = ":/public/font/MapleMono-NF-CN-Medium.ttf";
    if (QFile::exists(path))
    {
        int id = QFontDatabase::addApplicationFont(path);
        if (id != -1)
        {
            QStringList families = QFontDatabase::applicationFontFamilies(id);
            if (!families.isEmpty())
            {
                qInfo() << "[Data] Load font success:" << families.at(0);
                return QFont(families.at(0));
            }
        }
    }
    return QFont();
}

// ── Lazy load guards (first getter call reads disk, then cached) ──

void DataManager::ensureMenuLoaded()
{
    if (menuLoaded) return;
    QWriteLocker wl(&rwlock);
    if (menuLoaded) return;
    readMenuData();
    menuLoaded = true;
}

void DataManager::ensureBasicLoaded()
{
    if (basicLoaded) return;
    QWriteLocker wl(&rwlock);
    if (basicLoaded) return;
    readBasicData();
    basicLoaded = true;
}

void DataManager::ensureTodoLoaded()
{
    if (todoLoaded) return;
    QWriteLocker wl(&rwlock);
    if (todoLoaded) return;
    readTodoData();
    todoLoaded = true;
}

void DataManager::ensureTodoSettingLoaded()
{
    if (todoSettingLoaded) return;
    QWriteLocker wl(&rwlock);
    if (todoSettingLoaded) return;
    readTodoNotify();
    todoSettingLoaded = true;
}

void DataManager::ensureTTSLoaded()
{
    if (ttsLoaded) return;
    QWriteLocker wl(&rwlock);
    if (ttsLoaded) return;
    readTTSConfig();
    ttsLoaded = true;
}

void DataManager::ensureOpenWeatherLoaded()
{
    if (openWeatherLoaded) return;
    QWriteLocker wl(&rwlock);
    if (openWeatherLoaded) return;
    readOpenWeatherData();
    openWeatherLoaded = true;
}

void DataManager::ensureLlamaLoaded()
{
    if (llamaLoaded) return;
    QWriteLocker wl(&rwlock);
    if (llamaLoaded) return;
    readLlamaData();
    llamaLoaded = true;
}
