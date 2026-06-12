
#pragma once

#include <QLineEdit>

#include "data.hpp"
#include <QWidget>
#include "DoubleClickableLabel.h"
#include "TranslationManager.h"
#include "VersionChecker.h"
#include <QtWidgets/QSlider>
#include "pyLang.h"
#include "ui_setting.h"

namespace Ui
{
    class setting;
}

class SettingWidget : public QWidget
{
    Q_OBJECT
signals:
    void styleChanged(const QString &newStyle);

public:
    explicit SettingWidget(QWidget *parent = nullptr);

    ~SettingWidget();

    ConfigData getAllValues();

    void setAllValues(const ConfigData &data);

    void retranslateUI();

    void setTTSConfig(const TTSConfig &data) const;
    void setLlamaData(const LlamaData &llm) const;

    void setOpenWeatherData(const OpenWeatherData &data) const;

    TTSConfig getTTSConfigValue() const;
    LlamaData getLlamaDataValue() const;

    OpenWeatherData getOpenWeatherDataValue();

    void connectSignals();

    bool checkStartupLink();

    // horizontalSlider
    QSlider *getHorizontalSlider();

private slots:
    void saveData();

    void selectModelPath();

    QString selectColor();

    void resetSetting();

    void startupSwitch(bool flag = false);

    void onLogLevelChanged();

    void onTTSProviderChanged();
    void onTranslatorsChanged();
    void onChooseVoicevoxModel();
    void refreshVoicevoxStyles();
    void onChooseVoicevoxDict();
    void loadVoicevoxDict(const QString &dir);
    void onTestVoicevox();
    void loadNotice();

private:
    PyLang *m_langClient;
    VersionChecker *m_versionChecker;
    Ui::setting *ui;
    bool m_isUpdatingLanguage = false;
};
