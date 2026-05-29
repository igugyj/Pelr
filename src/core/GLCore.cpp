
#include "LAppDelegate.hpp" //一定要第一个导入
#include "LAppView.hpp"
#include "LAppPal.hpp"
#include "LAppLive2DManager.hpp"
#include "LAppModel.hpp" //模型加载-表情-动作控制
#include "LAppDefine.hpp"
#include <QMouseEvent>
#include <QtCore/Qt>
#include "GLCore.h"
#include "MediaPlayerWidget.h"
#include <complex>
#include <QDebug>
#include <QImage>
#include <QColor>
#include <QCoreApplication>
#include <QWindow>
#include <QScreen>
#include <QApplication>
#include <QPushButton>
#include "BubbleBox.h"
#include "getpowerstatus.h"
#include "weathermanager.h"
#include "todoNotify.hpp"
#include "ExtraMotionManager.h"
#include "launcherMenu.hpp"
#include "tray.h"
// 键盘监听相关
#include "globalinputlistener.h"
#include "convertcodetostring.h"
#include <QRandomGenerator>

#define RECORD_FILE "user/record.dat"
#define WINDOW_LOCATION_FILE "user/window_location.dat"
#define TODO_FEATURE_MSG "暂时不支持这个功能哟，试试别的功能吧！"

GLCore::GLCore(QWidget *parent) : QOpenGLWidget(parent)
{
    timer = new QTimer();
    PermanentTimer = new QTimer();
    inputCheckTimer = new QTimer();
    overlay = new KeyboardOverlay();
    listener = &GlobalInputListener::instance();
    contextMenu = new QMenu(this);
    menuWidget = new MenuWidget();
    modelChatBox = new ChatBoxOnModel();
    // keyCounterTimer = new QTimer();
    main_widget = new mainWidget();
    // main_widget->show();

    // 读取数据
    // recorder = new Recorder();
    // recorder->readBinaryData(RECORD_FILE, keyCounter.first, keyCounter.second);

    // 窗口标志
    if (DataManager::instance().getBasicData().isTop)
    {
        this->setWindowFlags(
            Qt::FramelessWindowHint |
            Qt::WindowStaysOnTopHint |
            Qt::Tool |
            Qt::WindowTransparentForInput |
            Qt::WindowDoesNotAcceptFocus);
    }
    else
    {
        this->setWindowFlags(
            Qt::FramelessWindowHint |
            Qt::Tool |
            Qt::WindowTransparentForInput |
            Qt::WindowDoesNotAcceptFocus);
    }
    this->setAttribute(Qt::WA_TranslucentBackground);

    connectSignals();
    // 鼠标跟踪
    if (DataManager::instance().getBasicData().isLookingMouse)
    {
        qDebug() << "[GLCore] Mouse tracking enabled";
        this->setMouseTracking(true);
    }
    LAppLive2DManager::SetDragStrength(DataManager::instance().getBasicData().LookingMouseStrength);
    int step = DataManager::instance().getBasicData().model_size; // 150;
    resize(4 * step, 3 * step);
    initContextMenu();
    overlay->show();
    move(1400, 300);
    retranslateUI();
}

void GLCore::checkMouseTransparency()
{
    // 获取当前鼠标位置
    QPoint globalPos = QCursor::pos();
    QPoint localPos = mapFromGlobal(globalPos);
    if (DataManager::instance().getBasicData().isLookingMouse)
    {
        LAppDelegate::GetInstance()->GetView()->OnTouchesMoved(localPos.x(), localPos.y());
    }
    // 如果鼠标在窗口外，不处理
    if (!rect().contains(localPos) || !isAllowDrag)
    {
        if (m_currentlyTransparent)
        {
            switchWindowTransparent(false);
            m_currentlyTransparent = false;
        }
        return;
    }
    // 鼠标位置的像素
    QImage image = grabFramebuffer();

    // 核心修复：将逻辑坐标转换为物理像素坐标（处理高分屏缩放）
    qreal dpr = this->devicePixelRatioF();
    QPoint physicalPos(qRound(localPos.x() * dpr), qRound(localPos.y() * dpr));

    // 越界保护
    if (physicalPos.x() < 0 || physicalPos.x() >= image.width() ||
        physicalPos.y() < 0 || physicalPos.y() >= image.height())
    {
        return;
    }

    QColor color = image.pixelColor(physicalPos);

    // 检查透明度
    bool shouldBeTransparent = (color.alpha() < 64); // 25%透明度阈值

    // 如果需要改变窗口的穿透状态
    if (shouldBeTransparent != m_currentlyTransparent)
    {
        switchWindowTransparent(shouldBeTransparent);
        m_currentlyTransparent = shouldBeTransparent;
    }
}

