
#include "manageStart.h"
#include "ui_manageStart.h"
#include <QDebug>
#include <QMessageBox>
#include <algorithm>
#include <QFileInfo>
#include <launcherMenu.hpp>
#include "NotificationWidget.h"
using MessageType = NotificationWidget::MessageType;
ManageStartWidget::ManageStartWidget(QWidget *parent) : QWidget(parent),
                                                        ui(new Ui::manageStart),
                                                        editorWidget(nullptr), isSaved(true)
{
    ui->setupUi(this);
    initWidget();
}

void ManageStartWidget::initWidget()
{

    connect(ui->pushButton, &QPushButton::clicked, this, &ManageStartWidget::saveMenuData);         // 保存
    connect(ui->pushButton_2, &QPushButton::clicked, this, &ManageStartWidget::showEditor);         // 添加
    connect(ui->pushButton_3, &QPushButton::clicked, this, &ManageStartWidget::editObj);            // 编辑
    connect(ui->pushButton_4, &QPushButton::clicked, this, &ManageStartWidget::deleteSelectedItem); // 删除
    // treeView初始化
    dataModel = new QStandardItemModel(ui->treeView);
    QStringList header;
    header << "Category" << "Name" << "Path" << "Icon" << "Description";
    dataModel->setHorizontalHeaderLabels(header);
    QList<QStandardItem *> parentStar, parentApp, parentLink, parentScripts;
    // 向模型中添加一些数据行（注意：QListView通常只显示第一列的数据）
    parentStar << new QStandardItem("Star");
    parentApp << new QStandardItem("App");
    parentLink << new QStandardItem("Link");
    parentScripts << new QStandardItem("Scripts");
    // 记录父节点指针
    QStandardItem *rootItem = dataModel->invisibleRootItem(); //: cite[1]:cite[4]
    p1_star = parentStar.first();
    rootItem->appendRow(p1_star);
    p2_app = parentApp.first();
    rootItem->appendRow(p2_app);
    p3_link = parentLink.first();
    rootItem->appendRow(p3_link);
    p4_scripts = parentScripts.first();
    rootItem->appendRow(p4_scripts);

    ui->treeView->setModel(dataModel);
    ui->treeView->header()->setStretchLastSection(true);                // 设置列宽调整策略（最后一列填充剩余空间）
    ui->treeView->setSortingEnabled(true);                              // 设置排序功能
    ui->treeView->sortByColumn(0, Qt::AscendingOrder);                  // 设置默认排序列
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 设置不可编辑
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection); // 设置单选模式
    ui->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);  // 设置选择行为为选择整行
    ui->treeView->setAlternatingRowColors(true);                        // 设置交替行背景色
    ui->treeView->setColumnWidth(4, 300);

    for (int i = 0; i < 5; ++i)
    {
        ui->treeView->header()->setSectionResizeMode(i, QHeaderView::Interactive); // 可交互
    }
    loadMenuData();

    QFont font = qApp->font();
    font.setWeight(QFont::Medium);
    ui->treeView->setFont(font);
    qDebug() << "treeView font:" << QFontInfo(ui->treeView->font()).family();
    qDebug() << "manageStartWidget init";
}

void ManageStartWidget::deleteSelectedItem()
{
    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (!currentIndex.isValid())
    {
        NotificationWidget::showNotification(tr("Warning"), tr("请先选择一个项目!"), 5000, MessageType::Warning);
        qWarning() << "index is invalid";
        return;
    }

    // 获取选中项
    QStandardItem *selectedItem = dataModel->itemFromIndex(currentIndex);
    if (!selectedItem)
    {
        return;
    }

    // 检查是否是顶级节点（分类节点）
    QModelIndex parentIndex = currentIndex.parent();
    if (!parentIndex.isValid())
    {
        qWarning() << "can't delete root item";
        return;
    }

    // 确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Confirmation"), tr("确定要删除选中的项目吗?"),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        // 获取父项
        QStandardItem *parentItem = dataModel->itemFromIndex(parentIndex);
        if (parentItem)
        {
            // 删除选中的行
            parentItem->removeRow(currentIndex.row());
            qDebug() << "Delete data:" << selectedItem->text();
        }
    }
    isSaved = false;
}

