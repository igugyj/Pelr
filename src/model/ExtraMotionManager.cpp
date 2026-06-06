
#include "ExtraMotionManager.h"
#include <QDebug>
#include "LAppDefine.hpp"
#include "TranslationManager.h"
#include "custommenu.h"
using namespace LAppDefine;
ExtraMotionManager *ExtraMotionManager::instance = nullptr;

ExtraMotionManager::ExtraMotionManager()
    : CustomMenu(nullptr), model(nullptr)
{
    setTitle(tr("Extra Content"));

    motionMenu = new CustomMenu(tr("Motions"), this);
    expressionMenu = new CustomMenu(tr("Expressions"), this);

    addMenu(motionMenu);
    addMenu(expressionMenu);

    connect(TranslationManager::instance(), &TranslationManager::languageChanged,
            this, [this](const QString &)
            { retranslateUI(); });
}

void ExtraMotionManager::retranslateUI()
{
    setTitle(tr("Extra Content"));
    refreshMenu();
}

ExtraMotionManager::~ExtraMotionManager()
{
    instance = nullptr;
    delete instance;
}

ExtraMotionManager *ExtraMotionManager::getInstance()
{
    if (!instance)
    {
        instance = new ExtraMotionManager();
    }
    return instance;
}

void ExtraMotionManager::destroyInstance()
{
    delete instance;
}

void ExtraMotionManager::setModel(LAppModel *newModel)
{
    model = newModel;
    refreshMenu();
}

void ExtraMotionManager::refreshMenu()
{
    // 清空现有菜单
    motionMenu->clear();
    expressionMenu->clear();

    if (!model)
    {
        QAction *noModelAction = new QAction(tr("No Model Loaded"), this);
        noModelAction->setEnabled(false);
        motionMenu->addAction(noModelAction);
        return;
    }

    // 添加动作
    int motionCount = 0;
    for (auto it = model->extraMotions.Begin(); it != model->extraMotions.End(); ++it)
    {
        if (it->Second == NULL)
            continue;

        QString motionName = QString::fromUtf8(it->First.GetRawString());
        QAction *action = new QAction(motionName, this);

        connect(action, &QAction::triggered, [this, motionName]()
                {
            if (model) {
                model->StartExtraMotion(motionName.toUtf8().constData(), PriorityNormal);
            } });

        motionMenu->addAction(action);
        motionCount++;
    }
    motionMenu->setTitle(tr("Motions (%1)").arg(motionCount));

    // 添加表情
    int expressionCount = 0;
    for (auto it = model->extraExpressions.Begin(); it != model->extraExpressions.End(); ++it)
    {
        if (it->Second == NULL)
            continue;
        QString expressionName = QString::fromUtf8(it->First.GetRawString());
        QAction *action = new QAction(expressionName, this);

        connect(action, &QAction::triggered, [this, expressionName]()
                {
            if (model) {
                model->SetExtraExpression(expressionName.toUtf8().constData());
            } });

        expressionMenu->addAction(action);
        expressionCount++;
    }
    expressionMenu->setTitle(tr("Expressions (%1)").arg(expressionCount));
}
