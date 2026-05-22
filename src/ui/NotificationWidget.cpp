#include <iostream>

#include "notificationwidget.h"
#include <QGraphicsDropShadowEffect>
#include <QStyle>
#include <QThread>
#include <QMetaObject>
#include <QDebug>

NotificationWidget *NotificationWidget::m_instance = nullptr;

NotificationWidget *NotificationWidget::instance() {
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
    return new NotificationWidget();
}

void NotificationWidget::showNotification(
    const QString &title,
    const QString &message,
    int duration,
    MessageType type,
    std::function<void()> clickCallback) {
    // 获取实例
    NotificationWidget *inst = instance();

    // 如果当前是UI线程,直接调用
    if (QThread::currentThread() == QApplication::instance()->thread()) {
        inst->showNotificationInternal(title, message, duration, type, clickCallback);
    } else {
        // 否则安排到UI线程执行
        // 注意:不要在Lambda中再次调用instance()
        QMetaObject::invokeMethod(inst, [inst, title, message, duration, type, clickCallback]() {
            inst->showNotificationInternal(title, message, duration, type, clickCallback);
        }, Qt::QueuedConnection);
    }
}

NotificationWidget::NotificationWidget(QWidget *parent)
    : QWidget(parent)
      , m_appIconLabel(nullptr)
      , m_titleLabel(nullptr)
      , m_iconLabel(nullptr)
      , m_messageLabel(nullptr)
      , m_hideTimer(nullptr)
      , m_clickCallback(nullptr) {
    setupUI();

    // 设置窗口属性
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);

    // 创建自动隐藏计时器
    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, &QTimer::timeout, this, &NotificationWidget::hide);
}

NotificationWidget::~NotificationWidget() {
    // 单例不需要手动删除
}

void NotificationWidget::setupUI() {
    // 获取屏幕尺寸
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    // 设置窗口宽度为屏幕的1/4，最小宽度400
    int windowWidth = qMax(400, screenGeometry.width() / 4);
    setFixedWidth(windowWidth);

    // 设置背景为透明，在paintEvent中绘制圆角背景
    setStyleSheet("background: transparent;");

    // 创建主布局 - 增加边距使其更美观
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(12);

    // 创建标题布局
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(12);

    // 左上角应用图标
    m_appIconLabel = new QLabel(this);
    m_appIconLabel->setFixedSize(28, 28);
    m_appIconLabel->setPixmap(getAppIcon());
    m_appIconLabel->setScaledContents(true);
    m_appIconLabel->setStyleSheet("background: transparent; padding: 2px;");
    m_appIconLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_appIconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);

    // 标题标签
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #333; padding: 2px 0;");
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titleLayout->addWidget(m_titleLabel, 1, Qt::AlignLeft | Qt::AlignVCenter);

    mainLayout->addLayout(titleLayout);

    // 创建内容布局
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 4, 0, 0);
    contentLayout->setSpacing(16);

    // 消息类型图标
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(64, 64);
    m_iconLabel->setScaledContents(true);
    m_iconLabel->setStyleSheet("background: transparent; padding: 4px;");
    contentLayout->addWidget(m_iconLabel, 0, Qt::AlignTop | Qt::AlignLeft);

    // 消息内容
    m_messageLabel = new QLabel(this);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setStyleSheet("font-size: 18px; color: #555; line-height: 1.5; padding: 8px 0;");
    m_messageLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    contentLayout->addWidget(m_messageLabel, 1);

    mainLayout->addLayout(contentLayout);

    // 添加柔和的阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);
    shadow->setXOffset(0);
    shadow->setYOffset(4);
    shadow->setColor(QColor(0, 0, 0, 30));
    setGraphicsEffect(shadow);

    // 设置窗口固定大小策略
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
}

void NotificationWidget::updatePosition() {
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    // 计算位置：顶部居中，距离顶端20像素
    int x = (screenGeometry.width() - width()) / 2;
    int y = 20;

    move(x, y);
}

QPixmap NotificationWidget::getIconForType(MessageType type) {
    QStyle *style = QApplication::style();
    QIcon icon;
    QSize iconSize(64, 64);

    switch (type) {
        case Information:
            icon = style->standardIcon(QStyle::SP_MessageBoxInformation);
            break;
        case Warning:
            icon = style->standardIcon(QStyle::SP_MessageBoxWarning);
            break;
        case Critical:
            icon = style->standardIcon(QStyle::SP_MessageBoxCritical);
            break;
        case Question:
            icon = style->standardIcon(QStyle::SP_MessageBoxQuestion);
            break;
        default:
            icon = style->standardIcon(QStyle::SP_MessageBoxInformation);
            break;
    }

    return icon.pixmap(iconSize);
}

QPixmap NotificationWidget::getAppIcon() {
    // 这里可以返回您的应用图标
    QIcon icon = QIcon(":/public/image/Pelr.png");
    // 如果图标不存在，使用默认图标
    if (icon.isNull()) {
        icon = QApplication::style()->standardIcon(QStyle::SP_ComputerIcon);
    }
    return icon.pixmap(28, 28);
}

void NotificationWidget::showNotificationInternal(
    const QString &title,
    const QString &message,
    int duration,
    MessageType type,
    std::function<void()> clickCallback) {
    qDebug() << "[Notify] Thread:" << QThread::currentThread();
    qDebug() << "[Notify] UI thread:" << QApplication::instance()->thread();

    // 停止计时器
    if (m_hideTimer && m_hideTimer->isActive()) {
        m_hideTimer->stop();
    }

    // 设置内容
    m_titleLabel->setText(title);
    m_iconLabel->setPixmap(getIconForType(type));
    m_messageLabel->setText(message);

    // 设置点击回调
    m_clickCallback = clickCallback ? clickCallback : [this]() { hide(); };

    // 计算并设置最佳高度
    m_messageLabel->adjustSize();
    int messageHeight = m_messageLabel->height();
    int calculatedHeight = 16 +
                           m_titleLabel->sizeHint().height() +
                           12 +
                           qMax(64, messageHeight) +
                           16;

    setFixedHeight(qMin(calculatedHeight, 250));

    // 更新位置
    updatePosition();

    // 确保窗口完全更新
    adjustSize();
    update();

    // 显示窗口（无动画）
    std::cout << "\a"; // 提示音效
    show();
    raise();
    activateWindow();

    // 启动自动隐藏计时器
    if (duration > 0) {
        m_hideTimer->start(duration);
    }
}

void NotificationWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆角白色背景
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);

    painter.fillPath(path, QBrush(Qt::white));

    // 绘制边框
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    painter.drawPath(path);
}

void NotificationWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // 停止自动隐藏计时器
        if (m_hideTimer && m_hideTimer->isActive()) {
            m_hideTimer->stop();
        }

        // 执行点击回调
        if (m_clickCallback) {
            m_clickCallback();
        }
    }
    hide();

    QWidget::mousePressEvent(event);
}
