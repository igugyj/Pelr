#include "componentcard.hpp"

#include <QFont>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QVBoxLayout>

ComponentCard::ComponentCard(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("componentCard"));

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->setSpacing(6);

    auto *header = new QHBoxLayout;
    header->setSpacing(8);

    m_title = new QLabel(title, this);
    QFont tf = m_title->font();
    tf.setBold(true);
    m_title->setFont(tf);

    m_status = new QLabel(QStringLiteral("—"), this);
    m_status->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    header->addWidget(m_title, 1);
    header->addWidget(m_status, 1);
    outer->addLayout(header);

    m_detail = new QLabel(this);
    m_detail->setWordWrap(true);
    m_detail->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_detail->setStyleSheet(QStringLiteral("color: palette(mid);"));
    outer->addWidget(m_detail);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(6);

    m_extract = new QPushButton(tr("Extract now"), this);
    m_browse = new QPushButton(tr("Open folder"), this);
    m_deleteZip = new QPushButton(tr("Delete archive"), this);

    buttons->addWidget(m_extract);
    buttons->addWidget(m_browse);
    buttons->addWidget(m_deleteZip);
    buttons->addStretch(1);
    outer->addLayout(buttons);

    connect(m_extract, &QPushButton::clicked, this, &ComponentCard::extractRequested);
    connect(m_browse, &QPushButton::clicked, this, &ComponentCard::browseRequested);
    connect(m_deleteZip, &QPushButton::clicked, this, &ComponentCard::deleteZipRequested);

    applyState(ComponentManager::State::Missing);
}

void ComponentCard::applyState(ComponentManager::State state)
{
    QString text;
    QString color;

    switch (state)
    {
    case ComponentManager::State::Installed:
        text = tr("Installed");
        color = QStringLiteral("#107c10");
        break;
    case ComponentManager::State::NeedsExtract:
        text = tr("Archive ready — extract to install");
        color = QStringLiteral("#c77700");
        break;
    case ComponentManager::State::Invalid:
        text = tr("Layout not recognized");
        color = QStringLiteral("#c42b1c");
        break;
    case ComponentManager::State::Missing:
    default:
        text = tr("Not installed");
        color = QStringLiteral("#8a8a8a");
        break;
    }

    m_status->setText(text);
    m_status->setStyleSheet(QStringLiteral("color: %1; font-weight: bold;").arg(color));
}

void ComponentCard::retranslateUI(const QString &hint)
{
    m_hint = hint;

    m_extract->setText(tr("Extract now"));
    m_browse->setText(tr("Open folder"));
    m_deleteZip->setText(tr("Delete archive"));

    if (m_state == ComponentManager::State::Missing)
        m_detail->setText(m_hint);
}

void ComponentCard::updateInfo(const ComponentManager::Info &info)
{
    m_state = info.state;
    m_title->setText(info.name);
    // 未安装时给出安装指引，其余状态给出诊断细节
    m_detail->setText(info.state == ComponentManager::State::Missing && !m_hint.isEmpty()
                          ? m_hint
                          : info.detail);
    applyState(info.state);

    m_extract->setVisible(info.state == ComponentManager::State::NeedsExtract);
    m_deleteZip->setVisible(!info.zip.isEmpty());
    m_browse->setVisible(true);
}
