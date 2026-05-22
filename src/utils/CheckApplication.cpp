
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
    setWindowTitle(tr("MIT 许可证确认"));
    setFixedSize(800, 600);

    // 创建控件
    QLabel *titleLabel = new QLabel(tr("MIT 许可证"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    // MIT 许可证文本
    licenseText = new QTextEdit(this);
    licenseText->setReadOnly(true);

    QString LicenseText = tr(
                              "MIT 许可证\n\n"
                              "版权所有 (C) 2007 自由软件基金会 <https://fsf.org/>\n\n"
                              "特此免费授予任何获得本软件及相关文档文件（“软件”）副本的人不受限制地处理本软件的权限，\n"
                              "包括但不限于使用、复制、修改、合并、发布、分发、再许可和/或销售软件副本的权利，\n"
                              "以及允许获得软件的人这样做，但须符合以下条件：\n\n"
                              "上述版权声明和本许可声明应包含在本软件的所有副本或重要部分中。\n\n"
                              "本软件“按原样”提供，不提供任何明示或暗示的保证，包括但不限于适销性、\n"
                              "特定用途适用性和非侵权性的保证。在任何情况下，作者或版权持有人均不对任何索赔、\n"
                              "损害或其他责任负责，无论是在合同、侵权或其他方面，由软件或软件的使用或其他交易引起或与之相关。\n\n"
                              "本程序使用 MIT 许可证。\n\n"
                              "用户: %1\n"
                              "版本: %2")
                              .arg(m_username)
                              .arg(m_version);

    licenseText->setPlainText(LicenseText);

    // 同意复选框
    agreeCheckbox = new QCheckBox(tr("我理解并接受上述 MIT 许可证条款"), this);

    // 按钮
    acceptButton = new QPushButton(tr("接受"), this);
    rejectButton = new QPushButton(tr("拒绝"), this);

    acceptButton->setEnabled(false);

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
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
