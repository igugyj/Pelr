#pragma once

#include <QFile>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QPair>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>
#include "llamaclient.h"

#define VERSION "0.1.3"

struct filePaths
{
    QString menuDataFile = "user/menuData.json";
    QString configDataFile = "user/configData.json";
    QString todoDataFile = "user/todoData.json";
    QString todoNotifyFile = "user/todoNotify.json";
    QString ttsConfigFile = "user/ttsConfig.json";
    QString openWeatherFile = "user/openWeather.json";
    QString llmConfigFile = "user/llmConfig.json";
    QString defaultTextFile = "assets/text/text.json";
    QString userTextFile = "user/text.json";
};
inline filePaths FilePaths;

struct colorPair
{
    QString forground;
    QString background;
};

struct ConfigData
{
    // basic
    QString model_path;
    int model_size = 120;
    int FPS = 30;
    int volume = 50;
    colorPair color_bubble = {"#ffffffff", "#ff00ffff"};
    colorPair color_tray = {"#002fff", "#ff0000"};
    QPair<int, int> RandomInterval = {10, 25};
    QString music_tray_symbol = "\u266B";
    // bool
    bool isStartUp = false;
    bool isListening = false;
    bool isLookingMouse = true;
    bool isStartStar = false;
    bool isRandomSpeech = true;
    bool isSaying = true;
    bool isHourAlarm = true;
    bool isTop = false;
    bool isTrayHourAlarm = false;
    bool isSilentBoot = false;
    bool isRecordWindowLocation = false;
    bool isMusicIcon = false;
    bool isShowThinkingBubble = false;
};

struct constConfigData
{
    const QString openai_edge_tts_Voice_Samples = "https://tts.travisvn.com/";
    const QString iFlytek_tts_url = "https://console.xfyun.cn/services/tts";
    const QString openWeather_url = "https://home.openweathermap.org/api_keys";
    const QString docs_link = "https://github.com/igugyj/Pelr/tree/master/docs";
    const QString version = VERSION;
    const QString Gitee_repo_owner = "Pfolg";
    const QString Gitee_repo_name = "Pelr";
    const QString Github_repo_owner = "igugyj";
    const QString Github_repo_name = "Pelr";
    const QString team_link = "https://github.com/igugyj/Pelr/graphs/contributors";
    const QString website_link = "https://github.com/igugyj/Pelr";
    const QString feedback_link = "https://github.com/igugyj/Pelr/issues";
    const QString VoiceFolder = "voice_files";
    const QString userFolder = "user";
    const QString logFolder = "log";
};

struct LlamaData
{
    QString model;
    QString systemPrompt;
    QString baseUrl;
    QString apiKey;
    QString promptFilePath;
    int maxContextMessages;
};

struct TodoData
{
    int category;
    QString title;
    QString content;
    QString deadline;
    QString remarks;
    bool isNotify;
};

struct MenuData
{
    QString category;
    QString name;
    QString path;
    QString icon;
    QString description;

    friend bool operator!=(const MenuData &m1, const MenuData &m2)
    {
        return m1.category != m2.category || m1.name != m2.name || m1.path != m2.path || m1.icon != m2.icon || m1.description != m2.description;
    }
};

struct ToDoSettingData
{
    bool is_show_todo = true;
    bool is_notify_tray = true;
};

struct TTSConfig
{
    int provider = 0;
    QString speaker_openai_edge_tts = "zh-CN-XiaoxiaoNeural";
    double speed_openai_edge_tts = 1.0;
    QString iFlytek_APPID;
    QString iFlytek_APISecret;
    QString iFlytek_APIKey;
    QString iFlytek_speaker = "x4_yezi";
    QString voicevox_dict_dir;
    QString voicevox_model;
    int voicevox_style_id;
    double voicevox_speed = 1.0;
    int tr_point;
    QString tr_provider;
    QString tr_lang_translators;
    QString tr_lang_libretranslate;
    QString tr_libretranslate_port = "5000";
    QString tr_tx_secret_id;
    QString tr_tx_secret_key;
    QString tr_tx_region;
    int tr_tx_project_id = 0;
    QString tr_tx_source_lang;
    QString tr_tx_target_lang;
};

