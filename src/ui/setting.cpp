
#include "setting.h"
#include <QFileDialog>
#include <QString>
#include "launcher.hpp"
#include <windows.h>
#include <shlobj.h>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QColorDialog>
#include <QColor>
#include <QDebug>
#include <QStyleFactory>
#include "LAppLive2DManager.hpp"
#include "logger.hpp"
#include "llamaclient.h"
#include "tray.h"
#include "NotificationWidget.h"
#include "loadText.h"
#include "UpdateDialog.h"
#include "voicevox_tts.h"
#include "voicegenerator.hpp"
#include <QProcess>
#include <QRandomGenerator>
#include <QDateTime>
#include <QStandardPaths>
#if __has_include(<FluentUI3Style/fluentui3styleproperties.h>)
#include <FluentUI3Style/fluentui3styleproperties.h>
#endif
using MessageType = NotificationWidget::MessageType;

ConfigData SettingWidget::getAllValues()
{
    ConfigData data;
    // basic
    data.model_path = ui->lineEdit->text();
    QFileInfo fi(data.model_path);
    // 检查文件是否存在
    if (fi.exists() && fi.isFile())
    {
        qDebug() << "[Settings] File exists:" << data.model_path;
    }
    else
    {
        qWarning() << "[Settings] File does not exist:" << data.model_path;
        data.model_path = "";
    }
    data.model_size = ui->horizontalSlider->value();
    data.FPS = ui->horizontalSlider_2->value();
    data.volume = ui->horizontalSlider_3->value();
    data.RandomInterval.first = ui->spinBox->value();
    data.RandomInterval.second = ui->spinBox_2->value();
    // basic color
    data.color_bubble.background = ui->label_24->text(); // ARGB
    data.color_bubble.forground = ui->label_25->text();
    // tray
    data.color_tray.forground = ui->label_34->text();
    data.color_tray.background = ui->label_35->text();
    data.music_tray_symbol = ui->lineEdit_9->text().isEmpty() ? "♫" : ui->lineEdit_9->text();

    // bool
    data.isStartUp = ui->checkBox->isChecked();
    data.isListening = ui->checkBox_2->isChecked();
    data.isLookingMouse = ui->checkBox_3->isChecked();
    data.isStartStar = ui->checkBox_4->isChecked();
    data.isRandomSpeech = ui->checkBox_5->isChecked();
    data.isSaying = ui->checkBox_6->isChecked();
    data.isHourAlarm = ui->checkBox_7->isChecked();
    data.isTop = ui->checkBox_8->isChecked();
    data.isTrayHourAlarm = ui->checkBox_9->isChecked();
    data.isSilentBoot = ui->checkBox_10->isChecked();
    data.isRecordWindowLocation = ui->checkBox_11->isChecked();
    data.isMusicIcon = ui->checkBox_12->isChecked();
    data.isShowThinkingBubble = ui->checkBox_14->isChecked();
    data.isLLMGreeting = ui->checkBox_13->isChecked();

    data.LookingMouseStrength = ui->doubleSpinBox_3->value();
    data.StarCheckTime = ui->spinBox_5->value();
    data.StarRunTimeout = ui->spinBox_6->value();

    data.language = ui->comboBox_2->currentData().toString();
    data.theme = ui->comboBox_8->currentData().toString();

    return data;
}

