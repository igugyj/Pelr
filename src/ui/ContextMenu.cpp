#include "ContextMenu.h"
#include "custommenu.h"
#include "launcherMenu.hpp"
#include "BubbleBox.h"
#include "todoNotify.hpp"
#include "ExtraMotionManager.h"
#include "getpowerstatus.h"
#include "weathermanager.h"
#include "MediaPlayerWidget.h"
#include "tray.h"
#include "TranslationManager.h"

void ContextMenu::init()
{
    m_switchListenerButton = new QPushButton(tr("Key Listener"), this);
    m_RandomSentenceButton = new QPushButton(tr("Say Something"), this);
    m_QuickStartButton = new QPushButton(tr("Launch"), this);
    m_SettingButton = new QPushButton(tr("Settings"), this);
    m_EmotionButton = new QPushButton(tr("EMO"), this);
    m_MediaButton = new QPushButton(tr("Media Player"), this);
    m_QuestionButton = new QPushButton(tr("Ask a Question"), this);

    m_QuestionMenu = new CustomMenu(this);
    m_askWeather = new QAction(tr("Weather"), m_QuestionMenu);
    m_askPowerStatus = new QAction(tr("Power Status"), m_QuestionMenu);
    m_askLatestNextTodoEvent = new QAction(tr("TODO"), m_QuestionMenu);

    qInfo() << "[ContextMenu] Init context menu";
};

ContextMenu::ContextMenu(QWidget *parent)
{
    init();
    m_QuickStartButton->setMenu(launcherMenu::instance());
    m_QuestionMenu->addActions({m_askLatestNextTodoEvent, m_askWeather, m_askPowerStatus});
    m_QuestionButton->setMenu(m_QuestionMenu);

    // 表情/动作控制菜单
    m_EmotionButton->setMenu(ExtraMotionManager::getInstance());

    setDefaultMsg();

    // 聊天
    connect(m_RandomSentenceButton, &QPushButton::clicked, [&]()
            { BubbleBox::instance()->RandomSentence(RandomSentenceMode::click); });
    // 媒体播放
    connect(m_MediaButton, &QPushButton::clicked, this, &ContextMenu::onPlayMedia);
    // 问讯菜单
    connect(m_askWeather, &QAction::triggered, this, &ContextMenu::onAskWeather);
    connect(m_askPowerStatus, &QAction::triggered, [&]()
            {
        std::vector<QString> powerStatus = getPowerStatus();
        if (!powerStatus.empty()) {
            QString msg = tr("Master, here is your PC's power status:\nAC: %1\nPercentage: %2%\nBattery State: %3").arg(
                powerStatus[0]).arg(powerStatus[1]).arg(powerStatus[2]);
            BubbleBox::instance()->textSet(msg);
        } });
    connect(m_askLatestNextTodoEvent, &QAction::triggered, [&]()
            { TodoNotify::instance().askLatestNextEvent(); });

    // 托盘功能
    connect(TrayIcon::instance()->action_mediaPlayer, &QAction::triggered, this, &ContextMenu::onPlayMedia);
    // 语言热切换
    connect(TranslationManager::instance(), &TranslationManager::languageChanged, this, &ContextMenu::retranslateUI);
    // 以一定次序添加按钮
    mainLayout->addWidget(m_SettingButton);
    mainLayout->addWidget(m_EmotionButton);
    // 如果有内容就添加到菜单
    if (launcherMenu::instance()->hasContent)
    {
        mainLayout->addWidget(m_QuickStartButton);
    }
    else
    {
        m_QuickStartButton->setEnabled(false); // 禁用
        m_QuickStartButton->hide();            // 隐藏
    }
    mainLayout->addWidget(m_switchListenerButton);
    mainLayout->addWidget(m_MediaButton);
    mainLayout->addWidget(m_RandomSentenceButton);
    mainLayout->addWidget(m_QuestionButton);
}

void ContextMenu::onAskWeather()
{
    // 获取单例实例
    WeatherManager *weatherManager = WeatherManager::instance();

    // API Key和城市名
    OpenWeatherData data = DataManager::instance().getOpenWeatherData();

    // 调用单例方法获取天气数据
    WeatherData weather = weatherManager->getWeatherData(data.city, data.api_key);
    QString msg;
    if (weather.error.isEmpty())
        msg = tr("%1, %2℃, %3, humidity: %4%.").arg(weather.city).arg(weather.temperature).arg(weather.description).arg(weather.humidity);
    else
        msg = weather.error;

    qDebug() << "[ContextMenu] " << msg;
    BubbleBox::instance()->textSet(msg);
}

void ContextMenu::onPlayMedia()
{
    MediaPlayerWidget::instance().setVisible(!MediaPlayerWidget::instance().isVisible());
    qDebug() << "[ContextMenu] MediaPlayerWidget visible: " << MediaPlayerWidget::instance().isVisible();
}
void ContextMenu::setDefaultMsg()
{
    // 把需要统一处理的按钮放进数组
    QPushButton *buttons[] = {
        m_switchListenerButton,
        m_RandomSentenceButton,
        m_QuickStartButton,
        m_SettingButton,
        m_EmotionButton,
        m_MediaButton,
        m_QuestionButton,
    };
    const size_t count = sizeof(buttons) / sizeof(buttons[0]);

    for (size_t i = 0; i < count; i++)
    {
        QPushButton *btn = buttons[i];
        connect(btn, &QPushButton::clicked, this, [btn]()
                { qDebug().noquote()
                      << QString("[ContextMenu] clicked: %1").arg(btn->text()); });
    }
}

void ContextMenu::retranslateUI()
{
    m_switchListenerButton->setText(tr("Key Listener"));
    m_RandomSentenceButton->setText(tr("Say Something"));
    m_QuickStartButton->setText(tr("Launch"));
    m_askWeather->setText(tr("Weather"));
    m_askPowerStatus->setText(tr("Power Status"));
    m_askLatestNextTodoEvent->setText(tr("TODO"));
    m_QuestionButton->setText(tr("Ask a Question"));
    m_SettingButton->setText(tr("Settings"));
    m_EmotionButton->setText(tr("EMO"));
    m_MediaButton->setText(tr("Media Player"));
    qDebug() << "[ContextMenu] Retranslate ui:" << typeid(*this).name();
}
