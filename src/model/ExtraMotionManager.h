
#pragma once
#include <QMenu>
#include "LAppModel.hpp"
#include "custommenu.h"

class ExtraMotionManager : public CustomMenu
{
    Q_OBJECT

public:
    static ExtraMotionManager *getInstance();

    static void destroyInstance();

    void setModel(LAppModel *newModel);

    void retranslateUI();

private:
    ExtraMotionManager();

    ~ExtraMotionManager();

    void refreshMenu();

    static ExtraMotionManager *instance;
    LAppModel *model;
    QMenu *motionMenu;
    QMenu *expressionMenu;
};