TTSConfig SettingWidget::getTTSConfigValue() const
{
    TTSConfig data;
    data.provider = ui->comboBox_4->currentIndex();
    qDebug() << "[Settings] TTS provider:" << data.provider;
    data.tr_point = ui->comboBox_7->currentIndex();
    qDebug() << "[Settings] TRA provider:" << data.tr_point;
    // openai_edge_tts
    data.speaker_openai_edge_tts = ui->lineEdit_11->text();
    data.speed_openai_edge_tts = ui->doubleSpinBox->value();
    // iFlytek
    data.iFlytek_APPID = ui->lineEdit_2->text();
    data.iFlytek_APISecret = ui->lineEdit_3->text();
    data.iFlytek_APIKey = ui->lineEdit_4->text();
    data.iFlytek_speaker = ui->lineEdit_5->text();
    // voicevox
    data.voicevox_dict_dir = ui->lineEdit_15->text();
    data.voicevox_model = ui->lineEdit_16->text();
    data.voicevox_style_id = ui->comboBox_6->currentData().toInt(); // 固定使用 userData 中的 ID
    data.voicevox_speed = ui->doubleSpinBox_2->value();
    // OpenAI-Compatible
    data.openai_endpoint = ui->lineEdit_25->text();
    data.openai_apiKey = ui->lineEdit_26->text();
    data.openai_model = ui->lineEdit_27->text();
    data.openai_voice = ui->lineEdit_28->text();
    data.openai_speed = ui->doubleSpinBox_4->value();
    // translate
    data.tr_lang_translators = ui->comboBox_5->currentText();
    data.tr_provider = ui->comboBox->currentText();
    data.tr_libretranslate_port = ui->lineEdit_17->text();
    data.tr_lang_libretranslate = ui->lineEdit_18->text();
    // Tencent
    data.tr_tx_secret_id = ui->lineEdit_19->text();
    data.tr_tx_secret_key = ui->lineEdit_20->text();
    data.tr_tx_region = ui->lineEdit_21->text();
    data.tr_tx_project_id = ui->spinBox_4->value();
    data.tr_tx_source_lang = ui->lineEdit_22->text();
    data.tr_tx_target_lang = ui->lineEdit_23->text();

    return data;
}

OpenWeatherData SettingWidget::getOpenWeatherDataValue()
{
    OpenWeatherData data;
    // OpenWeather
    data.city = ui->lineEdit_7->text();
    data.api_key = ui->lineEdit_8->text();
    return data;
}
LlamaData SettingWidget::getLlamaDataValue() const
{
    LlamaData llm;
    llm.baseUrl = ui->lineEdit_10->text();
    llm.model = ui->lineEdit_6->text();
    llm.apiKey = ui->lineEdit_12->text();
    llm.maxContextMessages = ui->spinBox_3->value();
    llm.promptFilePath = ui->lineEdit_13->text();
    llm.systemPrompt = ui->plainTextEdit->toPlainText();

    LlamaClient::instance()->configure(llm.baseUrl, llm.apiKey, llm.model, llm.systemPrompt, llm.maxContextMessages);
    LlamaClient::instance()->loadSystemPromptFromFile(llm.promptFilePath);

    return llm;
}

void SettingWidget::setTTSConfig(const TTSConfig &data) const
{
    // TTS
    qDebug() << "[Settings] TTS provider:" << data.provider;
    ui->comboBox_4->setCurrentIndex(static_cast<int>(data.provider));
    qDebug() << "[Settings] TRA point:" << data.tr_point;
    ui->comboBox_7->setCurrentIndex(static_cast<int>(data.tr_point));
    // openai-edge-tts
    ui->lineEdit_11->setText(data.speaker_openai_edge_tts);
    ui->doubleSpinBox->setValue(data.speed_openai_edge_tts);
    // iFlytek
    ui->lineEdit_2->setText(data.iFlytek_APIKey);
    ui->lineEdit_3->setText(data.iFlytek_APISecret);
    ui->lineEdit_4->setText(data.iFlytek_APPID);
    ui->lineEdit_5->setText(data.iFlytek_speaker);
    // voicevox
    ui->lineEdit_15->setText(data.voicevox_dict_dir);
    ui->lineEdit_16->setText(data.voicevox_model);
    ui->doubleSpinBox_2->setValue(data.voicevox_speed); // 改用 doubleSpinBox_2

    // 根据 style ID 选中风格
    int idx = ui->comboBox_6->findData(data.voicevox_style_id);
    if (idx >= 0)
    {
        ui->comboBox_6->setCurrentIndex(idx);
    }
    else
    {
        // 如果 ID 不存在（例如模型未加载），保留空白或第一个
        if (ui->comboBox_6->count() > 0)
            ui->comboBox_6->setCurrentIndex(0);
    }
    // OpenAI-Compatible
    ui->lineEdit_25->setText(data.openai_endpoint);
    ui->lineEdit_26->setText(data.openai_apiKey);
    ui->lineEdit_27->setText(data.openai_model);
    ui->lineEdit_28->setText(data.openai_voice);
    ui->doubleSpinBox_4->setValue(data.openai_speed);
    // translate
    ui->comboBox_5->setCurrentText(data.tr_lang_translators);
    ui->comboBox->setCurrentText(data.tr_provider);
    ui->lineEdit_17->setText(data.tr_libretranslate_port);
    ui->lineEdit_18->setText(data.tr_lang_libretranslate);

    // Tencent
    ui->lineEdit_19->setText(data.tr_tx_secret_id);
    ui->lineEdit_20->setText(data.tr_tx_secret_key);
    ui->lineEdit_21->setText(data.tr_tx_region);
    ui->spinBox_4->setValue(data.tr_tx_project_id);
    ui->lineEdit_22->setText(data.tr_tx_source_lang);
    ui->lineEdit_23->setText(data.tr_tx_target_lang);
}
void SettingWidget::setLlamaData(const LlamaData &llm) const
{
    ui->lineEdit_10->setText(llm.baseUrl);
    ui->lineEdit_6->setText(llm.model);
    ui->lineEdit_12->setText(llm.apiKey);
    ui->spinBox_3->setValue(llm.maxContextMessages);
    ui->lineEdit_13->setText(llm.promptFilePath);
    ui->plainTextEdit->setPlainText(llm.systemPrompt);
    LlamaClient::instance()->configure(
        llm.baseUrl,
        llm.apiKey,
        llm.model,
        llm.systemPrompt,
        llm.maxContextMessages);
    LlamaClient::instance()->loadSystemPromptFromFile(llm.promptFilePath);
}

