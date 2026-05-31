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
using MessageType = NotificationWidget::MessageType;
void SettingWidget::onTranslatorsChanged()
{
    int index = ui->comboBox_7->currentIndex();
    qDebug() << "[Settings] Translator changed to" << index;
    QList<QGroupBox *> groupBoxes = {
        ui->groupBox_5, // libretranslate
        ui->groupBox,   // translators
        ui->groupBox_6, // Tencent
    };
    for (int i = 0; i < groupBoxes.size(); i++)
    {
        if (i == index)
        {
            groupBoxes[i]->setVisible(true);
        }
        else
        {
            groupBoxes[i]->setVisible(false);
        }
    }
};

void SettingWidget::onTTSProviderChanged()
{
    int index = ui->comboBox_4->currentIndex();
    qDebug() << "[Settings] TTS provider changed to" << index;
    QList<QGroupBox *> groupBoxes = {
        ui->groupBox_2, // OpenAI-Edge-TTS
        ui->groupBox_3, // iFlytek
        ui->groupBox_4, // voicevox
        ui->groupBox_7, // openai
    };
    for (int i = 0; i < groupBoxes.size(); i++)
    {
        if (i == index)
            groupBoxes[i]->setVisible(true);
        else
            groupBoxes[i]->setVisible(false);
    }
}
void SettingWidget::onLogLevelChanged()
{
    LogLevel level = static_cast<LogLevel>(ui->comboBox_3->currentData().toInt());
    write_log_level(level);
    qDebug() << "[Settings] Set logLevel to" << static_cast<int>(level);
}

void SettingWidget::selectModelPath()
{
    QString folder = "./Resources/";
    QDir dir(folder);
    // 确保文件夹存在
    if (!dir.exists())
        QDir().mkpath(folder);
    QString path = QFileDialog::getOpenFileName(this, tr("选择模型文件"), folder, "*.model3.json");
    if (!path.isEmpty())
    {
        ui->lineEdit->setText(path);
        QFileInfo file_info(path);
        QString model_dir = file_info.absolutePath();
        QString fileName = file_info.fileName();
        qDebug() << "[Settings] Model dir:" << model_dir << "fileName:" << fileName;
        LAppLive2DManager::GetInstance()->LoadModelFromPath(model_dir.toStdString() + "/", fileName.toStdString());
        // 保存配置 用以加载模型表情
        saveData();
    }
}
// 用户点击“浏览”模型文件按钮
void SettingWidget::onChooseVoicevoxModel()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择音声模型文件"),
        "voicevox_core",
        tr("VVM 文件 (*.vvm);;所有文件 (*)"));
    if (filePath.isEmpty())
        return;

    ui->lineEdit_16->setText(filePath);

    if (!VoicevoxTTS::instance().loadModel(filePath))
    {
        NotificationWidget::showNotification(
            tr("Warning"),
            tr("模型加载失败，请检查模型文件是否正确，并重新加载。"), 5000, MessageType::Warning);
        qDebug() << "[Settings] Model load failed:" << filePath;
        return;
    }

    refreshVoicevoxStyles();

    // 强制选中第一个有效风格（如果当前选中的无效）
    if (ui->comboBox_6->count() > 0 && ui->comboBox_6->currentData().toInt() < 0)
    {
        ui->comboBox_6->setCurrentIndex(0);
        qDebug() << "[Settings] Forced selection to first style, id="
                 << ui->comboBox_6->currentData().toInt();
    }

    NotificationWidget::showNotification(tr("Information"), tr("模型加载成功，风格列表已更新。"), 2000);
}