void GLCore::switchWindowTransparent(bool transparent)
{
    // 1 穿透 0 不穿透
    //  获取当前窗口的 QWindow 句柄
    QWindow *windowHandle = this->windowHandle();
    if (!windowHandle)
    {
        // 窗口句柄可能尚未创建，例如在窗口显示之前
        // 可以考虑延迟执行或使用其他方法
        qCritical() << "[GLCore] Failed to get QWindow handle. Window may not have been created yet.";
        return;
    }

    // 获取当前的窗口标志
    Qt::WindowFlags currentFlags = windowHandle->flags();

    if (transparent)
    {
        // 添加穿透标志
        currentFlags |= Qt::WindowTransparentForInput;
        currentFlags |= Qt::WindowDoesNotAcceptFocus;
    }
    else
    {
        // 移除穿透标志
        currentFlags &= ~Qt::WindowTransparentForInput;
        currentFlags &= ~Qt::WindowDoesNotAcceptFocus;
    }

    // 应用新的标志
    windowHandle->setFlags(currentFlags);

    // 建议调用 update() 来请求重绘，而不是 hide()/show()
    this->update();
}

void GLCore::initContextMenu()
{
    qInfo() << "[GLCore] Init context menu";
    // 实时显示键盘and鼠标按键状态 switch on/off
    switchListenerButton = new QPushButton(tr("按键监听"), this);
    connect(switchListenerButton, &QPushButton::clicked, this, &GLCore::switchListener);

    // 聊天
    RandomSentenceButton = new QPushButton(tr("说点什么"), this);
    connect(RandomSentenceButton, &QPushButton::clicked, [&]()
            { BubbleBox::instance()->RandomSentence(); });

    // 启动 把Quick Tray的功能移植到这里
    QuickStartButton = new QPushButton(tr("启动"), this);
    launcherMenu *launcher_menu = launcherMenu::instance(this);
    QuickStartButton->setMenu(launcher_menu);

    // 询问天气（按钮）/询问电源状态/询问按键数（废弃）
    QMenu *QuestionMenu = new CustomMenu(this);

    askWeather = new QAction(tr("天气"), QuestionMenu);
    connect(askWeather, &QAction::triggered, [&]()
            { onAskWeather(); });
    askPowerStatus = new QAction(tr("电源状态"), QuestionMenu);
    connect(askPowerStatus, &QAction::triggered, [&]()
            {
        std::vector<QString> powerStatus = getPowerStatus();
        if (!powerStatus.empty()) {
            QString msg = tr("主人，这是您电脑目前的电源状态：\nAC: %1\nPercentage: %2%\nBattery State: %3").arg(
                powerStatus[0]).arg(powerStatus[1]).arg(powerStatus[2]);
            BubbleBox::instance()->textSet(msg);
        } });
    // 询问最近一次待办事项
    askLatestNextTodoEvent = new QAction(tr("TODO"), QuestionMenu);
    connect(askLatestNextTodoEvent, &QAction::triggered, [&]()
            { TodoNotify::instance().askLatestNextEvent(); });
    QuestionMenu->addActions({askLatestNextTodoEvent, askWeather, askPowerStatus});
    QuestionButton = new QPushButton(tr("问个问题"), this);
    QuestionButton->setMenu(QuestionMenu);

    // 设置界面
    SettingButton = new QPushButton(tr("界面"), this);
    connect(SettingButton, &QPushButton::clicked, [&]()
            {
        if (main_widget->isHidden()) {
            main_widget->show(); // 显示界面
            BubbleBox::instance()->hide();
            menuWidget->hide();
            modelChatBox->hide();
            // 更新数据
            main_widget->Widget_Todo->loadAllData();
            qDebug() << "[GLCore] Show main_widget";
        } else {
            main_widget->hide(); // 隐藏界面
            qDebug() << "[GLCore] Hide main_widget";
        } });
    // 表情/动作控制菜单
    EmotionButton = new QPushButton(tr("EMO"), this);
    EmotionButton->setMenu(ExtraMotionManager::getInstance());

    // 媒体播放
    MediaButton = new QPushButton(tr("媒体播放"), this);
    connect(MediaButton, &QPushButton::clicked, this, &GLCore::onPlayMedia);
    // 以一定次序添加按钮
    menuWidget->mainLayout->addWidget(SettingButton);
    menuWidget->mainLayout->addWidget(EmotionButton);
    // 如果有内容就添加到菜单
    if (launcherMenu::instance()->hasContent)
    {
        menuWidget->mainLayout->addWidget(QuickStartButton);
    }
    else
    {
        QuickStartButton->setEnabled(false); // 禁用
        QuickStartButton->hide();            // 隐藏
    }
    menuWidget->mainLayout->addWidget(switchListenerButton);
    menuWidget->mainLayout->addWidget(MediaButton);
    menuWidget->mainLayout->addWidget(RandomSentenceButton);
    menuWidget->mainLayout->addWidget(QuestionButton);
}

