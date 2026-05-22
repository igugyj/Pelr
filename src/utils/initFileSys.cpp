#include "initFileSys.h"
#include <QList>
#include <QDebug>
void initFileSys()
{
    QList<QString> paths = {
        "user",
        "log",
        "voice_files",
        "Resources",
        "voicevox_core",
    };
    for (auto &path : paths)
    {
        QDir().mkpath(path);
    }
    qDebug() << "[APP] Necessary paths have been ensured";
}