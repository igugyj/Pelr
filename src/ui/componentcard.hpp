#pragma once

#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "componentmanager.hpp"

// Components 设置页中的一张组件卡片
class ComponentCard : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentCard(const QString &title, QWidget *parent = nullptr);

    void updateInfo(const ComponentManager::Info &info);

    // 语言热载：hint 由调用方重新 tr() 后传入（本类不持有文案来源），
    // 同时刷新按钮文本与 Missing 态下的安装指引。
    void retranslateUI(const QString &hint);

signals:
    void extractRequested();
    void browseRequested();
    void deleteZipRequested();

private:
    void applyState(ComponentManager::State state);

    QLabel *m_title;
    QLabel *m_status;
    QLabel *m_detail;
    QPushButton *m_extract;
    QPushButton *m_browse;
    QPushButton *m_deleteZip;
    QString m_hint;
    ComponentManager::State m_state = ComponentManager::State::Missing;
};
