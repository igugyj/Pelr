
#include "todoWidget.h"
#include "ui_todoWidget.h"
#include <QStandardItem>
#include <QList>
#include <QDebug>
#include <QMessageBox>
#include <QDateTimeEdit>
#include "datetimepickerdialog.h"
#include "data.hpp"
#include "TranslationManager.h"

todoWidget::todoWidget(QWidget *parent) : QWidget(parent), ui(new Ui::todoWidget)
{
    ui->setupUi(this);
    initWidget();
    connect(TranslationManager::instance(), &TranslationManager::languageChanged,
            this, [this](const QString &) { retranslateUI(); });
}

void todoWidget::retranslateUI()
{
    ui->retranslateUi(this);
}

void todoWidget::onTableViewDoubleClicked(const QModelIndex &index)
{
    // deadline列是第3列（索引为2）
    if (index.column() == 2)
    {
        // 获取当前单元格的值
        QString currentText = index.data(Qt::DisplayRole).toString();
        QDateTime currentDateTime = QDateTime::currentDateTime();

        // 尝试解析当前值
        if (!currentText.isEmpty())
        {
            QDateTime parsed = QDateTime::fromString(currentText, "yyyy-MM-dd HH:mm");
            if (parsed.isValid())
            {
                currentDateTime = parsed;
            }
            else
            {
                parsed = QDateTime::fromString(currentText, "yyyy-MM-dd");
                if (parsed.isValid())
                {
                    currentDateTime = parsed;
                }
            }
        }

        // 显示日期时间选择对话框
        bool ok;
        QDateTime selectedDateTime = DateTimePickerDialog::getDateTime(
            this, currentDateTime, &ok);

        if (ok)
        {
            // 更新模型数据
            QAbstractItemModel *model = ui->tableView->model();
            model->setData(index, selectedDateTime.toString("yyyy-MM-dd HH:mm"), Qt::EditRole);
        }
    }
}

void todoWidget::initWidget()
{
    model_todo = new QStandardItemModel(ui->tableView);
    model_done = new QStandardItemModel(ui->tableView_2);

    // 设置表头
    QStringList header;
    header << tr("Title") << tr("Content") << tr("Deadline") << tr("Notes") << tr("Reminder");
    model_todo->setHorizontalHeaderLabels(header);
    model_done->setHorizontalHeaderLabels(header);
    // 设置模型
    ui->tableView->setModel(model_todo);
    ui->tableView_2->setModel(model_done);
    // 设置表格属性
    for (QTableView *tv : {ui->tableView, ui->tableView_2})
    {
        tv->setSelectionBehavior(QAbstractItemView::SelectRows);
        tv->setAlternatingRowColors(true);
        tv->setEditTriggers(QAbstractItemView::DoubleClicked);
        tv->horizontalHeader()->setStretchLastSection(true);
        tv->setSortingEnabled(true);

        // 设置列宽
        tv->setColumnWidth(0, 150); // 标题
        tv->setColumnWidth(1, 200); // 内容
        tv->setColumnWidth(2, 240); // 截止时间
        tv->setColumnWidth(3, 200); // 备注
        tv->setColumnWidth(4, 16);  // 提醒
        // 设置所有行的统一高度
        tv->verticalHeader()->setDefaultSectionSize(50);
        // tv->resizeRowsToContents();
        // tv->resizeColumnsToContents();
    }
    // 加载数据
    loadAllData();

    // connect信号槽函数
    // +
    connect(ui->pushButton, &QPushButton::clicked, [&]()
            { addTodoItem(model_todo); });
    connect(ui->pushButton_5, &QPushButton::clicked, [&]()
            { addTodoItem(model_done); });

    // move
    connect(ui->pushButton_2, &QPushButton::clicked, [&]()
            { moveItem(ui->tableView); });
    connect(ui->pushButton_6, &QPushButton::clicked, [&]()
            { moveItem(ui->tableView_2); });
    //-
    connect(ui->pushButton_3, &QPushButton::clicked, [&]()
            { deleteSelectedItem(ui->tableView); });
    connect(ui->pushButton_7, &QPushButton::clicked, [&]()
            { deleteSelectedItem(ui->tableView_2); });
    // 清除数据
    connect(ui->pushButton_4, &QPushButton::clicked, [&]()
            { clearModelData(model_todo); });
    connect(ui->pushButton_8, &QPushButton::clicked, [&]()
            { clearModelData(model_done); });
    // 连接表格的双击事件
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &todoWidget::onTableViewDoubleClicked);
    // 保存数据 这个经验它是无价的
    connect(model_todo, &QStandardItemModel::dataChanged, this, &todoWidget::saveAllData);
    connect(model_done, &QStandardItemModel::dataChanged, this, &todoWidget::saveAllData);
    connect(ui->checkBox, &QCheckBox::clicked, this, &todoWidget::onCheckBoxClicked);
    connect(ui->checkBox_2, &QCheckBox::clicked, this, &todoWidget::onCheckBoxClicked);
}

void todoWidget::onCheckBoxClicked()
{
    ToDoSettingData setting;
    setting.is_show_todo = ui->checkBox->isChecked();
    setting.is_notify_tray = ui->checkBox_2->isChecked();
    DataManager::instance().writeData(setting);
    qDebug() << "[TODO] TodoNotify setting changed and saved.";
}

