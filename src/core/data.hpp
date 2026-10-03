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
#include <QReadWriteLock>
#include "llamaclient.h"
#include "ttsconfig.hpp"

#define VERSION "v0.8.1"

enum TrayIconMode : int
{
    TrayIcon_Static = 0,
    TrayIcon_Text = 1,
    TrayIcon_Gif = 2
};
enum RandomSentenceMode : int
{
    automation = 0,
    click = 1
};
enum ChatScenery : int
{
    onModel = 1,
    inUI = 2,
    bubble = 3
};

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
    QString menuSigFile = "user/.menuSig"; // H15: 菜单 HMAC 签名（明文 JSON，密钥由机器标识派生）
    QString windowLocationFile = "user/window_location.dat";
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
    float LookingMouseStrength = 1.0;
    bool isStartStar = false;
    int StarCheckTime = 20;
    int StarRunTimeout = 1;
    bool isRandomSpeech = true;
    bool isSaying = true;
    bool isHourAlarm = true;
    bool isTop = false;
    bool isTrayHourAlarm = false;
    bool isSilentBoot = false;
    bool isRecordWindowLocation = false;
    int trayIconMode = TrayIconMode::TrayIcon_Static;
    bool ShowLaunchMenuinTrayMenu = true;
    bool alwaysDynamicEffects = false;
    QString trayGifPath;
    bool isShowThinkingBubble = false;
    bool isLLMGreeting = false;
    QString language;
    QString theme;
};

struct constConfigData
{
    const QString openai_edge_tts_Voice_Samples = "https://tts.travisvn.com/";
    const QString iFlytek_tts_url = "https://console.xfyun.cn/services/tts";
    const QString openWeather_url = "https://home.openweathermap.org/api_keys";
    const QString docs_link = "https://github.com/igugyj/Pelr/tree/master/docs";
    // Components 页顶部引导链接（与 docs/app-components.md 对应）
    const QString components_doc_link = "https://github.com/igugyj/Pelr/blob/master/docs/app-components.md";
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

enum TodoCategory : int
{
    done = 0,
    todo = 1
};

struct TodoData
{
    int category;
    QString title;
    QString content;
    QString deadline;
    QString remarks;
    bool isNotify;

    bool operator==(const TodoData &other) const
    {
        return category == other.category && title == other.title && content == other.content && deadline == other.deadline && remarks == other.remarks && isNotify == other.isNotify;
    }

    bool operator!=(const TodoData &other) const
    {
        return !(*this == other);
    }
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

static QVector<QPair<QString, int>> TTSProviderList = {
    {"OpenAI-Edge-TTS", 0},
    {"iFlytek", 1},
    {"voicevox", 2},
    {"OpenAI-Compatible", 3},
};
static QVector<QPair<QString, int>> TrayIconModes = {
    {"Static", TrayIcon_Static},
    {"Text", TrayIcon_Text},
    {"GIF", TrayIcon_Gif},
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

    bool menuLoaded = false;
    bool basicLoaded = false;
    bool todoLoaded = false;
    bool todoSettingLoaded = false;
    bool ttsLoaded = false;
    bool openWeatherLoaded = false;
    bool llamaLoaded = false;

    QReadWriteLock rwlock;

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
        ensureOpenWeatherLoaded();
        QReadLocker rl(&rwlock);
        return openWeather_data;
    }
    LlamaData getLlamaData()
    {
        ensureLlamaLoaded();
        QReadLocker rl(&rwlock);
        return llama_data;
    }
    TTSConfig getTTSConfig()
    {
        ensureTTSLoaded();
        QReadLocker rl(&rwlock);
        return tts_config;
    }

    ToDoSettingData getTodoSetting()
    {
        ensureTodoSettingLoaded();
        QReadLocker rl(&rwlock);
        return todo_setting_data;
    }

    QList<MenuData> getMenuData()
    {
        ensureMenuLoaded();
        QReadLocker rl(&rwlock);
        return cached_menu_data;
    }

    ConfigData getBasicData()
    {
        ensureBasicLoaded();
        QReadLocker rl(&rwlock);
        return basic_data;
    }

    QList<TodoData> getTodoData()
    {
        ensureTodoLoaded();
        QReadLocker rl(&rwlock);
        return todo_data;
    }

    static void writeOpenWeatherData(const OpenWeatherData &opwdt);
    static void writeTTSConfig(const TTSConfig &ttsc);
    static void writeLlamaData(const LlamaData &llm);

    template <typename T>
    void writeData(const T &data)
    {
        QWriteLocker wl(&rwlock);
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
        if constexpr (std::is_same_v<T, QList<MenuData>>)
        {
            const QList<MenuData> snapshot = data; // 拷贝给子线程，避免跨线程读共享状态
            wl.unlock();                           // 菜单文件已落盘，签名改为后台执行，不再阻塞 UI
            scheduleMenuResign(snapshot);
        }
    }

    // H15: 菜单数据签名/验签（HMAC-SHA256（密钥由机器标识派生）+ 逐条目文件 SHA-256）
    void signMenuData(const QList<MenuData> &items, int gen);
    static void scheduleMenuResign(const QList<MenuData> &items);
    bool verifyMenuData(bool *jsonOk = nullptr, QStringList *failedFiles = nullptr);

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

    void ensureMenuLoaded();
    void ensureBasicLoaded();
    void ensureTodoLoaded();
    void ensureTodoSettingLoaded();
    void ensureTTSLoaded();
    void ensureOpenWeatherLoaded();
    void ensureLlamaLoaded();
};