void GLCore::connectSignals()
{
    // 更新频率
    connect(timer, &QTimer::timeout, this, [=]()
            {
                update();
                BubbleBox::instance()->updateWindowLocation(x(), y(), width(), height());
                modelChatBox->updateWindowLocation(x(), y(), width(), height());
                checkFocus();
                if (DataManager::instance().getBasicData().isTop)
                    raise(); // 提升窗口
            });
    timer->start((1.0 / DataManager::instance().getBasicData().FPS) * 1000.0); // 30fps
    connect(PermanentTimer, &QTimer::timeout, this, [&]()
            {
        // 报时
        if (DataManager::instance().getBasicData().isHourAlarm && !isHidden()) BubbleBox::instance()->showTime();
        TodoNotify::instance().todoNotify(); });
    PermanentTimer->start(1000); // 1s
    // 键盘事件槽连接
    connect(listener, &GlobalInputListener::keyPressed, [&](int code, ModifierKeys mods)
            {
        QString keyStr = keyCodeToKeyString(code);
        QString modStr = modifiersToString(mods);

        // 【新增逻辑】：如果当前按下的正是修饰键本身（如单独按下 Ctrl 或 Shift）
        if (isModifierKeyCode(code)) {
            QString currentMod = modStr;
            // 兜底：如果系统状态还未及时更新，手动赋予对应的修饰键名称
            if (currentMod.isEmpty()) {
                if (code == VK_SHIFT || code == VK_LSHIFT || code == VK_RSHIFT) currentMod = "Shift";
                else if (code == VK_CONTROL || code == VK_LCONTROL || code == VK_RCONTROL) currentMod = "Ctrl";
                else if (code == VK_MENU || code == VK_LMENU || code == VK_RMENU) currentMod = "Alt";
                else if (code == VK_LWIN || code == VK_RWIN) currentMod = "Win";
            }
            if (!currentMod.isEmpty()) {
                overlay->onSpecialKey(currentMod); // 作为特殊键独占显示
            }
            return;
        }

        if (keyStr.isEmpty() && modStr.isEmpty()) return;

        QString fullStr = modStr.isEmpty() ? keyStr : (modStr + (keyStr.isEmpty() ? "" : " + ") + keyStr);

        if (isSpecialKey(code, mods)) {
            overlay->onSpecialKey(fullStr);
        } else {
            overlay->onNormalKey(fullStr);
        } });

    // 鼠标按键
    connect(listener, &GlobalInputListener::mouseReleased,
            [&](MouseButton btn, int x, int y, ModifierKeys mods)
            {
                Q_UNUSED(x);
                Q_UNUSED(y); // 坐标在此项目中用于展示按键无需使用
                QString btnStr = mouseCodeToString(btn);
                QString modStr = modifiersToString(mods);
                QString fullStr = modStr.isEmpty() ? btnStr : (modStr + " + " + btnStr);
                overlay->onSpecialKey(fullStr);
            });
    // 定时检查鼠标透明度
    inputCheckTimer->setInterval(50);
    connect(inputCheckTimer, &QTimer::timeout, this, &GLCore::checkMouseTransparency);

    inputCheckTimer->start();
    if (DataManager::instance().getBasicData().isListening)
    {
        qDebug() << "[GLCore] Key listening enabled";
        // 开始监听
        if (!listener->startListening())
        {
            qCritical() << "[GLCore] Failed to start global input listening";
        }
        else
        {
            qDebug() << "[GLCore] Global input listener is running.";
        }
    }
    // 定时说话
    randomSentenceTimer = new QTimer();
    randomSentenceTimer->setSingleShot(true);
    connect(randomSentenceTimer, &QTimer::timeout, [&]()
            {
        if (!isHidden()) {
            BubbleBox::instance()->RandomSentence();
        }
        // 重置定时器
        int minTime = DataManager::instance().getBasicData().RandomInterval.first;
        int maxTime = DataManager::instance().getBasicData().RandomInterval.second;
        int randomTime = QRandomGenerator::global()->bounded(maxTime - minTime + 1) + minTime; //15-25min
        qInfo() << "[GLCore] Next random sentence in " << QString::number(randomTime * 60 * 1000) << " s";
        randomSentenceTimer->start(randomTime * 60 * 1000); });
    if (DataManager::instance().getBasicData().isRandomSpeech)
    {
        qDebug() << "[GLCore] Random speech enabled";
        randomSentenceTimer->start(10 * 60 * 1000); // 3min
    }
    /*
        keyCounterTimer->setInterval(60000); // 1min
        connect(keyCounterTimer, &QTimer::timeout, [&]() {
                    recorder->writeBinaryData(RECORD_FILE, keyCounter.first, keyCounter.second);
                }
        );
        keyCounterTimer->start();
        */
    // 实时预览模型
    // load model //void SettingWidget::selectModelPath()
    //  connect(main_widget->Widget_Setting->ui->lineEdit, &QLineEdit::textChanged, this, &GLCore::loadModel);
    // size
    connect(main_widget->Widget_Setting->getHorizontalSlider(), &QSlider::valueChanged, [&]()
            {
        int var = main_widget->Widget_Setting->getHorizontalSlider()->value();
        resize(4 * var, 3 * var); });
    // 退出时记录窗口位置
    connect(TrayIcon::instance()->action_quit, &QAction::triggered, [&]()
            {
        saveWindowLocation();
        if (m_watcher.isRunning())
            m_watcher.cancel();
        qApp->quit(); });
    // 重置window位置
    connect(TrayIcon::instance()->action_resetWinLoc, &QAction::triggered, this, &GLCore::resetLocation);
    // 显示界面
    connect(TrayIcon::instance()->action_showWin, &QAction::triggered, this->main_widget, &QWidget::show);
    // 静默模式
    connect(TrayIcon::instance()->action_silentMode, &QAction::triggered, this, &GLCore::silentMode);
    // 按键监听
    connect(TrayIcon::instance()->action_keyListener, &QAction::triggered, this, &GLCore::switchListener);

    // 拖动窗口
    connect(TrayIcon::instance()->action_switchDrag, &QAction::triggered, this, &GLCore::switchDragStatus);
    // 播放媒体
    connect(TrayIcon::instance()->action_mediaPlayer, &QAction::triggered, this, &GLCore::onPlayMedia);
    connect(TrayIcon::instance(), &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason)
            {
        // 判断是否为双击动作
        if (reason == QSystemTrayIcon::DoubleClick) {
            if (main_widget->isHidden()) {
                main_widget->show();
            } else {
                main_widget->hide();
            }
        } });

    // 启动启动项
    if (DataManager::instance().getBasicData().isStartStar)
    {
        qDebug() << "[GLCore] Starting app in star category";
        startRunStarIfPoweredInThread();
    }
}

