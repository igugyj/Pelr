
#pragma once

#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QFile>
#include <QDataStream>
#include <QDir>
#include <QHostInfo>
#include <QDateTime>
#include <QDebug>
#include "TranslationManager.h"

class CheckApplication : public QDialog
{
    Q_OBJECT

public:
    explicit CheckApplication(QWidget *parent = nullptr);

    bool isLicenseAccepted() const { return m_licenseAccepted; }

    void retranslateUI();

    // 添加静态方法来检查许可证
    static bool hasValidLicense();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onAcceptClicked();

    void onRejectClicked();

private:
    void setupUI();

    bool validateExistingLicense();

    void saveLicenseAgreement();

    QString getComputerName();

    QString getCurrentVersion();

    QLabel *m_titleLabel;
    QTextEdit *licenseText;
    QCheckBox *agreeCheckbox;
    QPushButton *acceptButton;
    QPushButton *rejectButton;

    bool m_licenseAccepted;
    QString m_username;
    QString m_version;
    bool m_licenseValidated;
};