void SettingWidget::setOpenWeatherData(const OpenWeatherData &data) const
{
    // OpenWeather
    ui->lineEdit_7->setText(data.city);
    ui->lineEdit_8->setText(data.api_key);
}

void SettingWidget::setAllValues(const ConfigData &data)
{
    ui->label_2->setText(DataManager::instance().const_config_data.version);
    // basic
    ui->lineEdit->setText(data.model_path);
    ui->horizontalSlider->setValue(data.model_size);
    ui->horizontalSlider_2->setValue(data.FPS);
    ui->horizontalSlider_3->setValue(data.volume);
    ui->label_7->setText(QString::number(data.model_size));
    ui->label_8->setText(QString::number(data.FPS));
    ui->label_9->setText(QString::number(data.volume));
    ui->spinBox->setValue(data.RandomInterval.first);
    ui->spinBox_2->setValue(data.RandomInterval.second);
    ui->comboBox_3->setCurrentIndex(static_cast<int>(getLogLevel()));
    // color
    ui->label_24->setStyleSheet(
        QString("background-color: %1;color: %2;").arg(data.color_bubble.background).arg(data.color_bubble.forground));
    ui->label_24->setText(data.color_bubble.background);
    ui->label_25->setStyleSheet(
        QString("background-color: %1;color: %2;").arg(data.color_bubble.background).arg(data.color_bubble.forground));
    ui->label_25->setText(data.color_bubble.forground);
    // tray
    ui->label_34->setStyleSheet(QString("color: %1;").arg(data.color_tray.forground));
    ui->label_35->setStyleSheet(QString("color: %1;").arg(data.color_tray.background));
    ui->label_34->setText(data.color_tray.forground);
    ui->label_35->setText(data.color_tray.background);
    ui->lineEdit_9->setText(data.music_tray_symbol);
    // bool
    ui->checkBox->setChecked(data.isStartUp);
    if (checkStartupLink())
    {
        ui->checkBox->setChecked(true);
    }
    else
    {
        ui->checkBox->setChecked(false);
    }
    ui->checkBox_2->setChecked(data.isListening);
    ui->checkBox_3->setChecked(data.isLookingMouse);
    ui->checkBox_4->setChecked(data.isStartStar);
    ui->checkBox_5->setChecked(data.isRandomSpeech);
    ui->checkBox_6->setChecked(data.isSaying);
    ui->checkBox_7->setChecked(data.isHourAlarm);
    ui->checkBox_8->setChecked(data.isTop);
    ui->checkBox_9->setChecked(data.isTrayHourAlarm);
    ui->checkBox_10->setChecked(data.isSilentBoot);
    ui->checkBox_11->setChecked(data.isRecordWindowLocation);
    ui->checkBox_12->setChecked(data.isMusicIcon);
    ui->checkBox_14->setChecked(data.isShowThinkingBubble);
    ui->checkBox_13->setChecked(data.isLLMGreeting);

    ui->doubleSpinBox_3->setValue(data.LookingMouseStrength);
    ui->spinBox_5->setValue(data.StarCheckTime);
    ui->spinBox_6->setValue(data.StarRunTimeout);

    QString langCode = data.language.isEmpty()
                           ? TranslationManager::instance()->currentLanguage()
                           : data.language;
    int langIdx = ui->comboBox_2->findData(langCode);
    if (langIdx >= 0)
    {
        m_isUpdatingLanguage = true;
        ui->comboBox_2->setCurrentIndex(langIdx);
        m_isUpdatingLanguage = false;
    }
    // 填充可用样式
    QStringList styles = QStyleFactory::keys();
    ui->comboBox_8->clear();
    for (const auto &style : styles)
        ui->comboBox_8->addItem(style, style);
    ui->comboBox_8->setCurrentText(data.theme);
    loadNotice();
}