void GLCore::switchListener()
{
    if (listener->isListening())
    {
        listener->stopListening();
        QString msg = "key listening disabled";
        if (!isHidden())
            BubbleBox::instance()->textSet(msg);
        qDebug() << "[GLCore] " << msg;
        overlay->hide();
        TrayIcon::instance()->action_keyListener->setChecked(false);
    }
    else
    {
        listener->startListening();
        QString msg = "key listening enabled";
        if (!isHidden())
            BubbleBox::instance()->textSet(msg);
        qDebug() << "[GLCore] " << msg;
        overlay->show();
        TrayIcon::instance()->action_keyListener->setChecked(true);
    }
}

void GLCore::silentMode()
{
    if (isSilentMode)
    {
        show();
        timer->start();
        inputCheckTimer->start();
        if (DataManager::instance().getBasicData().isRandomSpeech)
            randomSentenceTimer->start();
        if (DataManager::instance().getBasicData().isListening)
            listener->startListening();
        qDebug() << "[GLCore] Silent mode off";
        isSilentMode = false;
    }
    else
    {
        hide();
        timer->stop();
        inputCheckTimer->stop();
        randomSentenceTimer->stop();
        BubbleBox::instance()->hide();
        menuWidget->hide();
        modelChatBox->hide();
        qDebug() << "[GLCore] Silent mode on";
        isSilentMode = true;
    }
    TrayIcon::instance()->action_silentMode->setChecked(isSilentMode);
}

