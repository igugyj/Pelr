#pragma once

#include <QDate>
#include <QDateTime>
#include <QSet>
#include <QString>

#include "BubbleBox.h"
#include "NotificationWidget.h"
#include "data.hpp"

class TodoNotify : public QObject
{
    Q_OBJECT

    TodoNotify() = default;
    ~TodoNotify() = default;
    TodoNotify(const TodoNotify &) = delete;
    TodoNotify &operator=(const TodoNotify &) = delete;

public:
    static TodoNotify &instance()
    {
        static TodoNotify inst;
        return inst;
    }

    // 遍历当前数据，对「刚好到点」的条目发通知（同一条只发一次）
    void todoNotify()
    {
        clearIfNewDay();

        // 值拷贝：迭代期间数据可能被其它线程/定时器修改
        const QList<TodoData> data = DataManager::instance().todo_data;
        const auto setting = DataManager::instance().getTodoSetting();
        if (!setting.is_show_todo)
            return;

        const QString now =
            QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm"));

        for (const TodoData &item : data)
        {
            if (!item.isNotify)
                continue;
            if (item.deadline != now) // 只处理恰好到点的这一分钟
                continue;

            const QString key = notifyKey(item);
            if (m_notifiedKeys.contains(key))
                continue; // 这一分钟内已提醒过

            notifyItem(item, now, setting.is_notify_tray);
            m_notifiedKeys.insert(key);
        }
    }

    // 查询最近的一条未来待办
    static void askLatestNextEvent()
    {
        const QList<TodoData> data = DataManager::instance().todo_data;
        if (data.isEmpty())
        {
            qDebug() << "[TODO] TODO data is empty";
            BubbleBox::instance()->textSet(tr("No todo items yet!"));
            return;
        }

        const QDateTime now = QDateTime::currentDateTime();
        const QString timeFormat = QStringLiteral("yyyy-MM-dd HH:mm");

        QDateTime nearestFuture;
        TodoData nearestEvent;
        bool found = false;

        for (const TodoData &a : data)
        {
            const QDateTime eventTime = QDateTime::fromString(a.deadline, timeFormat);
            if (!eventTime.isValid() || eventTime <= now)
                continue;

            if (!found || eventTime < nearestFuture)
            {
                nearestFuture = eventTime;
                nearestEvent = a;
                found = true;
            }
        }

        qDebug() << "[TODO] nearestFuture:" << nearestFuture
                 << "nearestEvent:" << nearestEvent.title;

        if (!found)
        {
            qInfo() << "[TODO] no future todo";
            BubbleBox::instance()->textSet(tr("No recent todo items!"));
            return;
        }

        const QString rem =
            tr("The nearest todo item is \"%1\", deadline: %2")
                .arg(nearestEvent.title, nearestEvent.deadline);
        BubbleBox::instance()->textSet(rem);
    }

private:
    // 业务键：deadline + title，用不可见字符做分隔，避免拼接歧义
    static QString notifyKey(const TodoData &item)
    {
        return item.deadline + QLatin1Char('\x1f') + item.title;
    }

    // 跨天清理已通知记录
    void clearIfNewDay()
    {
        const QDate today = QDate::currentDate();
        if (today != m_lastDate)
        {
            m_notifiedKeys.clear();
            m_lastDate = today;
        }
    }

    // 真正的「弹通知」逻辑，集中在一处
    void notifyItem(const TodoData &item, const QString &now, bool viaTray)
    {
        const QString msg =
            tr("Your event \"%1\" is due soon! Please complete it.\n%2")
                .arg(item.title, now);

        // 待办事项 → 气泡提示
        if (item.category == TodoCategory::todo)
            BubbleBox::instance()->textSet(msg);

        // 托盘提醒
        if (viaTray)
        {
            const QString title = tr("Todo Reminder"); // 提前翻译，lambda 里不再调 tr
            QMetaObject::invokeMethod(
                qApp,
                [title, msg]()
                {
                    NotificationWidget::showNotification(
                        title, msg, 10000, NotificationWidget::Information);
                },
                Qt::QueuedConnection);
        }

        qDebug() << "[TODO] Notify:" << msg;
    }

private:
    QSet<QString> m_notifiedKeys;            // 已通知过的业务键
    QDate m_lastDate = QDate::currentDate(); // 用于跨天清理
};