void todoWidget::moveItem(QTableView *view)
{
    // 获取选择模型
    QItemSelectionModel *selectionModel = view->selectionModel();

    // 获取所有选中的行（按行号去重）
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    if (selectedIndexes.isEmpty())
    {
        qWarning() << "[TODO] Please select an item first";
        return;
    }

    // 提取不重复的行号
    QSet<int> selectedRows;
    for (const QModelIndex &index : selectedIndexes)
    {
        selectedRows.insert(index.row());
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  tr("Are you sure you want to move the selected %1 item(s)?")
                                      .arg(selectedRows.size()),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // 将行号转换为列表并排序（从大到小，这样删除时不会影响前面的行号）
    QList<int> rows = selectedRows.values();
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    // 获取源模型和目标模型
    QStandardItemModel *from_model = view == ui->tableView ? model_todo : model_done;
    QStandardItemModel *to_model = from_model == model_todo ? model_done : model_todo;

    // 移动所有选中的行
    for (int row : rows)
    {
        // 获取整行数据
        QList<QStandardItem *> rowItems = from_model->takeRow(row);
        // 添加到目标模型
        to_model->appendRow(rowItems);
    }
    qDebug() << "[TODO] Count of moved items:" << selectedRows.size();

    // 清除原视图的选择
    selectionModel->clearSelection();
    saveAllData();
}

void todoWidget::clearModelData(QStandardItemModel *model, bool cfm)
{
    // 确认对话框
    if (!cfm)
    {
        model->removeRows(0, model->rowCount());
        return;
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation", "确定要清除当前列表吗？",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        model->removeRows(0, model->rowCount());
    }
    saveAllData();
}

void todoWidget::deleteSelectedItem(QTableView *view)
{
    // 获取选择模型
    QItemSelectionModel *selectionModel = view->selectionModel();

    // 获取所有选中的行（按行号去重）
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    if (selectedIndexes.isEmpty())
    {
        qWarning() << "[TODO] Please select an item first";
        return;
    }

    // 提取不重复的行号
    QSet<int> selectedRows;
    for (const QModelIndex &index : selectedIndexes)
    {
        selectedRows.insert(index.row());
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  tr("Are you sure you want to delete the selected %1 item(s)?")
                                      .arg(selectedRows.size()),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // 将行号转换为列表并排序（从大到小，这样删除时不会影响前面的行号）
    QList<int> rows = selectedRows.values();
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    // 获取源模型和目标模型
    QStandardItemModel *model = view == ui->tableView ? model_todo : model_done;

    // 删除所有选中的行
    for (int row : rows)
    {
        // 获取整行数据
        QList<QStandardItem *> rowItems = model->takeRow(row);
        model->removeRow(row);
    }
    qDebug() << "[TODO] Count of deleted items:" << selectedRows.size();

    // 清除原视图的选择
    selectionModel->clearSelection();
    saveAllData();
}

void todoWidget::loadAllData()
{
    clearModelData(model_todo, false);
    clearModelData(model_done, false);
    // 读取数据
    QList<TodoData> todo_data = DataManager::instance().getTodoData();
    ui->checkBox->setChecked(DataManager::instance().getTodoSetting().is_show_todo);
    ui->checkBox_2->setChecked(DataManager::instance().getTodoSetting().is_notify_tray);
    // 加载数据到模型
    for (QTableView *tv : {ui->tableView, ui->tableView_2})
    {
        QStandardItemModel *model = tv == ui->tableView ? model_todo : model_done;
        for (const TodoData &data : todo_data)
        {
            if (data.category == (tv == ui->tableView ? 1 : 0))
            {
                // 1为待办，0为已完成
                QStandardItem *title = new QStandardItem(data.title);
                QStandardItem *content = new QStandardItem(data.content);
                QStandardItem *deadline = new QStandardItem(data.deadline);
                QStandardItem *remark = new QStandardItem(data.remarks);
                QStandardItem *remind = new QStandardItem();
                remind->setCheckable(true);
                remind->setEditable(false);
                remind->setTextAlignment(Qt::AlignCenter);
                if (data.isNotify)
                {
                    remind->setCheckState(Qt::Checked);
                }
                model->appendRow({title, content, deadline, remark, remind});
            }
        }
    }
}

void todoWidget::saveAllData()
{
    QList<TodoData> todo_data;
    for (QTableView *tv : {ui->tableView, ui->tableView_2})
    {
        QStandardItemModel *model = tv == ui->tableView ? model_todo : model_done;
        for (int i = 0; i < model->rowCount(); i++)
        {
            TodoData data;
            data.category = model == model_todo ? 1 : 0; // 1为待办，0为已完成
            data.title = model->item(i, 0)->text();
            data.content = model->item(i, 1)->text(); // 可选
            data.deadline = model->item(i, 2)->text();
            data.remarks = model->item(i, 3)->text(); // 可选
            data.isNotify = model->item(i, 4)->checkState() == Qt::Checked;
            if (data.title.isEmpty() && data.deadline.isEmpty())
            {
                qWarning() << "[TODO] Item" << i << "is empty, skip it.";
                continue;
            } // 如果标题、截止日期为空，则不保存
            todo_data.append(data);
        }
    }
    qDebug() << "[TODO] Count of saved items:" << todo_data.size();
    // 保存数据到文件
    DataManager::instance().writeData<QList<TodoData>>(todo_data);
    qDebug() << "[TODO] Data saved successfully.";
}

void todoWidget::addTodoItem(QStandardItemModel *model)
{
    QList<QStandardItem *> items;
    QStandardItem *title = new QStandardItem();
    QStandardItem *content = new QStandardItem();
    QStandardItem *deadline = new QStandardItem();
    QStandardItem *remark = new QStandardItem();
    QStandardItem *remind = new QStandardItem();
    title->setEditable(true);
    content->setEditable(true);
    deadline->setTextAlignment(Qt::AlignCenter);
    remark->setEditable(true);
    remind->setCheckable(true);
    remind->setEditable(false);
    remind->setTextAlignment(Qt::AlignCenter);
    items << title << content << deadline << remark << remind;
    model->appendRow(items);
}

todoWidget::~todoWidget()
{
    delete ui;
}
