
#pragma once

#include <QMenu>
#include "launcher.hpp"
#include "data.hpp"
#include <QAction>
#include <QIcon>
#include <QDebug>
#include <QPushButton>
#include "custommenu.h"

class launcherMenu : public CustomMenu
{
    Q_OBJECT

public:
    bool hasContent;

    // 删除拷贝构造函数和赋值运算符
    launcherMenu(const launcherMenu &) = delete;

    launcherMenu &operator=(const launcherMenu &) = delete;

    // 获取单例实例的静态方法
    static launcherMenu *instance(QWidget *parent = nullptr)
    {
        static launcherMenu instance(parent);
        return &instance;
    }

    // 初始化菜单
    void initMenu()
    {
        clear();
        QueueTimer = new QTimer(this);
        QueueTimer->setInterval(3000);

        int star_count = 0, app_count = 0, link_count = 0, scripts_count = 0;
        menu_data = DataManager::instance().getMenuData();
        QString p1, p2, p3, p4, p5;
        p1 = ":/public/IconaMoon/General/Star - 24x24.png";
        p2 = ":/public/IconaMoon/General/Apps - 24x24.png";
        p3 = ":/public/IconaMoon/Interface/Link External - 24x24.png";
        p4 = ":/public/IconaMoon/Files/File - 24x24.png";
        p5 = ":/public/IconaMoon/Media/Player -  Play - 24x24.png";
        menu_Star = new CustomMenu("Star", this);
        menu_App = new CustomMenu("App", this);
        menu_Link = new CustomMenu("Link", this);
        menu_Scripts = new CustomMenu("Scripts", this);
        CustomMenu *menu_LaunchAll = new CustomMenu("Launch All", this);
        menu_LaunchAll->setIcon(QIcon(p5));
        for (MenuData &item : menu_data)
        {
            if (item.name.isEmpty() || item.category.isEmpty() || item.path.isEmpty())
                continue;
            QAction *action = new QAction(QString(item.name), this);

            if (!item.description.isEmpty())
            {
                action->setToolTip(item.description);
            }
            // 连接信号槽
            connect(action, &QAction::triggered, [&]()
                    { launchByPath(item.path); });
            if (item.category == menu_Star->title())
            {
                menu_Star->addAction(action);
                action->setIcon(QIcon(p1));
                star_count++;
                queue_Star.enqueue(item.path);
            }
            else if (item.category == menu_App->title())
            {
                menu_App->addAction(action);
                action->setIcon(QIcon(p2));
                app_count++;
                queue_App.enqueue(item.path);
            }
            else if (item.category == menu_Link->title())
            {
                menu_Link->addAction(action);
                action->setIcon(QIcon(p3));
                link_count++;
                queue_Link.enqueue(item.path);
            }
            else if (item.category == menu_Scripts->title())
            {
                menu_Scripts->addAction(action);
                action->setIcon(QIcon(p4));
                scripts_count++;
                queue_Scripts.enqueue(item.path);
            }
            if (!item.icon.isEmpty())
            {
                action->setIcon(QIcon(item.icon));
            }
        }

        menu_Star->setTitle("Star (" + QString::number(star_count) + ")");
        menu_Star->setIcon(QIcon(p1));
        menu_App->setTitle("App (" + QString::number(app_count) + ")");
        menu_App->setIcon(QIcon(p2));
        menu_Link->setTitle("Link (" + QString::number(link_count) + ")");
        menu_Link->setIcon(QIcon(p3));
        menu_Scripts->setTitle("Scripts (" + QString::number(scripts_count) + ")");
        menu_Scripts->setIcon(QIcon(p4));
        // ------
        QAction *action_LaunchAll_Star = new QAction(menu_Star->title(), menu_LaunchAll);
        QAction *action_LaunchAll_App = new QAction(menu_App->title(), menu_LaunchAll);
        QAction *action_LaunchAll_Link = new QAction(menu_Link->title(), menu_LaunchAll);
        QAction *action_LaunchAll_Scripts = new QAction(menu_Scripts->title(), menu_LaunchAll);
        action_LaunchAll_Star->setIcon(QIcon(p1));
        action_LaunchAll_App->setIcon(QIcon(p2));
        action_LaunchAll_Link->setIcon(QIcon(p3));
        action_LaunchAll_Scripts->setIcon(QIcon(p4));
        connect(action_LaunchAll_Star, &QAction::triggered, [this]()
                { launchQueue(queue_Star); });
        connect(action_LaunchAll_App, &QAction::triggered, [this]()
                { launchQueue(queue_App); });
        connect(action_LaunchAll_Link, &QAction::triggered, [this]()
                { launchQueue(queue_Link); });
        connect(action_LaunchAll_Scripts, &QAction::triggered, [this]()
                { launchQueue(queue_Scripts); });
        menu_LaunchAll->addActions({action_LaunchAll_Star, action_LaunchAll_App, action_LaunchAll_Link, action_LaunchAll_Scripts});
        //------

        addMenu(menu_Star);
        addMenu(menu_App);
        addMenu(menu_Link);
        addMenu(menu_Scripts);
        addSeparator();
        addMenu(menu_LaunchAll);
        qDebug() << "[LauncherMenu] Menu loaded";
        hasContent = (star_count + app_count + link_count + scripts_count != 0);
    }

    // 刷新菜单数据
    void refreshMenu()
    {
        qDebug() << "[LauncherMenu] Refresh menu";
        initMenu();
    }

protected:
    // 将构造函数设为protected，确保只能通过instance方法创建
    explicit launcherMenu(QWidget *parent = nullptr) : CustomMenu(parent)
    {
        initMenu();
        applyStyle();
    }

private slots:
    void launchQueue(QQueue<QString> &queue)
    {
        if (QueueTimer && QueueTimer->isActive())
        {
            QueueTimer->stop();
            delete QueueTimer;
            QueueTimer = nullptr;
        }
        queue_Temp.clear();
        queue_Temp = queue;
        if (queue_Temp.isEmpty())
            return;
        QueueTimer = new QTimer(this);
        QueueTimer->setInterval(3000);

        // 连接定时器信号
        connect(QueueTimer, &QTimer::timeout, this, [this]()
                { processNextItem(); });
        QueueTimer->start();
    }

    void processNextItem()
    {
        if (!queue_Temp.isEmpty())
        {
            // 获取队首元素（但不移除）
            QString path = queue_Temp.head();
            launchByPath(path);
            // 移除已处理的元素
            queue_Temp.dequeue();

            // 如果队列为空，停止定时器
            if (queue_Temp.isEmpty())
            {
                if (QueueTimer && QueueTimer->isActive())
                {
                    QueueTimer->stop();
                }
            }
        }
    }

private:
    QMenu *menu_Star, *menu_App, *menu_Link, *menu_Scripts;
    QList<MenuData> menu_data;
    QQueue<QString> queue_Star, queue_App, queue_Link, queue_Scripts, queue_Temp;
    QTimer *QueueTimer = nullptr;
};