void GLCore::onAskWeather()
{
    // 获取单例实例
    WeatherManager *weatherManager = WeatherManager::instance();

    // API Key和城市名
    OpenWeatherData data = DataManager::instance().getOpenWeatherData();

    // 调用单例方法获取天气数据
    WeatherData weather = weatherManager->getWeatherData(data.city, data.api_key);
    QString msg;
    if (weather.error.isEmpty())
    {
        /*
        qDebug() << "城市：" << weather.city;
        qDebug() << "温度：" << weather.temperature << "℃";
        qDebug() << "天气：" << weather.description;
        qDebug() << "湿度：" << weather.humidity << "%";*/
        msg = tr("%1, %2℃, %3, humidity: %4%.").arg(weather.city).arg(weather.temperature).arg(weather.description).arg(weather.humidity);
    }
    else
    {
        msg = weather.error;
    }
    qDebug() << "[GLCore] " << msg;
    BubbleBox::instance()->textSet(msg);
}

void GLCore::startRunStarIfPoweredInThread()
{
    if (m_watcher.isRunning())
    {
        qWarning() << "[GLCore] Background task already running, skipping";
        return;
    }
    if (DataManager::instance().getBasicData().StarCheckTime < 0)
    {
        qWarning() << "[GLCore] Check time is negative, skipping";
        return;
    }
    QString title = DataManager::instance().Project_Name + " " + QTime::currentTime().toString("hh:mm:ss");
    // 如果开机时长大于指定时长，则return
    if (isSystemUptimeExceeds(DataManager::instance().getBasicData().StarCheckTime))
    {
        TrayIcon::showMessage(
            title,
            tr("开机时间过长，不启动启动项"));
        return;
    }
    // 显示提示
    TrayIcon::showMessage(
        title,
        tr("将在 %1 分后启动启动项").arg(DataManager::instance().getBasicData().StarRunTimeout));

    // 设置完成信号与槽的连接
    connect(&m_watcher, &QFutureWatcher<void>::finished, this, &GLCore::onRunStarIfPoweredFinished);

    // 启动任务，运行在子线程中
    QFuture<void> future = QtConcurrent::run([this]()
                                             { runStarIfPowered(); });
    m_watcher.setFuture(future);
}

void GLCore::onPlayMedia()
{
    MediaPlayerWidget::instance().setVisible(!MediaPlayerWidget::instance().isVisible());
    qDebug() << "[GLCore] MediaPlayerWidget visible: " << MediaPlayerWidget::instance().isVisible();
}

void GLCore::onRunStarIfPoweredFinished()
{
    // 任务完成后进行清理
    m_watcher.disconnect();
    qDebug() << "[GLCore] Task completed and resources cleaned.";
}