SettingWidget::SettingWidget(QWidget *parent) : QWidget(parent), ui(new Ui::setting)
{
    ui->setupUi(this);
    // 检查更新
    m_versionChecker = new VersionChecker(this);
    // 语言获取
    m_langClient = new PyLang(this);
    // basic
    ui->horizontalSlider->setRange(80, 600);   // size
    ui->horizontalSlider_2->setRange(15, 120); // FPS
    ui->horizontalSlider_3->setRange(0, 100);  // volume

    // 初始化日志等级选择
    struct LogLevelItem
    {
        QString text;
        LogLevel value;
    };

    QVector<LogLevelItem> logLevels = {
        {tr("Debug"), LogLevel::Debug},
        {tr("Info"), LogLevel::Info},
        {tr("Warning"), LogLevel::Warning},
        {tr("Critical"), LogLevel::Critical},
        {tr("Fatal"), LogLevel::Fatal}};

    ui->comboBox_3->clear();
    for (const auto &item : logLevels)
    {
        ui->comboBox_3->addItem(item.text, static_cast<int>(item.value));
    }
    ui->comboBox_4->clear();
    for (const auto &item : TTSProviderList)
    {
        ui->comboBox_4->addItem(item.first, item.second);
    }
    ui->comboBox_7->clear();
    for (const auto &item : Translators)
    {
        ui->comboBox_7->addItem(item.first, item.second);
    }

    // 填充语言选择
    ui->comboBox_2->clear();
    QStringList codes = TranslationManager::instance()->availableLanguages();
    QMap<QString, QString> langDisplayNames;
    langDisplayNames["en_US"] = "English";
    langDisplayNames["zh_CN"] = "简体中文";
    for (const QString &code : codes)
    {
        ui->comboBox_2->addItem(langDisplayNames.value(code, code), code);
    }

    // 连接信号槽
    connectSignals();
    // 设置初始值
    setAllValues(DataManager::instance().getBasicData());
    setTTSConfig(DataManager::instance().getTTSConfig());
    setOpenWeatherData(DataManager::instance().getOpenWeatherData());
    setLlamaData(DataManager::instance().getLlamaData());

    onTTSProviderChanged();
    onTranslatorsChanged();
}

