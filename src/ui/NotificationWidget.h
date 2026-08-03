#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QMessageBox>
#include <functional>

class NotificationWidget : public QWidget {
    Q_OBJECT

public:
    enum MessageType {
        Information,
        Warning,
        Critical,
        Question
    };
    // 显示通知（线程安全，可在任何地方调用）
    static void showNotification(
        const QString &title,
        const QString &message = QString(),
        int duration = 5000,
        MessageType type = Information,
        std::function<void()> clickCallback = nullptr
    );

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

private:
    explicit NotificationWidget(QWidget *parent = nullptr);

    ~NotificationWidget();

    // 禁用拷贝构造和赋值
    NotificationWidget(const NotificationWidget &) = delete;

    NotificationWidget &operator=(const NotificationWidget &) = delete;

    void setupUI();

    void updatePosition();

    QPixmap getIconForType(MessageType type);

    QPixmap getAppIcon();

    // 内部显示方法
    void showNotificationInternal(
        const QString &title,
        const QString &message,
        int duration,
        MessageType type,
        std::function<void()> clickCallback
    );

    QLabel *m_appIconLabel; // 应用图标
    QLabel *m_titleLabel; // 标题标签
    QLabel *m_iconLabel; // 消息类型图标
    QLabel *m_messageLabel; // 消息内容
    QTimer *m_hideTimer; // 自动隐藏计时器

    std::function<void()> m_clickCallback; // 点击回调函数
};

#endif // NOTIFICATIONWIDGET_H
