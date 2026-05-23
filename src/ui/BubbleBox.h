
#pragma once

#include <QLabel>
#include <QTimer>
#include <QString>
#include <QMediaPlayer>
#include <QThread>
#include "llamaclient.h"

class BubbleBox : public QLabel
{
    Q_OBJECT

private:
    // 私有构造函数
    explicit BubbleBox(QLabel *parent = nullptr);

    // 禁止拷贝和赋值
    BubbleBox(const BubbleBox &) = delete;

    BubbleBox &operator=(const BubbleBox &) = delete;

public:
    ~BubbleBox();

    // 获取单例实例的静态方法
    static BubbleBox *instance(QLabel *parent = nullptr);

    void setThinkingText();

    QString GetSystemTime();

    QTimer *fadeTimer;
    QString voice;
    QString now = "";

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // 静态单例实例指针
    static BubbleBox *m_instance;
    bool isFirst = true;
    QString m_text;

    QString getPeriodText();

public:
    static constexpr int AI_RANDOM_ID = 3;

public slots:
    void showTime();

    void resetFadeTimer();

    void textSet(const QString &text);

    void RandomSentence();

    void onRandomSentenceAI(const QString &text, int id);
    void onRandomSentenceError(const QString &error, int id);

    void updateWindowLocation(int f_x, int f_y, int f_w, int f_h);

signals:
    void generateVoiceSignal(const QString &APPID, const QString &APIKey,
                             const QString &APISecret, const QString &text);
};