void SettingWidget::connectSignals()
{
    // basic
    connect(ui->pushButton_2, &QPushButton::clicked, this, &SettingWidget::selectModelPath);
    connect(ui->horizontalSlider, &QSlider::valueChanged, [&]()
            { ui->label_7->setText(QString::number(ui->horizontalSlider->value())); });
    connect(ui->label_7, &DoubleClickableLabel::doubleClicked, [&]()
            {
                ConfigData temp ;
                ui->horizontalSlider->setValue(temp.model_size); });
    connect(ui->horizontalSlider_2, &QSlider::valueChanged, [&]()
            { ui->label_8->setText(QString::number(ui->horizontalSlider_2->value())); });
    connect(ui->label_8, &DoubleClickableLabel::doubleClicked, [&]()
            {
                ConfigData temp ;
                ui->horizontalSlider_2->setValue(temp.FPS); });
    connect(ui->horizontalSlider_3, &QSlider::valueChanged, [&]()
            { ui->label_9->setText(QString::number(ui->horizontalSlider_3->value())); });
    connect(ui->label_9, &DoubleClickableLabel::doubleClicked, [&]()
            {
                ConfigData temp ;
                ui->horizontalSlider_3->setValue(temp.volume); });
    connect(ui->comboBox_3, &QComboBox::currentTextChanged, this, &SettingWidget::onLogLevelChanged);
    connect(ui->comboBox_8, &QComboBox::currentTextChanged, this, [this]()
            {
    QString style = ui->comboBox_8->currentData().toString();
    qApp->setStyle(style);
    if (style != qApp->style()->objectName())
    {
        qDebug() << "[Settings] current theme set to" << style;
        emit styleChanged(style);
    } });
    // color
    connect(ui->pushButton_17, &QPushButton::clicked, [&]()
            {
        //bg
        QString color = selectColor();
        if (color.isEmpty())return;
        ui->label_24->setText(color);
        ui->label_24->setStyleSheet(
            QString("background-color: %1;color: %2;").arg(color).arg(ui->label_25->text()));
        ui->label_25->setStyleSheet(
            QString("background-color: %1;color: %2;").arg(color).arg(ui->label_25->text())); });
    connect(ui->pushButton_18, &QPushButton::clicked, [&]()
            {
        // fg
        QString color = selectColor();
        if (color.isEmpty())return;
        ui->label_25->setText(color);
        ui->label_24->setStyleSheet(
            QString("background-color: %1;color: %2;").arg(ui->label_24->text()).arg(color));
        ui->label_25->setStyleSheet(
            QString("background-color: %1;color: %2;").arg(ui->label_24->text()).arg(color)); });
    // tray
    connect(ui->pushButton_19, &QPushButton::clicked, [&]()
            {
        QString color = selectColor();
        if (color.isEmpty())return;
        ui->label_34->setText(color);
        ui->label_34->setStyleSheet(QString("color: %1;").arg(color)); });
    connect(ui->pushButton_20, &QPushButton::clicked, [&]()
            {
        QString color = selectColor();
        if (color.isEmpty())return;
        ui->label_35->setText(color);
        ui->label_35->setStyleSheet(QString("color: %1;").arg(color)); });
    // bool
    connect(ui->checkBox, &QCheckBox::clicked, [&]()
            { startupSwitch(!ui->checkBox->isChecked()); });
    connect(ui->checkBox_3, &QCheckBox::checkStateChanged, [&]()
            { ui->widget_7->setVisible(ui->checkBox_3->isChecked()); });
    connect(ui->checkBox_4, &QCheckBox::checkStateChanged, [&]()
            { ui->widget_6->setVisible(ui->checkBox_4->isChecked()); });
    connect(ui->checkBox_12, &QCheckBox::checkStateChanged, [&]()
            { TrayIcon::instance()->switchMusicIcon(ui->checkBox_12->isChecked()); });
    // TTS
    connect(ui->comboBox_4, &QComboBox::currentTextChanged, this, &SettingWidget::onTTSProviderChanged);
    connect(ui->comboBox_7, &QComboBox::currentTextChanged, this, &SettingWidget::onTranslatorsChanged);
    connect(ui->pushButton_21, &QPushButton::clicked, [&]()
            {
                m_langClient->fetchProviders();
                m_langClient->fetchLanguages(); });
    connect(ui->pushButton_23, &QPushButton::clicked, [&]()
            {
        QString provider = ui->comboBox->currentText();
        QString language = ui->comboBox_5->currentText();
        QString text = ui->lineEdit_14->text();
        if (provider.isEmpty() || language.isEmpty() || text.isEmpty())
        {
            qDebug() << "[Settings] Translate test value is empty";
            NotificationWidget::showNotification(tr("Warning"), tr("Please fill in all fields first."), 5000, MessageType::Warning);
        }
        else
        {
            m_langClient->testTranslation(text, language, provider);
        } });
    connect(m_langClient, &PyLang::languagesReady, m_langClient, [this](const QList<QString> &langs)
            {
        qDebug() << "[Settings] Languages fetch successfully";
        ui->comboBox_5->clear();
        int index = 0;
        for (const auto &lang : langs)
        {
            ui->comboBox_5->addItem(lang, index);
            index++;
        } });

    connect(m_langClient, &PyLang::providersReady, m_langClient, [this](const QList<QString> &providers)
            {
        qDebug() << "[Settings] Providers fetch successfully";
        ui->comboBox->clear();
        int index = 0;
        for (const auto &provider : providers)
        {
            ui->comboBox->addItem(provider, index);
            index++;
        } });

    connect(ui->pushButton_24, &QPushButton::clicked, this, &SettingWidget::onChooseVoicevoxDict);
    connect(ui->pushButton_25, &QPushButton::clicked, this, &SettingWidget::onChooseVoicevoxModel);
    connect(ui->pushButton_26, &QPushButton::clicked, this, &SettingWidget::onTestVoicevox);

    // test Tencent
    connect(ui->pushButton_29, &QPushButton::clicked, [&]()
            {
                if(ui->lineEdit_24->text().isEmpty())
                {
                    NotificationWidget::showNotification(tr("Warning"), tr("Please fill in all fields first."), 5000, MessageType::Warning);
                }
                TTSConfig cfg = getTTSConfigValue();
                QString msg;
                TencentTranslator::testTranslate(cfg, ui->lineEdit_24->text(), &msg);
                ui->label_56->setText(msg); });

    // llama
    connect(ui->pushButton_15, &QPushButton::clicked, [&]()
            {
                // 打开文件选择对话框，支持多种文本格式（包括 .md, .txt, .json 等）
                QString path = QFileDialog::getOpenFileName(
                    this,
                    tr("Select Prompt File"),
                    QString(), // 默认目录，可设为最近使用的路径
                    tr("Text Files (*.txt *.md *.json);;All Files (*)"));

                if (path.isEmpty())
                    return; // 用户取消
                ui->lineEdit_13->setText(path);
                // 直接调用单例方法加载
                bool x = LlamaClient::instance()->loadSystemPromptFromFile(path);
                if (x)
                    NotificationWidget::showNotification(tr("Information"), tr("Loaded prompt from [%1] successfully.").arg(path));
                else
                    NotificationWidget::showNotification(tr("Warning"), tr("Failed to load prompt from [%1].").arg(path), 5000, MessageType::Warning); });
    // 链接跳转
    connect(ui->pushButton_7, &QPushButton::clicked, [&]()
            { launchByPath(DataManager::instance().const_config_data.openai_edge_tts_Voice_Samples); });
    connect(ui->pushButton_3, &QPushButton::clicked, [&]()
            { launchByPath(DataManager::instance().const_config_data.iFlytek_tts_url); });
    connect(ui->pushButton_4, &QPushButton::clicked, [&]()
            { launchByPath(DataManager::instance().const_config_data.openWeather_url); });
    // git-repo
    connect(ui->pushButton_8, &QPushButton::clicked, [&]()
            { launchByPath(DataManager::instance().const_config_data.website_link); });
    // Contributors
    connect(ui->pushButton_9, &QPushButton::clicked, [&]()
            { launchByPath(DataManager::instance().const_config_data.team_link); });

    // docs
    connect(ui->pushButton_16, &QPushButton::clicked, [&]()
            { launchByPath(DataManager::instance().const_config_data.docs_link); });

    // issues
    connect(ui->pushButton_6, &QPushButton::clicked, [&]()
            { launchByPath(DataManager::instance().const_config_data.feedback_link); });
    // text
    connect(ui->pushButton_12, &QPushButton::clicked, [&]()
            { initUserTextFile(); });
    //.
    connect(ui->pushButton_13, &QPushButton::clicked, [&]()
            { launchByPath("."); });
    // voiceFolder
    connect(ui->pushButton_5, &QPushButton::clicked, [&]()
            { launchByPath(DataManager::instance().const_config_data.VoiceFolder); });
    // reset
    connect(ui->pushButton_11, &QPushButton::clicked, this, &SettingWidget::resetSetting);
    // save
    connect(ui->pushButton, &QPushButton::clicked, [&]()
            {
        saveData();
        NotificationWidget::showNotification(tr("Information"), tr("Saved!\nWill take effect on next startup.")); });
    // 检查更新
    connect(ui->pushButton_10, &QPushButton::clicked, [&]()
            { m_versionChecker->checkVersionMatch(DataManager::instance().const_config_data.version); });
    connect(m_versionChecker, &VersionChecker::versionCheckResult, this, [this](const VersionCheckSummary &summary)
            {
        UpdateDialog dialog(summary, this);
        dialog.exec(); });

    // 语言选择
    connect(ui->comboBox_2, &QComboBox::currentIndexChanged, this, [this](int idx)
            {
        if (m_isUpdatingLanguage) return;
        m_isUpdatingLanguage = true;
        QString code = ui->comboBox_2->currentData().toString();
        TranslationManager::instance()->setLanguage(code);
        retranslateUI();
        m_isUpdatingLanguage = false; });

    connect(TranslationManager::instance(), &TranslationManager::languageChanged,
            this, [this](const QString &)
            { retranslateUI(); });

    // 组件初始状态
    ui->widget_6->hide();
    ui->widget_7->hide();
}