void ManageStartWidget::loadMenuData()
{
    QList<MenuData> data = DataManager::instance().getMenuData();
    if (data.isEmpty())
    {
        qDebug() << "No data to load";
        return;
    }; // 如果数据为空，则不加载
    for (MenuData item : data)
    {
        QList<QStandardItem *> items;
        QStandardItem *iconItem = new QStandardItem(item.icon);
        if (!item.icon.isEmpty())
            iconItem->setIcon(QIcon(item.icon));
        items << new QStandardItem("")
              << new QStandardItem(item.name)
              << new QStandardItem(item.path)
              << iconItem
              << new QStandardItem(item.description);
        if (item.category == "Star")
            p1_star->appendRow(items);
        if (item.category == "App")
            p2_app->appendRow(items);
        if (item.category == "Link")
            p3_link->appendRow(items);
        if (item.category == "Scripts")
            p4_scripts->appendRow(items);
    }
}

void ManageStartWidget::saveMenuData()
{
    QList<MenuData> data;
    getAllItems(data);
    for (int i = 0; i < data.size(); ++i)
    {
        QFileInfo fileInfo(data[i].path);
        if (!fileInfo.exists() && data[i].category != "Link")
            NotificationWidget::showNotification(
                tr("Warning"), tr("项目 %1 \n的路径不存在：%2").arg(data[i].name).arg(data[i].path), 5000, MessageType::Warning);
        qDebug() << "Save data:" << data[i].category << " " << data[i].name << " " << data[i].path << " "
                 << data[i].icon << " " << data[i].description;
    }
    DataManager::instance().writeData<QList<MenuData>>(data);
    isSaved = true;
    launcherMenu::instance()->refreshMenu();
    NotificationWidget::showNotification(tr("Information"), tr("保存成功！"));
}

void ManageStartWidget::getAllItems(QList<MenuData> &data)
{
    data.clear(); // 清空传入的数据列表
    // 遍历每个父节点
    for (QStandardItem *parentItem : {p1_star, p2_app, p3_link, p4_scripts})
    {
        // 获取父节点的所有子节点
        int rowCount = parentItem->rowCount();

        for (int row = 0; row < rowCount; ++row)
        {
            MenuData menuData;

            // 设置分类（父节点的文本）
            menuData.category = parentItem->text();
            // 获取子节点的所有列数据
            for (int col = 0; col < parentItem->columnCount(); ++col)
            {
                QStandardItem *childItem = parentItem->child(row, col);
                if (childItem)
                {
                    QString itemText = childItem->text();

                    // 根据列索引设置不同的属性
                    switch (col)
                    {
                    case 0: // Category (通常为空，因为父节点已经表示分类)
                        // 这里可以跳过，因为我们已经从父节点设置了分类
                        break;
                    case 1: // Name
                        menuData.name = itemText;
                        break;
                    case 2: // Path
                        menuData.path = itemText;
                        break;
                    case 3: // Icon
                        menuData.icon = itemText;
                        break;
                    case 4: // Description
                        menuData.description = itemText;
                        break;
                    }
                }
            }

            // 添加到结果列表
            data.append(menuData);
        }
    }
}

void ManageStartWidget::showEditor()
{
    if (editorWidget)
    {
        delete editorWidget;
        editorWidget = nullptr;
    }
    editorWidget = new EditorWidget(nullptr); // 不能是子部件 除非是模态窗口
    connect(editorWidget, &EditorWidget::accepted, this, &ManageStartWidget::onEditorAccepted);
    editorWidget->show();
}

void ManageStartWidget::onEditorAccepted()
{
    //{category, {name, path, icon, desc}}
    QPair<QList<QString>, QList<QString>> info = editorWidget->getAllInfo();
    if (info.first.isEmpty() || info.second.isEmpty())
    {
        return; // 如果信息不完整，则不添加
    }
    qDebug() << info.first;

    if (info.first.contains("Star"))
    {
        addItemToList(p1_star, info.second);
    }
    if (info.first.contains("App"))
    {
        addItemToList(p2_app, info.second);
    }
    if (info.first.contains("Link"))
    {
        addItemToList(p3_link, info.second);
    }
    if (info.first.contains("Scripts"))
    {
        addItemToList(p4_scripts, info.second);
    }
    isSaved = false;
    editorWidget->close();
    editorWidget->deleteLater();
    editorWidget = nullptr;
}