// 刷新风格下拉框（comboBox_6）
void SettingWidget::refreshVoicevoxStyles()
{
    const QString previousStyleText = ui->comboBox_6->currentText();
    ui->comboBox_6->blockSignals(true);
    ui->comboBox_6->clear();

    const auto speakers = VoicevoxTTS::instance().getSpeakers();
    if (speakers.isEmpty())
    {
        ui->comboBox_6->addItem(tr("(无可用风格)"), -1);
        ui->comboBox_6->blockSignals(false);
        qWarning() << "[Settings] No speakers/styles available after loading model";
        return;
    }

    int indexToSelect = -1;
    int currentIdx = 0;
    for (const auto &speaker : speakers)
    {
        for (const auto &style : speaker.styles)
        {
            const QString itemText = QStringLiteral("%1 - %2").arg(speaker.name, style.name);
            ui->comboBox_6->addItem(itemText, style.id);
            qDebug() << "[Settings] Added style:" << itemText << "id=" << style.id;
            if (!previousStyleText.isEmpty() && itemText == previousStyleText)
                indexToSelect = currentIdx;
            ++currentIdx;
        }
    }

    if (indexToSelect >= 0)
    {
        ui->comboBox_6->setCurrentIndex(indexToSelect);
        qDebug() << "[Settings] Restored previous style selection:" << previousStyleText;
    }
    else if (ui->comboBox_6->count() > 0)
    {
        ui->comboBox_6->setCurrentIndex(0);
        int firstId = ui->comboBox_6->currentData().toInt();
        qDebug() << "[Settings] No previous style match, selected first style with id:" << firstId;
    }

    ui->comboBox_6->blockSignals(false);
}

// 实际加载辞书：更新UI并调用单例初始化
void SettingWidget::loadVoicevoxDict(const QString &dir)
{
    ui->lineEdit_15->setText(dir);

    if (!VoicevoxTTS::instance().initialize(dir))
    {
        qWarning() << "[Settings] Failed to initialize VoicevoxTTS:" << dir;
        NotificationWidget::showNotification(tr("Warning"), tr("辞书加载失败，请检查路径是否正确，并确保该路径下有有效的字典文件"), 5000, NotificationWidget::Warning);
        return;
    }

    qDebug() << "[Settings] Voicevox dictionary initialized successfully:" << dir;

    QString currentModel = ui->lineEdit_16->text();
    if (!currentModel.isEmpty() && QFile::exists(currentModel))
    {
        if (VoicevoxTTS::instance().loadModel(currentModel))
        {
            refreshVoicevoxStyles();
            qDebug() << "[Settings] Model reloaded after dict change:" << currentModel;
        }
        else
        {
            qWarning() << "[Settings] Failed to reload model after dict change:" << currentModel;
        }
    }
}

// 浏览按钮槽：选择目录后调用 loadVoicevoxDict
void SettingWidget::onChooseVoicevoxDict()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择辞书目录"),
        "voicevox_core",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
    {
        loadVoicevoxDict(dir);
    }
}
void SettingWidget::onTestVoicevox()
{
    TTSConfig cfg = getTTSConfigValue();
    cfg.provider = 2; // 强制使用 VOICEVOX

    qDebug() << "[Settings] Testing Voicevox with:"
             << "model=" << cfg.voicevox_model
             << "dict=" << cfg.voicevox_dict_dir
             << "styleId=" << cfg.voicevox_style_id;

    // 应用配置并检查风格有效性
    if (!VoicevoxTTS::instance().applyConfig(cfg))
    {
        NotificationWidget::showNotification(
            tr("Warning"),
            tr("应用配置失败，请检查辞书目录和模型文件是否正确。"),
            5000, MessageType::Warning);
        return;
    }

    QVector<int> validIds = VoicevoxTTS::instance().getStyleIds();
    if (!validIds.contains(cfg.voicevox_style_id))
    {
        QString msg = tr("当前选中的风格无效（ID: %1），可用风格: %2")
                          .arg(cfg.voicevox_style_id)
                          .arg(validIds.isEmpty() ? tr("无") : QString::number(validIds.first()));
        NotificationWidget::showNotification(tr("Warning"), msg, 5000, MessageType::Warning);
        return;
    }

    QByteArray wav = VoicevoxTTS::instance().testSynthesis(cfg);
    if (wav.isEmpty())
    {
        NotificationWidget::showNotification(tr("Warning"), tr("语音合成失败，请检查日志。"), 5000, MessageType::Warning);
        return;
    }

    // 生成唯一临时文件名（时间戳 + 随机数）
    QString testFile = QDir::tempPath() + "/voicevox_test_" +
                       QString::number(QDateTime::currentMSecsSinceEpoch()) +
                       "_" + QString::number(QRandomGenerator::global()->generate()) +
                       ".wav";

    QFile file(testFile);
    if (!file.open(QIODevice::WriteOnly))
    {
        NotificationWidget::showNotification(tr("Warning"), tr("无法写入临时文件。"), 5000, MessageType::Warning);
        return;
    }
    file.write(wav);
    file.close();

    VoiceGenerator::instance()->playVoice(testFile);
    qDebug() << "[Settings] Test synthesis and playback completed.";
}