void SettingWidget::retranslateUI()
{
    saveData();
    ConfigData data = DataManager::instance().getBasicData();

    ui->retranslateUi(this);

    // comboBox_3 的条目是 tr() 字符串，必须重建
    ui->comboBox_3->clear();
    QVector<QPair<QString, LogLevel>> logLevels = {
        {tr("Debug"), LogLevel::Debug},
        {tr("Info"), LogLevel::Info},
        {tr("Warning"), LogLevel::Warning},
        {tr("Critical"), LogLevel::Critical},
        {tr("Fatal"), LogLevel::Fatal}};
    for (const auto &item : logLevels)
        ui->comboBox_3->addItem(item.first, static_cast<int>(item.second));

    setAllValues(data); // 恢复所有控件状态（包括 label_2、comboBox_3 索引等）
}

QSlider *SettingWidget::getHorizontalSlider()
{
    return ui->horizontalSlider;
}

void SettingWidget::saveData()
{
    ConfigData data = getAllValues();
    TTSConfig tts_data = getTTSConfigValue();
    OpenWeatherData weather_data = getOpenWeatherDataValue();
    LlamaData llm_data = getLlamaDataValue();
    // 保存配置
    DataManager::instance().writeData<ConfigData>(data);
    DataManager::writeTTSConfig(tts_data);
    DataManager::writeOpenWeatherData(weather_data);
    DataManager::writeLlamaData(llm_data);
    qDebug() << "[Settings] Save data success";
}

