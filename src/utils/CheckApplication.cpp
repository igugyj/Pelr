
#include "CheckApplication.h"
#include "data.hpp"

#define LICENSE_CHECK_FILE "user/license.dat"

CheckApplication::CheckApplication(QWidget *parent)
    : QDialog(parent),
      m_licenseAccepted(false),
      m_licenseValidated(false)
{
    // 获取系统信息
    m_username = getComputerName();
    m_version = getCurrentVersion();

    // 延迟许可证验证到 showEvent
    setupUI();
    setModal(true);

    connect(TranslationManager::instance(), &TranslationManager::languageChanged,
            this, [this](const QString &) { retranslateUI(); });
}

void CheckApplication::showEvent(QShowEvent *event)
{
    // 在窗口显示前检查许可证
    if (!m_licenseValidated)
    {
        m_licenseValidated = true;

        if (validateExistingLicense())
        {
            m_licenseAccepted = true;
            // 延迟调用 accept，确保对象完全构造
            QMetaObject::invokeMethod(this, "accept", Qt::QueuedConnection);
            return;
        }
    }

    QDialog::showEvent(event);
}

bool CheckApplication::hasValidLicense()
{
    // 静态方法，用于在创建对话框前检查许可证
    QString username = QHostInfo::localHostName();
    QString version = DataManager::instance().const_config_data.version; // 与实际版本保持一致

    QFile file(LICENSE_CHECK_FILE);
    if (!file.exists())
    {
        return false;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    QString savedUsername, savedVersion;
    QDateTime savedTimestamp;

    in >> savedUsername >> savedVersion >> savedTimestamp;
    file.close();

    // 验证用户名和版本是否匹配
    if (savedUsername == username && savedVersion == version)
    {
        // 检查许可证是否在有效期内
        if (savedTimestamp.daysTo(QDateTime::currentDateTime()) <= 365)
        {
            qDebug() << "[License] Found valid license file";
            return true;
        }
    }

    return false;
}

void CheckApplication::setupUI()
{
    setWindowTitle(tr("MIT License Agreement"));
    setFixedSize(800, 600);

    // 创建控件
    m_titleLabel = new QLabel(tr("MIT License"), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);

    // MIT 许可证文本
    licenseText = new QTextEdit(this);
    licenseText->setReadOnly(true);

    QString LicenseText = tr(
                              "MIT License\n\n"
                              "Copyright (c) 2007 Free Software Foundation <https://fsf.org/>\n\n"
                              "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n\n"
                              "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n\n"
                              "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\n"
                              "This program is licensed under the MIT License.\n\n"
                              "User: %1\n"
                              "Version: %2")
                              .arg(m_username)
                              .arg(m_version);

    licenseText->setPlainText(LicenseText);

    // 同意复选框
    agreeCheckbox = new QCheckBox(tr("I understand and accept the above MIT License terms"), this);

    // 按钮
    acceptButton = new QPushButton(tr("Accept"), this);
    rejectButton = new QPushButton(tr("Reject"), this);

    acceptButton->setEnabled(false);

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(licenseText);
    mainLayout->addWidget(agreeCheckbox);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(rejectButton);
    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(agreeCheckbox, &QCheckBox::checkStateChanged,
            [this](Qt::CheckState state)
            { acceptButton->setEnabled(state == Qt::Checked); });
    connect(acceptButton, &QPushButton::clicked, this, &CheckApplication::onAcceptClicked);
    connect(rejectButton, &QPushButton::clicked, this, &CheckApplication::onRejectClicked);
}

void CheckApplication::retranslateUI()
{
    setWindowTitle(tr("MIT License Agreement"));
    m_titleLabel->setText(tr("MIT License"));
    QString LicenseText = tr(
                              "MIT License\n\n"
                              "Copyright (c) 2007 Free Software Foundation <https://fsf.org/>\n\n"
                              "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n\n"
                              "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n\n"
                              "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n\n"
                              "This program is licensed under the MIT License.\n\n"
                              "User: %1\n"
                              "Version: %2")
                              .arg(m_username)
                              .arg(m_version);
    licenseText->setPlainText(LicenseText);
    agreeCheckbox->setText(tr("I understand and accept the above MIT License terms"));
    acceptButton->setText(tr("Accept"));
    rejectButton->setText(tr("Reject"));
}

bool CheckApplication::validateExistingLicense()
{
    QFile file(LICENSE_CHECK_FILE);
    if (!file.exists())
    {
        qDebug() << "[License] License check file not found";
        return false;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[License] License check file open failed";
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
        if (savedTimestamp.daysTo(QDateTime::currentDateTime()) <= 365)
        {
            qDebug() << "[License] Found valid license file, user:" << savedUsername << "version:" << savedVersion;
            return true;
        }
        else
        {
            qDebug() << "[License] Found expired license file";
        }
    }

    qDebug() << "[License] License check failed, expected user:" << m_username << "actual user:" << savedUsername
             << "expected version:" << m_version << "actual version:" << savedVersion;
    return false;
}

void CheckApplication::saveLicenseAgreement()
{
    QDir dir;
    if (!dir.exists("user"))
    {
        dir.mkdir("user");
    }

    QFile file(LICENSE_CHECK_FILE);
    if (file.open(QIODevice::WriteOnly))
    {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);

        QDateTime timestamp = QDateTime::currentDateTime();
        out << m_username << m_version << timestamp;
        file.close();
        qDebug() << "[License] License agreement saved, user:" << m_username << "version:" << m_version;
    }
    else
    {
        qDebug() << "[License] Can't save license agreement";
    }
}

QString CheckApplication::getComputerName()
{
    return QHostInfo::localHostName();
}

QString CheckApplication::getCurrentVersion()
{
    return DataManager::instance().const_config_data.version;
}

void CheckApplication::onAcceptClicked()
{
    saveLicenseAgreement();
    m_licenseAccepted = true;
    accept();
}

void CheckApplication::onRejectClicked()
{
    m_licenseAccepted = false;
    reject();
}
