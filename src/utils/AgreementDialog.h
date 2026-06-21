
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

class AgreementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AgreementDialog(QWidget *parent = nullptr);
    ~AgreementDialog() override = default;

    bool isAccepted() const { return m_accepted; }

    void retranslateUI();

    static bool hasAcceptedCheck(const QString &fileName, int maxDays);

protected:
    void showEvent(QShowEvent *event) override;

    virtual QString agreementTitle() const = 0;
    virtual QString agreementText() const = 0;
    virtual QString checkFileName() const = 0;

    virtual QString checkboxText() const;
    virtual QString acceptButtonText() const;
    virtual QString rejectButtonText() const;
    virtual int agreementMaxDays() const;

protected:
    QString getComputerName() const;
    QString getCurrentVersion() const;

protected slots:
    void onAcceptClicked();
    void onRejectClicked();

private:
    void setupUI();
    void ensureUiInitialized();
    bool validateExistingAgreement();
    void saveAgreement();

    bool m_uiInitialized = false;

    QLabel *m_titleLabel;
    QTextEdit *m_textEdit;
    QCheckBox *m_agreeCheckbox;
    QPushButton *m_acceptButton;
    QPushButton *m_rejectButton;

    bool m_accepted = false;
    QString m_username;
    QString m_version;
    bool m_validated = false;
};
