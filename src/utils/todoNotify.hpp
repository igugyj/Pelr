
#pragma once

#include <QDateTime>
#include "data.hpp"
#include "BubbleBox.h"
#include <QList>
#include "NotificationWidget.h"

class TodoNotify : public QObject
{
    Q_OBJECT

    TodoNotify() = default;                             // 私有构造函数
    ~TodoNotify() = default;                            // 私有析构函数
    TodoNotify(const TodoNotify &) = delete;            // 删除拷贝构造函数
    TodoNotify &operator=(const TodoNotify &) = delete; // 删除赋值运算符

public:
    QString newest_title = "";

    static TodoNotify &instance()
    {
        static TodoNotify instance;
        return instance;
    }

    // 不会修改数据，只读取
    void todoNotify()
    {
        // 加载数据
        QList<TodoData> data = DataManager::instance().todo_data;          // first
        auto todo_setting_data = DataManager::instance().getTodoSetting(); // second
        bool is_notify = todo_setting_data.is_show_todo;                   // second
        bool is_notify_by_tray = todo_setting_data.is_notify_tray;         // second
        // 遍历比较数据的时间
        const QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
        for (const TodoData &item : data)
        {
            if (now == item.deadline && is_notify && item.title != newest_title && item.isNotify)
            {
                // 发送提醒
                QString msg = tr("您的事件：「%1」即将截止，请及时完成！\n%2").arg(item.title).arg(now);
                // 如果是待办事项，则显示气泡提示
                if (item.category == 1)
                    BubbleBox::instance()->textSet(msg);
                // 如果选择了托盘提醒，则弹出提示
                if (is_notify_by_tray)
                {
                    QMetaObject::invokeMethod(qApp, [msg]()
                                              { NotificationWidget::showNotification(
                                                    tr("待办事项提醒"), msg,
                                                    10000, NotificationWidget::Information); }, Qt::QueuedConnection);
                }
                qDebug() << "[TODO] Notify:" << msg;
                // 更新最新数据
                newest_title = item.title;
            }
        }
    }

    static void askLatestNextEvent()
    {
        // 加载数据
        QList<TodoData> data = DataManager::instance().todo_data; // first
        if (data.isEmpty())
        {
            qDebug() << "[TODO] TODO data is empty";
            BubbleBox::instance()->textSet(tr("还没有待办事项哦！"));
            return;
        }
        const QDateTime now = QDateTime::currentDateTime();
        QDateTime nearestFuture;
        TodoData nearestEvent;

        const QString timeFormat = "yyyy-MM-dd HH:mm";
        for (TodoData const &a : data)
        {
            QDateTime eventTime = QDateTime::fromString(a.deadline, timeFormat);
            // 确保时间转换成功且是未来时间
            if (eventTime.isValid() && eventTime > now)
            {
                // 如果是第一个未来时间，或者比当前记录的更近
                if (!nearestFuture.isValid() || eventTime < nearestFuture)
                {
                    nearestFuture = eventTime;
                    nearestEvent = a;
                }
            }
        }
        qDebug() << "[TODO] nearestFuture:" << nearestFuture << "nearestEvent:" << nearestEvent.title;
        if (nearestEvent.title.isEmpty())
        {
            qInfo() << "[TODO] NearestEvent.title isEmpty";
            BubbleBox::instance()->textSet(tr("还没有最近的待办事项哦！"));
            return;
        }
        QString rem = tr("最近的一次待办事项是「%1」，截止时间是：%2").arg(nearestEvent.title).arg(nearestEvent.deadline);
        BubbleBox::instance()->textSet(rem);
    }
};