bool SettingWidget::checkStartupLink()
{
    // 获取启动文件夹路径
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString appData = env.value("APPDATA");

    if (appData.isEmpty())
    {
        appData = env.value("USERPROFILE");
        if (appData.isEmpty())
        {
            qWarning() << "[Settings] Cannot get startup folder path";
            return false;
        }
        appData.append("/AppData/Roaming");
    }

    QString startupFolder = appData + "/Microsoft/Windows/Start Menu/Programs/Startup";

    // 获取当前应用程序路径
    QString executablePath = QCoreApplication::applicationFilePath();
    if (executablePath.isEmpty())
    {
        qWarning() << "[Settings] Cannot get application path";
        return false;
    }

    // 构建快捷方式名称和路径
    QString shortcutName = QFileInfo(executablePath).baseName() + ".lnk";
    QString shortcutPath = QDir(startupFolder).filePath(shortcutName);
    if (!QFile::exists(shortcutPath))
    {
        qDebug() << "[Settings] Shortcut does not exist:" << shortcutPath;
        return false;
    }
    return true;
}
void SettingWidget::resetSetting()
{
    // 第一次确认
    const auto re = QMessageBox::question(
        this, tr("Confirmation"),
        tr("Are you sure you want to reset all settings?\nThis will restore all options to defaults."),
        QMessageBox::Yes | QMessageBox::No);
    if (re == QMessageBox::No)
        return;

    // 第二次确认：是否删除所有用户数据
    const auto re2 = QMessageBox::question(
        this, tr("Confirmation"),
        tr("Delete all user data (logs, cache, user folder, etc.) as well?\n\n"
           "\"Yes\" will clean everything and restart - all data will be lost permanently.\n"
           "\"No\" will only reset settings to defaults."),
        QMessageBox::Yes | QMessageBox::No);

    // 无论哪种情况，都移除启动项
    startupSwitch(true);

    if (re2 == QMessageBox::Yes)
    {
        // 删除程序目录下的 user、log 和 voice_files 文件夹
        const QString appDir = QApplication::applicationDirPath();
        QDir userDir(DataManager::instance().const_config_data.userFolder);
        QDir logDir(DataManager::instance().const_config_data.logFolder);
        QDir voiceDir(DataManager::instance().const_config_data.VoiceFolder);

        bool success = true;
        if (userDir.exists())
            success &= userDir.removeRecursively();
        if (logDir.exists())
            success &= logDir.removeRecursively();
        if (voiceDir.exists())
            success &= voiceDir.removeRecursively();

        // 删除 AppData 中的程序数据文件夹
        const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir appDataDir(dataDir);
        if (appDataDir.exists())
            success &= appDataDir.removeRecursively();

        if (!success)
        {
            qWarning() << "[Settings] Failed to delete some user data folders";
            NotificationWidget::showNotification(
                tr("Error"),
                tr("Some data folders could not be deleted. Please clean them manually and restart."),
                5000, NotificationWidget::Warning);
            return;
        }

        // 重启程序
        QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
        QApplication::quit();
    }
    else
    {
        // 仅重置配置文件
        const ConfigData new_data = ConfigData();
        setAllValues(new_data);
        DataManager::instance().writeData<ConfigData>(new_data);
        qDebug() << "[Settings] Reset setting to defaults";
        NotificationWidget::showNotification(tr("Information"), tr("Settings have been reset!"));
    }
}

