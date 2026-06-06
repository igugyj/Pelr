
#pragma once

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QDebug>

class ScrollLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int scrollOffset READ scrollOffset WRITE setScrollOffset)

public:
    explicit ScrollLabel(QWidget *parent = nullptr);

    void setText(const QString &text);

    void startScrolling();

    void stopScrolling();

protected:
    void resizeEvent(QResizeEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

private:
    int scrollOffset() const;

    void setScrollOffset(int offset);

    QString m_fullText;
    QPropertyAnimation *m_scrollAnimation;
    int m_scrollOffset;
    bool m_needsScrolling;
};

class MediaPlayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MediaPlayerWidget(QWidget *parent = nullptr);

    ~MediaPlayerWidget();

    static MediaPlayerWidget &instance();

    void playMedia(const QString &filePath);

    void setVolume(int volume);

private slots:
    void closeEvent(QCloseEvent *event) override;

    void onOpenFile();

    void onPlayPause();

    void onStop();

    void onPositionChanged(qint64 position);

    void onDurationChanged(qint64 duration);

    void onStateChanged(QMediaPlayer::PlaybackState state);

    void onVolumeChanged(int volume);

    void onSliderPressed();

    void onSliderReleased();

    void onSliderMoved(int position);

    void updateTimeDisplay();

    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

    void onPlayerError(QMediaPlayer::Error error, const QString &errorString);

    void showLAVFiltersDownloadDialog();

private:
    void setupUI();

    void setupConnections();

    bool checkFormatSupport(const QString &filePath);

    void setupVideoOutput();

    void cleanupVideoOutput();

    QString formatTime(qint64 milliseconds);

    void updatePositionDisplay(qint64 position, qint64 duration);

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QVideoWidget *m_videoWidget;

    // UI 组件
    QPushButton *m_openButton;
    QPushButton *m_playPauseButton;
    QPushButton *m_stopButton;
    QSlider *m_positionSlider;
    QSlider *m_volumeSlider;
    QLabel *m_timeLabel; // 改为普通QLabel，避免滚动
    ScrollLabel *m_fileNameLabel;

    QTimer *m_positionTimer;
    QString m_currentMedia;
    bool m_hasVideo;
    bool m_isSeeking;
    QLabel *m_volumeLabel;

    void retranslateUI();
};