void GLCore::runStarIfPowered()
{
    const int wt = DataManager::instance().getBasicData().StarRunTimeout * 60;
    if (wt < 0)
    {
        qDebug() << "[GLCore] StarRunTimeout is negative, skip.";
        return;
    }
    qDebug() << "[GLCore] Run star if powered, wait" << wt << "s";
    // 提前拷贝 future，避免访问 m_watcher 时对象已析构
    QFuture<void> future = m_watcher.future();
    for (int i = 0; i < wt; ++i)
    {
        if (future.isCanceled())
        {
            qDebug() << "[GLCore] RunStarIfPowered cancelled";
            return;
        }
        QThread::sleep(1);
    }
    QList<MenuData> menu_data = DataManager::instance().getMenuData();
    std::vector<QString> powerStatus = getPowerStatus();
    if (powerStatus.size() < 1 || powerStatus[0] != "Online (AC)")
        return;
    for (MenuData &item : menu_data)
    {
        if (item.category == "Star")
        {
            launchByPath(item.path);
            QThread::sleep(3);
        }
    }
}

void GLCore::switchDragStatus()
{
    isAllowDrag = !isAllowDrag;
    QString msg;
    msg = isAllowDrag ? "drag enabled" : "drag disabled";
    qDebug() << "[GLCore] " << msg;
    switchWindowTransparent(isAllowDrag);
    TrayIcon::instance()->action_switchDrag->setChecked(!isAllowDrag);
}

void GLCore::checkFocus()
{
    QWidget *focusWidget = QApplication::focusWidget();
    // qDebug() << "focusWidget:" << &focusWidget;

    isFocused = focusWidget != nullptr;
    if (!isFocused && menuWidget->isVisible())
    {
        menuWidget->hide();
    }
}

// unused
void GLCore::loadModel()
{
    QString model_path = DataManager::instance().getBasicData().model_path;
    // QString msg;
    if (model_path.isEmpty())
    {
        qCritical() << "[GLCore] Model path is empty";
        model_path = "Resources/Hiyori/Hiyori.model3.json";
    }
    QFileInfo file_info(model_path);
    if (!file_info.exists())
    {
        qCritical() << "[GLCore] Model file not exists: " << model_path;
        model_path = "Resources/Hiyori/Hiyori.model3.json";
        file_info.setFile(model_path);
    }
    QString dir = file_info.absolutePath();
    QString fileName = file_info.fileName();
    qDebug() << "[GLCore] Model dir: " << dir << " fileName: " << fileName;
    LAppLive2DManager::GetInstance()->LoadModelFromPath(dir.toStdString() + "/", fileName.toStdString());
}

// 保存窗口位置
void GLCore::saveWindowLocation()
{
    // 如果记录窗口位置选项关闭，则不保存
    if (!DataManager::instance().getBasicData().isRecordWindowLocation)
        return;
    QFile file(WINDOW_LOCATION_FILE);
    if (!file.open(QIODevice::WriteOnly))
    {
        // 无法打开文件进行写入
        QMessageBox::critical(nullptr, "Error", "写入数据失败！");
        qCritical() << "[GLCore] Write data failed: can not open file" << WINDOW_LOCATION_FILE;
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_15); // 设置流版本以确保兼容性

    // 写入数据
    out << x() << y();
    file.close();
}

// 加载窗口位置
void GLCore::loadWindowLocation()
{
    // 如果记录窗口位置选项关闭，则不加载
    if (!DataManager::instance().getBasicData().isRecordWindowLocation)
        return;
    QFile file(WINDOW_LOCATION_FILE);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[GLCore] Read data failed: can not open file" << WINDOW_LOCATION_FILE;
        return; // 文件不存在或无法打开，返回空列表
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_11); // 设置流版本以确保兼容性
    int x, y;
    in >> x >> y;
    file.close();
    qDebug() << "[GLCore] Load window location: " << x << " " << y;
    // 移动窗口到指定位置
    move(x, y);
}

void GLCore::resetLocation()
{
    // 重置窗口大小
    int step = DataManager::instance().getBasicData().model_size;
    resize(4 * step, 3 * step);
    // 获取主屏幕可用尺寸
    QCoreApplication *coreapp = QCoreApplication::instance();
    QApplication *app = qobject_cast<QApplication *>(coreapp);
    QScreen *screen = app->screens().at(0);
    QRect screenRect = screen->availableGeometry();
    // 计算窗口位置
    int x = (screenRect.width() - width()) / 2;
    int y = (screenRect.height() - height()) / 2;
    // 重置位置
    move(x, y);
}

