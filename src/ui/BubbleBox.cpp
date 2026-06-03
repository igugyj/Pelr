
#include "BubbleBox.h"
#include <QCoreApplication>
#include <QApplication>
#include <QScreen>
#include <QTime>
#include <QDebug>
#include <ctime>
#include <qmediaplayer.h>
#include <QPainter>
#include <random>
#include <QSizePolicy>
#include <QUrl>
#include <QThread>
#include "voicegenerator.hpp"
#include "data.hpp"
#include <QString>
#include <QColor>
#include "NotificationWidget.h"
#include "loadText.h"

// 初始化静态成员变量
BubbleBox *BubbleBox::m_instance = nullptr;

BubbleBox *BubbleBox::instance(QLabel *parent)
{
    if (!m_instance)
    {
        m_instance = new BubbleBox(parent);
    }
    return m_instance;
}

BubbleBox::BubbleBox(QLabel *parent) : QLabel(parent)
{
    if (DataManager::instance().getBasicData().isTop)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowTransparentForInput | Qt::WindowStaysOnTopHint);
    }
    else
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowTransparentForInput);
    }
    int max_width = 300 * DataManager::instance().getBasicData().model_size / 150;
    int max_height = 500 * DataManager::instance().getBasicData().model_size / 150;
    setMaximumSize(max_width, max_height);
    // 设置透明背景属性
    setAttribute(Qt::WA_TranslucentBackground);
    setAlignment(Qt::AlignCenter);
    // 设置“显示时不激活”属性
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    QFont font = qApp->font();
    font.setPointSize((int)12 * (DataManager::instance().getBasicData().model_size / 150));
    font.setWeight(QFont::Normal);
    // font.setItalic(true);
    setFont(font);
    QString foreColor = DataManager::instance().getBasicData().color_bubble.forground;
    if (foreColor.isEmpty())
    {
        foreColor = "#ffffffff";
    }
    setStyleSheet(
        "color: " + foreColor + ";"
                                "border-radius:20px;"
                                "padding:8px;");
    setWordWrap(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    adjustSize();

    // 连接信号
    connect(VoiceGenerator::instance(), &VoiceGenerator::voiceGenerated,
            this, [&](const QString &filePath)
            {
                qDebug() << "[BubbleBox] Voice generated:" << filePath;
                VoiceGenerator::instance()->playVoice(filePath);
                setText(m_text);
                adjustSize();
                show();
                resetFadeTimer(); });

    connect(VoiceGenerator::instance(), &VoiceGenerator::errorOccurred,
            this, [&](const QString &error)
            {
                qDebug() << "[BubbleBox] Error:" << error;
                setText(m_text);
                adjustSize();
                show();
                resetFadeTimer(); });

    connect(LlamaClient::instance(), &LlamaClient::textGenerated,
            this, &BubbleBox::onRandomSentenceAI);
    connect(LlamaClient::instance(), &LlamaClient::errorOccurred,
            this, &BubbleBox::onRandomSentenceError);

    fadeTimer = new QTimer(this);
    fadeTimer->setSingleShot(true);
    fadeTimer->setInterval(8000); // 8s
    connect(fadeTimer, &QTimer::timeout, this, &BubbleBox::hide);
    hide();

    // 初始化当前时间
    now = "";
}

void BubbleBox::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿
    QString color = DataManager::instance().getBasicData().color_bubble.background;
    QColor backgroundColor(color);
    backgroundColor = !backgroundColor.isValid() ? QColor("#62afef") : backgroundColor;
    // 绘制半透明圆角矩形背景
    QRect rect = this->rect();
    painter.setBrush(backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, 20, 20);

    // 调用基类的paintEvent来绘制文本
    QLabel::paintEvent(event);
}

void BubbleBox::RandomSentence()
{
    const ConfigData basic = DataManager::instance().getBasicData();
    if (basic.isLLMGreeting && LlamaClient::instance()->isConfigured())
    {
        qInfo() << "[BubbleBox] RandomSentence: using LLM greeting";
        LlamaClient::instance()->generateRandomAsync(
            QStringLiteral(
                "Generate a short spontaneous sentence as yourself.\n"
                "Greet the user, make a casual remark, or comment on something.\n"
                "Vary the topic each time — don't repeat what you said before.\n"
                "Keep it under 60 characters.\n"
                "Never mention that you are an AI or language model.\n"
                "Match the language of your role description above.\n"),
            AI_RANDOM_ID);
        setThinkingText();
        return;
    }
    qInfo() << "[BubbleBox] RandomSentence: falling back to file";
    textSet(loadText("daily"));
}