static QVector<QPair<QString, int>> TTSProviderList = {
    {"OpenAI-Edge-TTS", 0},
    {"iFlytek", 1},
    {"voicevox", 2},
};

static QVector<QPair<QString, int>> Translators = {
    {"libretranslate", 0},
    {"translators", 1},
    {"Tencent", 2},
};

struct OpenWeatherData
{
    QString city;
    QString api_key;
};

class DataManager
{
private:
    DataManager() = default;
    ~DataManager() = default;
    DataManager(const DataManager &) = delete;
    DataManager &operator=(const DataManager &) = delete;

protected:
    QList<MenuData> cached_menu_data;
    ConfigData basic_data;
    ToDoSettingData todo_setting_data;
    TTSConfig tts_config;
    OpenWeatherData openWeather_data;
    LlamaData llama_data;

public:
    QList<TodoData> todo_data;
    constConfigData const_config_data;
    QFont _font = loadFont();
    const QString Project_Name = "Pelr";

    static DataManager &instance()
    {
        static DataManager instance;
        return instance;
    }

    OpenWeatherData getOpenWeatherData()
    {
        readOpenWeatherData();
        return openWeather_data;
    }
    LlamaData getLlamaData()
    {
        readLlamaData();
        return llama_data;
    }
    TTSConfig getTTSConfig()
    {
        readTTSConfig();
        return tts_config;
    }

    ToDoSettingData getTodoSetting()
    {
        readTodoNotify();
        return todo_setting_data;
    }

    QList<MenuData> getMenuData()
    {
        readMenuData();
        return cached_menu_data;
    }

    ConfigData getBasicData()
    {
        readBasicData();
        return basic_data;
    }

    QList<TodoData> getTodoData()
    {
        readTodoData();
        return todo_data;
    }

    static void writeOpenWeatherData(const OpenWeatherData &opwdt);
    static void writeTTSConfig(const TTSConfig &ttsc);
    static void writeLlamaData(const LlamaData &llm);

    template <typename T>
    void writeData(const T &data)
    {
        QString filename;
        QJsonDocument doc;

        if constexpr (std::is_same_v<T, ConfigData>)
        {
            filename = FilePaths.configDataFile;
            doc.setObject(serializeConfig(data));
            basic_data = data;
        }
        else if constexpr (std::is_same_v<T, QList<MenuData>>)
        {
            filename = FilePaths.menuDataFile;
            doc.setArray(serializeMenuList(data));
            cached_menu_data = data;
        }
        else if constexpr (std::is_same_v<T, QList<TodoData>>)
        {
            filename = FilePaths.todoDataFile;
            doc.setArray(serializeTodoList(data));
            todo_data = data;
        }
        else
        {
            qCritical() << "[Data] Unsupported data type for writing:" << typeid(T).name();
            return;
        }
        writeJsonFile(filename, doc);
    }

    void writeData(ToDoSettingData setting);

protected:
    QJsonDocument readJsonFile(const QString &filePath);
    static bool writeJsonFile(const QString &filePath, const QJsonDocument &doc);
    static QJsonObject serializeConfig(const ConfigData &data);
    static ConfigData deserializeConfig(const QJsonObject &obj);
    static QJsonArray serializeMenuList(const QList<MenuData> &list);
    static QList<MenuData> deserializeMenuList(const QJsonArray &arr);
    static QJsonArray serializeTodoList(const QList<TodoData> &list);
    static QList<TodoData> deserializeTodoList(const QJsonArray &arr);

    void readLlamaData();
    void readOpenWeatherData();
    void readTTSConfig();
    void readTodoNotify();
    void readTodoData();
    static QFont loadFont();
    void readMenuData();
    void readBasicData();
};