void ManageStartWidget::addItemToList(QStandardItem *parent, QList<QString> &item)
{
    //{category, {name, path, icon, desc}}
    QList<QStandardItem *> obj;
    obj << new QStandardItem("");
    for (int i = 0; i < item.size(); ++i)
    {
        QStandardItem *x = new QStandardItem(item[i]);
        if (i == 2 && !item[i].isEmpty())
            x->setIcon(QIcon(item[i]));
        obj << x;
    }
    parent->appendRow(obj);
}

void ManageStartWidget::editObj()
{
    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (!currentIndex.isValid())
    {
        qWarning() << "Invalid index";
        return;
    }

    // 检查是否是顶级节点（分类节点）
    QModelIndex parentIndex = currentIndex.parent();
    if (!parentIndex.isValid())
    {
        qWarning() << "Invalid parent";
        return;
    }
    // 获取父项（分类节点）
    QStandardItem *parentItem = dataModel->itemFromIndex(parentIndex);
    if (!parentItem)
    {
        qWarning() << "Invalid parent";
        return;
    }

    // 获取当前行号
    currentRow = currentIndex.row();
    // 获取分类
    currentCategory = parentItem->text();
    // 获取各项数据
    QStandardItem *nameItem = parentItem->child(currentRow, 1);
    QStandardItem *pathItem = parentItem->child(currentRow, 2);
    QStandardItem *iconItem = parentItem->child(currentRow, 3);
    QStandardItem *descItem = parentItem->child(currentRow, 4);
    if (!nameItem || !pathItem)
    {
        qWarning() << "Error: name or path is null";
        return;
    }
    currentName = nameItem->text();
    QString currentPath = pathItem->text();
    QString currentIcon = iconItem ? iconItem->text() : "";
    QString currentDesc = descItem ? descItem->text() : "";
    // 创建并显示编辑器，填充当前数据
    if (editorWidget)
    {
        delete editorWidget;
        editorWidget = nullptr;
    }
    editorWidget = new EditorWidget();
    editorWidget->setData(currentCategory, currentName, currentPath, currentIcon, currentDesc);
    connect(editorWidget, &EditorWidget::accepted, this, &ManageStartWidget::onEditorAcceptedForEdit);
    editorWidget->show();
}

void ManageStartWidget::onEditorAcceptedForEdit()
{
    QPair<QList<QString>, QList<QString>> info = editorWidget->getAllInfo();
    if (info.first.isEmpty() || info.second.isEmpty())
    {
        return;
    }
    qDebug() << info.first;
    // 获取父项
    QStandardItem *parentItem = nullptr;
    if (currentCategory == "Star")
        parentItem = p1_star;
    else if (currentCategory == "App")
        parentItem = p2_app;
    else if (currentCategory == "Link")
        parentItem = p3_link;
    else if (currentCategory == "Scripts")
        parentItem = p4_scripts;

    if (!parentItem)
    {
        return;
    }
    // 从原分类删除
    parentItem->removeRow(currentRow);
    qDebug() << "Remove row:" << currentRow;

    if (info.first.contains("Star"))
    {
        addItemToList(p1_star, info.second);
    }
    if (info.first.contains("App"))
    {
        addItemToList(p2_app, info.second);
    }
    if (info.first.contains("Link"))
    {
        addItemToList(p3_link, info.second);
    }
    if (info.first.contains("Scripts"))
    {
        addItemToList(p4_scripts, info.second);
    }
    isSaved = false;
    editorWidget->close();
    editorWidget->deleteLater();
    editorWidget = nullptr;
}

ManageStartWidget::~ManageStartWidget()
{
    if (editorWidget)
    {
        editorWidget->deleteLater();
    }
    delete ui;
}
