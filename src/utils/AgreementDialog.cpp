
#include "AgreementDialog.h"
#include "data.hpp"

AgreementDialog::AgreementDialog(QWidget *parent)
    : QDialog(parent)
{
    m_username = getComputerName();
    m_version = getCurrentVersion();
    setModal(true);
}

void AgreementDialog::ensureUiInitialized()
{
    if (m_uiInitialized)
        return;
    m_uiInitialized = true;

    setupUI();

    connect(TranslationManager::instance(), &TranslationManager::languageChanged,
            this, [this](const QString &) { retranslateUI(); });
}

void AgreementDialog::showEvent(QShowEvent *event)
{
    ensureUiInitialized();

    if (!m_validated)
    {
        m_validated = true;

        if (validateExistingAgreement())
        {
            m_accepted = true;
            QMetaObject::invokeMethod(this, "accept", Qt::QueuedConnection);
            return;
        }
    }

    QDialog::showEvent(event);
}

bool AgreementDialog::hasAcceptedCheck(const QString &fileName, int maxDays)
{
    QString username = QHostInfo::localHostName();
    QString version = DataManager::instance().const_config_data.version;

    QFile file(QString("user/%1").arg(fileName));
    if (!file.exists())
        return false;

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    QString savedUsername, savedVersion;
    QDateTime savedTimestamp;

    in >> savedUsername >> savedVersion >> savedTimestamp;
    file.close();

    if (savedUsername == username && savedVersion == version)
    {
        if (maxDays < 0)
            return true;

        if (savedTimestamp.daysTo(QDateTime::currentDateTime()) <= maxDays)
            return true;
    }

    return false;
}

void AgreementDialog::setupUI()
{
    setWindowTitle(agreementTitle());
    setFixedSize(800, 600);

    m_titleLabel = new QLabel(agreementTitle(), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setPlainText(agreementText());

    m_agreeCheckbox = new QCheckBox(checkboxText(), this);

    m_acceptButton = new QPushButton(acceptButtonText(), this);
    m_rejectButton = new QPushButton(rejectButtonText(), this);
    m_acceptButton->setEnabled(false);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_textEdit);
    mainLayout->addWidget(m_agreeCheckbox);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_acceptButton);
    buttonLayout->addWidget(m_rejectButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_agreeCheckbox, &QCheckBox::checkStateChanged,
            [this](Qt::CheckState state)
            { m_acceptButton->setEnabled(state == Qt::Checked); });
    connect(m_acceptButton, &QPushButton::clicked, this, &AgreementDialog::onAcceptClicked);
    connect(m_rejectButton, &QPushButton::clicked, this, &AgreementDialog::onRejectClicked);
}

void AgreementDialog::retranslateUI()
{
    if (!m_uiInitialized)
        return;
    setWindowTitle(agreementTitle());
    m_titleLabel->setText(agreementTitle());
    m_textEdit->setPlainText(agreementText());
    m_agreeCheckbox->setText(checkboxText());
    m_acceptButton->setText(acceptButtonText());
    m_rejectButton->setText(rejectButtonText());
}

bool AgreementDialog::validateExistingAgreement()
{
    QFile file(QString("user/%1").arg(checkFileName()));
    if (!file.exists())
    {
        qDebug() << "[Agreement] Check file not found:" << checkFileName();
        return false;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[Agreement] Check file open failed:" << checkFileName();
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    QString savedUsername, savedVersion;
    QDateTime savedTimestamp;

    in >> savedUsername >> savedVersion >> savedTimestamp;
    file.close();

    if (savedUsername == m_username && savedVersion == m_version)
    {
        int maxDays = agreementMaxDays();
        if (maxDays < 0)
        {
            qDebug() << "[Agreement] Found valid agreement:" << checkFileName();
            return true;
        }

        if (savedTimestamp.daysTo(QDateTime::currentDateTime()) <= maxDays)
        {
            qDebug() << "[Agreement] Found valid agreement:" << checkFileName();
            return true;
        }
        else
        {
            qDebug() << "[Agreement] Found expired agreement:" << checkFileName();
        }
    }

    return false;
}

void AgreementDialog::saveAgreement()
{
    QDir dir;
    if (!dir.exists("user"))
    {
        dir.mkdir("user");
    }

    QFile file(QString("user/%1").arg(checkFileName()));
    if (file.open(QIODevice::WriteOnly))
    {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);

        QDateTime timestamp = QDateTime::currentDateTime();
        out << m_username << m_version << timestamp;
        file.close();
        qDebug() << "[Agreement] Agreement saved:" << checkFileName()
                 << "user:" << m_username << "version:" << m_version;
    }
    else
    {
        qDebug() << "[Agreement] Can't save agreement:" << checkFileName();
    }
}

QString AgreementDialog::getComputerName() const
{
    return QHostInfo::localHostName();
}

QString AgreementDialog::getCurrentVersion() const
{
    return DataManager::instance().const_config_data.version;
}

void AgreementDialog::onAcceptClicked()
{
    saveAgreement();
    m_accepted = true;
    accept();
}

void AgreementDialog::onRejectClicked()
{
    m_accepted = false;
    reject();
}

// ---- Default overridables ----

QString AgreementDialog::checkboxText() const
{
    return tr("I have read and agree to the above terms");
}

QString AgreementDialog::acceptButtonText() const
{
    return tr("Accept");
}

QString AgreementDialog::rejectButtonText() const
{
    return tr("Reject");
}

int AgreementDialog::agreementMaxDays() const
{
    return -1;
}
