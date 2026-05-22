
#include "editor.h"
#include "ui_editor.h"
#include <QFileDialog>
#include <QMessageBox>
#include "NotificationWidget.h"
using MessageType = NotificationWidget::MessageType;

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent), ui(new Ui::editor)
{
    ui->setupUi(this);

    this->setWindowIcon(qApp->windowIcon());
    // QFile styleFile(":/thirdParty/QSS/Ubuntu.qss");
    // if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
    // {
    //     QString styleSheet = QLatin1String(styleFile.readAll());
    //     this->setStyleSheet(styleSheet);
    //     styleFile.close();
    // }
    connect(ui->pushButton_2, &QPushButton::clicked, [&]()
            { selectFile(ui->lineEdit_2); });
    connect(ui->pushButton_3, &QPushButton::clicked, [&]()
            { selectFile(ui->lineEdit_3); });
    connect(ui->pushButton, &QPushButton::clicked, this, &EditorWidget::accepted);
}

QPair<QList<QString>, QList<QString>> EditorWidget::getAllInfo()
{
    QString name = ui->lineEdit->text();
    QString path = ui->lineEdit_2->text();
    QString icon = ui->lineEdit_3->text();
    QString desc = ui->textEdit->toPlainText();
    QList<QString> category;
    if (name.isEmpty() || path.isEmpty())
    {
        NotificationWidget::showNotification(tr("Warning"), tr("名称和路径不能为空"), 5000, MessageType::Warning);
        return {};
    }
    if (ui->checkBox->isChecked())
    {
        category.append("Star");
    }
    if (ui->checkBox_2->isChecked())
    {
        category.append("App");
    }
    if (ui->checkBox_3->isChecked())
    {
        category.append("Link");
    }
    if (ui->checkBox_4->isChecked())
    {
        category.append("Scripts");
    }
    if (category.isEmpty())
    {
        NotificationWidget::showNotification(tr("Warning"), tr("至少选择一个分类"), 5000, MessageType::Warning);
        return {};
    }
    return {category, {name, path, icon, desc}};
}

void EditorWidget::selectFile(QLineEdit *lineEdit)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (fileName.isEmpty())
    {
        return;
    }
    lineEdit->setText(fileName);
}

void EditorWidget::setData(const QString &category, const QString &name,
                           const QString &path, const QString &icon, const QString &description)
{
    ui->lineEdit->setText(name);
    ui->lineEdit_2->setText(path);
    ui->lineEdit_3->setText(icon);
    ui->textEdit->setPlainText(description);

    // 设置分类复选框
    ui->checkBox->setChecked(category == "Star");
    ui->checkBox_2->setChecked(category == "App");
    ui->checkBox_3->setChecked(category == "Link");
    ui->checkBox_4->setChecked(category == "Scripts");
}

EditorWidget::~EditorWidget()
{
    delete ui;
}
