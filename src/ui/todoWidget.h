
#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QDateEdit>
#include <QModelIndex>
#include <QObject>
#include <QTableView>

namespace Ui
{
    class todoWidget;
}

class todoWidget : public QWidget
{
    Q_OBJECT

public:
    QStandardItemModel *model_todo;
    QStandardItemModel *model_done;

    explicit todoWidget(QWidget *parent = nullptr);

    ~todoWidget();

    void initWidget();

    void loadAllData();

    void retranslateUI();

private slots:

    void addTodoItem(QStandardItemModel *model);

    void deleteSelectedItem(QTableView *view);

    void onTableViewDoubleClicked(const QModelIndex &index);

    void clearModelData(QStandardItemModel *model, bool cfm = true);

    void moveItem(QTableView *view);

    void saveAllData();

    void onCheckBoxClicked();

private:
    Ui::todoWidget *ui;
};