void GLCore::mousePressEvent(QMouseEvent *event)
{
    LAppDelegate::GetInstance()->GetView()->OnTouchesBegan(event->position().x(), event->position().y());
    // 按住鼠标开始拖动
    if (event->button() == Qt::LeftButton)
    {
        left_button_down = true;
        dragStartPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
    }

    if (event->button() == Qt::RightButton)
    {
        right_button_down = true;
        if (menuWidget->isHidden())
        {
            menuWidget->showNearMouse();
        }
        else
        {
            menuWidget->hide();
        }
    }
    if (event->button() == Qt::MiddleButton)
    {
        event->accept();
    }
}

void GLCore::mouseMoveEvent(QMouseEvent *event)
{
    LAppDelegate::GetInstance()->GetView()->OnTouchesMoved(event->position().x(), event->position().y());
    // 按住鼠标移动时拖动窗口
    if (left_button_down && (event->buttons() & Qt::LeftButton) && isAllowDrag)
    {
        move(event->globalPosition().toPoint() - dragStartPos);
        // keyLabel->updateWindowLocation(this->x(), this->y(), width(), height());
    }
}

void GLCore::handleClick(const QPoint &localPos)
{
    if (!rect().contains(localPos))
        return;

    QImage frame = grabFramebuffer();

    // 核心修复：将逻辑坐标转换为物理像素坐标（处理高分屏缩放）
    qreal dpr = this->devicePixelRatioF();
    QPoint physicalPos(qRound(localPos.x() * dpr), qRound(localPos.y() * dpr));

    // 越界保护
    if (physicalPos.x() < 0 || physicalPos.x() >= frame.width() ||
        physicalPos.y() < 0 || physicalPos.y() >= frame.height())
        return;

    QColor color = frame.pixelColor(physicalPos);

    // 如果是透明区域，则忽略点击
    if (color.alpha() < 64)
        return;

    LAppModel *model = LAppLive2DManager::GetInstance()->GetModel(0);
    if (model)
        model->StartRandomMotion(LAppDefine::MotionGroupTapBody,
                                 LAppDefine::PriorityNormal, nullptr, nullptr);
}

void GLCore::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        handleClick(event->position().toPoint());
        left_button_down = false;
    }

    // 停止拖拽（眼神跟随用）
    LAppLive2DManager::GetInstance()->OnDrag(0.0f, 0.0f);

    if (event->button() == Qt::RightButton)
    {
        right_button_down = false;
    }
}

void GLCore::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        modelChatBox->setVisible(!modelChatBox->isVisible());
        qDebug() << "[GLCore] Model chat box visible: " << modelChatBox->isVisible();
    }
}

void GLCore::wheelEvent(QWheelEvent *event)
{
}

void GLCore::initializeGL()
{
    LAppDelegate::GetInstance()->Initialize(this);
}

void GLCore::paintGL()
{
    LAppDelegate::GetInstance()->update();
}

void GLCore::resizeGL(int w, int h)
{
    LAppDelegate::GetInstance()->resize(w, h);
}

void GLCore::retranslateUI()
{
    if (!switchListenerButton)
    {
        qDebug() << "[GLCore] UI part is null, can not retranslate ui by " << typeid(*this).name();
        return;
    }
    switchListenerButton->setText(tr("按键监听"));
    RandomSentenceButton->setText(tr("说点什么"));
    QuickStartButton->setText(tr("启动"));
    askWeather->setText(tr("天气"));
    askPowerStatus->setText(tr("电源状态"));
    askLatestNextTodoEvent->setText(tr("TODO"));
    QuestionButton->setText(tr("问个问题"));
    SettingButton->setText(tr("界面"));
    EmotionButton->setText(tr("EMO"));
    MediaButton->setText(tr("媒体播放"));
    qDebug() << "[GLCore] Retranslate ui:" << typeid(*this).name();
}

void GLCore::closeEvent(QCloseEvent *event)
{
    saveWindowLocation();
    if (m_watcher.isRunning())
        m_watcher.cancel();
    event->accept();
    qApp->quit();
}

void GLCore::hideEvent(QHideEvent *event)
{
    saveWindowLocation();
    event->accept();
}

void GLCore::showEvent(QShowEvent *event)
{
    loadWindowLocation();
    event->accept();
}
