#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

// 可选组件（Live2D Cubism Core / VOICEVOX CORE）的探测、解包与清理。
class ComponentManager : public QObject
{
    Q_OBJECT

public:
    enum class State
    {
        Missing,      // 目标文件不存在
        NeedsExtract, // 发现 zip，等待解压
        Installed,    // 可用
        Invalid       // 目录存在但结构不符合预期
    };
    Q_ENUM(State)

    struct Info
    {
        QString id;    // "live2d" / "voicevox"
        QString name;
        State state = State::Missing;
        QString detail;
        QString dir;
        QString zip;
    };

    static ComponentManager &instance();

    static QList<Info> scanAll();
    static Info scanLive2d();
    static Info scanVoicevox();

    static bool live2dCoreAvailable();
    static bool voicevoxAvailable();
    static bool voicevoxPluginAvailable();

    static QStringList findZips(const QString &dir);

    // 解包 zip 到 destDir，剥离 zip 的单一顶层目录。
    // created 返回本次实际写入的顶层条目（相对 destDir），供清理使用。
    static bool unzip(const QString &zipPath, const QString &destDir,
                      QStringList *created, QString *err);

    // 启动时调用：自动解包 Live2D/*.zip → 提升核心文件 → 清理残留 → 发射 componentsChanged
    bool scanAllForZip();

    // 移除 dir 下所有 *.zip，然后发射 componentsChanged
    void deleteZips(const QString &dir);

signals:
    void componentsChanged();

private:
    explicit ComponentManager(QObject *parent = nullptr);

    static bool exists(const QString &path);
    static bool validateLive2dLayout(const QString &dir);
    static bool promoteLive2dArtifacts(const QString &live2dDir, QString *err);
    static void cleanupAfterExtract(const QString &live2dDir, const QStringList &created);
};