QString BubbleBox::getPeriodText()
{
    QTime currentTime = QTime::currentTime();
    int hour = currentTime.hour();
    QString keyName;
    if (hour >= 6 && hour < 10)
    {
        keyName = "dawn";
    }
    else if (hour >= 10 && hour < 12)
    {
        keyName = "morning";
    }
    else if (hour >= 12 && hour < 14)
    {
        keyName = "noon";
    }
    else if (hour >= 14 && hour < 18)
    {
        keyName = "afternoon";
    }
    else if (hour >= 18 && hour < 20)
    {
        keyName = "dusk";
    }
    else
    {
        keyName = "night";
    }
    return loadText(keyName);
}

void BubbleBox::showTime()
{
    const QString time = QTime::currentTime().toString("hh:mm");
    QString period;
    if (isFirst)
    {
        period = getPeriodText(); // 获取当前时间段句子
        textSet(tr("%1\n现在是%2哦~").arg(period).arg(time));
        isFirst = false;
    }
    else if ((time.contains(":00") || time.contains(":30")) && time != this->now)
    {
        period = getPeriodText();
        textSet(tr("%1\n现在是%2哦~").arg(period).arg(time));
        const bool fg = DataManager::instance().getBasicData().isTrayHourAlarm;
        if (fg)
        {
            NotificationWidget::showNotification(
                DataManager::instance().Project_Name, tr("现在是%1").arg(time));
        }
        qDebug() << "[BubbleBox] Now:" << time << "isTrayHourAlarm:" << fg;
        this->now = time;
    }
}

QString BubbleBox::GetSystemTime()
{
    QTime currentTime = QTime::currentTime();
    return currentTime.toString("hh:mm");
}

void BubbleBox::onRandomSentenceAI(const QString &text, int id)
{
    if (id != AI_RANDOM_ID)
        return;
    qDebug() << "[BubbleBox] onRandomSentenceAI:" << text.left(50);
    textSet(text);
}

void BubbleBox::onRandomSentenceError(const QString &error, int id)
{
    if (id != AI_RANDOM_ID)
        return;
    qWarning() << "[BubbleBox] onRandomSentenceError:" << error;
    textSet(loadText("daily"));
}

void BubbleBox::setThinkingText()
{
    fadeTimer->stop();
    if (!DataManager::instance().getBasicData().isShowThinkingBubble)
        return;
    setText(tr("In response..."));
    adjustSize();
    show();
}

void BubbleBox::textSet(const QString &text)
{
    m_text = text.trimmed();
    if (!DataManager::instance().getBasicData().isSaying)
    {
        qDebug() << "[BubbleBox] No text-to-speech interface is used";
        setText(m_text);
        qInfo() << "[BubbleBox] BubbleBox:" << m_text;
        adjustSize();
        show();
        resetFadeTimer();
        return;
    }

    VoiceGenerator::instance()->generateVoice(DataManager::instance().getTTSConfig(), m_text);
}

void BubbleBox::resetFadeTimer()
{
    fadeTimer->stop();
    fadeTimer->start();
}

void BubbleBox::updateWindowLocation(int f_x, int f_y, int f_w, int f_h)
{
    QCoreApplication *coreapp = QCoreApplication::instance();
    QApplication *app = qobject_cast<QApplication *>(coreapp);
    if (!app)
        return;

    // 计算初始位置
    int x = f_x - f_w / 8;
    int y = f_y;

    // 位置未改变则不更新
    if (this->x() == x && this->y() == y)
        return;

    // 获取目标位置所在的屏幕
    QScreen *targetScreen = app->screenAt(QPoint(x, y));
    if (!targetScreen)
    {
        // 如果没有找到屏幕，使用主屏幕作为备选
        targetScreen = app->primaryScreen();
        if (!targetScreen)
            return;
    }

    // 获取屏幕的可用几何区域（考虑任务栏等系统UI）
    QRect screenGeometry = targetScreen->availableGeometry();

    // 窗口宽度
    int winWidth = width();
    int screenWidth = screenGeometry.width();
    int screenLeft = screenGeometry.left();

    // 水平边界检测
    if (x < screenLeft)
    {
        // 窗口左侧超出屏幕左边界
        x = x + f_w;
    }
    else if (x + winWidth > screenLeft + screenWidth)
    {
        // 窗口右侧超出屏幕右边界
        if (winWidth <= screenWidth)
        {
            // 窗口宽度小于等于屏幕宽度，可以完全放入屏幕
            x = screenLeft + screenWidth - winWidth;
        }
        else
        {
            // 窗口宽度大于屏幕宽度，只能尽可能显示
            x = screenLeft;
        }
    }

    // 垂直边界检测
    if (y < screenGeometry.top())
    {
        y = screenGeometry.top();
    }
    else if (y + height() > screenGeometry.bottom())
    {
        y = screenGeometry.bottom() - height();
    }

    move(x, y);
}

BubbleBox::~BubbleBox()
{
    // 清理静态实例指针
    m_instance = nullptr;
}