void SettingWidget::startupSwitch(const bool flag)
{
    // 获取启动文件夹路径
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString appData = env.value("APPDATA");

    if (appData.isEmpty())
    {
        appData = env.value("USERPROFILE");
        if (appData.isEmpty())
        {
            qWarning() << "[Settings] Cannot get startup folder path";
        }
        appData.append("/AppData/Roaming");
    }

    QString startupFolder = appData + "/Microsoft/Windows/Start Menu/Programs/Startup";

    // 获取当前应用程序路径
    QString executablePath = QCoreApplication::applicationFilePath();
    if (executablePath.isEmpty())
    {
        qWarning() << "[Settings] Cannot get application path";
    }

    // 构建快捷方式名称和路径
    QString shortcutName = QFileInfo(executablePath).baseName() + ".lnk";
    QString shortcutPath = QDir(startupFolder).filePath(shortcutName);
    if (!ui->checkBox->isChecked() || flag)
    {
        // 移除启动项
        if (!QFile::exists(shortcutPath))
        {
            qDebug() << "[Settings] Shortcut does not exist:" << shortcutPath;
            NotificationWidget::showNotification(tr("Information"), tr("Shortcut no longer exists!"));
            return;
        }
        bool success = QFile::remove(shortcutPath);
        if (!success)
        {
            qWarning() << "[Settings] Cannot remove shortcut:" << shortcutPath;
            NotificationWidget::showNotification(tr("Warning"), tr("Failed to remove shortcut!"), 5000, MessageType::Warning);
            return;
        }
        qDebug() << "[Settings] Remove shortcut success:" << shortcutPath;
        NotificationWidget::showNotification(tr("Information"), tr("Shortcut removed!"));
    }
    else
    {
        // 添加启动项
        IShellLink *pShellLink = nullptr;
        IPersistFile *pPersistFile = nullptr;

        HRESULT hr = CoInitialize(nullptr);
        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink,
                                  (void **)&pShellLink);
            if (SUCCEEDED(hr))
            {
                pShellLink->SetPath(executablePath.toStdWString().c_str());
                pShellLink->SetWorkingDirectory(QFileInfo(executablePath).absolutePath().toStdWString().c_str());
                hr = pShellLink->QueryInterface(IID_IPersistFile, (void **)&pPersistFile);
                if (SUCCEEDED(hr))
                {
                    hr = pPersistFile->Save(shortcutPath.toStdWString().c_str(), TRUE);
                    pPersistFile->Release();
                }
                pShellLink->Release();
            }
            CoUninitialize();
        }

        if (FAILED(hr))
        {
            qWarning() << "[Settings] Cannot create shortcut:" << shortcutPath;
            NotificationWidget::showNotification(tr("Warning"), tr("Failed to create shortcut!"), 5000, MessageType::Warning);
        }
        else
        {
            qDebug() << "[Settings] Create shortcut success:" << shortcutPath;
            NotificationWidget::showNotification(tr("Information"), tr("Shortcut created!"));
        }
    }
}

QString SettingWidget::selectColor()
{
    // 创建颜色选择对话框
    QColorDialog dialog;
    dialog.setOption(QColorDialog::ShowAlphaChannel, true); // 启用透明度选项
    dialog.setCurrentColor(QColor(255, 0, 0, 128));         // 设置初始颜色（半透明红色）

    // 显示对话框并获取结果
    if (dialog.exec() == QDialog::Accepted)
    {
        QColor selectedColor = dialog.currentColor();
        QString colorStr = selectedColor.name(QColor::HexArgb);
        qDebug() << "[Settings] Chose color:" << colorStr;
        return colorStr;
    }
    return "";
}
SettingWidget::~SettingWidget()
{
    delete ui;
}
