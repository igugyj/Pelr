
#include "MediaPlayerWidget.h"

#include <QCloseEvent>
#include <QPainter>
#include <QMessageBox>
#include <QDesktopServices>

// ScrollLabel 实现
ScrollLabel::ScrollLabel(QWidget *parent)
    : QLabel(parent), m_scrollAnimation(new QPropertyAnimation(this, "scrollOffset", this)), m_scrollOffset(0),
      m_needsScrolling(false)
{
    setAlignment(Qt::AlignCenter);
    setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; }");

    m_scrollAnimation->setDuration(15000); // 15秒完成一次滚动
    m_scrollAnimation->setLoopCount(-1);   // 无限循环
    m_scrollAnimation->setEasingCurve(QEasingCurve::Linear);
}

void ScrollLabel::setText(const QString &text)
{
    m_fullText = text;

    // 检查文本是否需要滚动
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(text);
    m_needsScrolling = (textWidth > width() - 10); // 减去内边距

    if (m_needsScrolling)
    {
        // 在文本后添加空格，使滚动更自然
        QString displayText = text + "    ";
        QLabel::setText(displayText);
        m_fullText = displayText;
        startScrolling();
    }
    else
    {
        stopScrolling();
        QLabel::setText(text);
    }
}

void ScrollLabel::startScrolling()
{
    if (!m_needsScrolling)
        return;

    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(m_fullText);
    int containerWidth = width();

    // 计算需要滚动的距离
    int scrollDistance = textWidth - containerWidth + 20;

    if (scrollDistance > 0)
    {
        m_scrollAnimation->setStartValue(0);
        m_scrollAnimation->setEndValue(-scrollDistance);
        m_scrollAnimation->start();
    }
}

void ScrollLabel::stopScrolling()
{
    m_scrollAnimation->stop();
    m_scrollOffset = 0;
    update();
}

void ScrollLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);

    // 重新检查是否需要滚动
    if (!m_fullText.isEmpty())
    {
        QFontMetrics fm(font());
        int textWidth = fm.horizontalAdvance(m_fullText.replace("    ", ""));
        m_needsScrolling = (textWidth > width() - 10);

        if (m_needsScrolling)
        {
            startScrolling();
        }
        else
        {
            stopScrolling();
            QLabel::setText(m_fullText.replace("    ", ""));
        }
    }
}

int ScrollLabel::scrollOffset() const
{
    return m_scrollOffset;
}

void ScrollLabel::setScrollOffset(int offset)
{
    m_scrollOffset = offset;
    update();
}

void ScrollLabel::paintEvent(QPaintEvent *event)
{
    if (m_needsScrolling && m_scrollAnimation->state() == QPropertyAnimation::Running)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::TextAntialiasing);

        // 设置文本颜色
        painter.setPen(palette().text().color());

        // 绘制滚动的文本
        QFontMetrics fm(font());
        int textHeight = fm.height();
        int y = (height() - textHeight) / 2 + fm.ascent();

        painter.drawText(m_scrollOffset, y, m_fullText);

        // 如果文本长度不足，绘制第二份文本以实现循环效果
        int textWidth = fm.horizontalAdvance(m_fullText);
        if (textWidth + m_scrollOffset < width())
        {
            painter.drawText(m_scrollOffset + textWidth, y, m_fullText);
        }
    }
    else
    {
        QLabel::paintEvent(event);
    }
}

// MediaPlayerWidget 实现
MediaPlayerWidget &MediaPlayerWidget::instance()
{
    static MediaPlayerWidget instance;
    return instance;
}

MediaPlayerWidget::MediaPlayerWidget(QWidget *parent)
    : QWidget(parent), m_player(new QMediaPlayer(this)), m_videoWidget(nullptr),
      m_openButton(new QPushButton("打开文件", this)), m_playPauseButton(new QPushButton("播放", this)),
      m_stopButton(new QPushButton("停止", this)), m_positionSlider(new QSlider(Qt::Horizontal, this)),
      m_volumeSlider(new QSlider(Qt::Horizontal, this)), m_timeLabel(new QLabel("00:00/00:00", this)),
      m_fileNameLabel(new ScrollLabel(this)), m_positionTimer(new QTimer(this)), m_hasVideo(false),
      m_isSeeking(false)
{
    setupUI();
    setupConnections();

    // 初始化播放器和音频输出
    m_audioOutput = new QAudioOutput();
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.5);
    m_volumeSlider->setValue(50);

    // 设置位置更新定时器
    m_positionTimer->setInterval(200); // 增加间隔减少更新频率

    // 设置窗口标题
    setWindowTitle("媒体播放器");
}

MediaPlayerWidget::~MediaPlayerWidget()
{
    if (m_videoWidget)
    {
        m_videoWidget->deleteLater();
    }
    delete m_audioOutput;
}

void MediaPlayerWidget::closeEvent(QCloseEvent *event)
{
    onStop();
    hide();
    event->ignore();
}

void MediaPlayerWidget::showLAVFiltersDownloadDialog()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("解码器问题");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("无法播放此视频文件，可能是因为缺少合适的解码器。\n\n"
                   "建议下载并安装 LAV Filters 以获得更好的视频格式支持。");

    // 添加按钮
    QPushButton *downloadButton = msgBox.addButton("下载 LAV Filters", QMessageBox::ActionRole);
    QPushButton *ignoreButton = msgBox.addButton("忽略", QMessageBox::RejectRole);

    msgBox.exec();
    if (msgBox.clickedButton() == downloadButton)
    {
        // 打开下载链接
        QDesktopServices::openUrl(QUrl("https://github.com/Nevcairiel/LAVFilters/releases/tag/0.80"));
    }
}

void MediaPlayerWidget::setupUI()
{
    setFixedSize(500, 200);

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 文件名显示 - 支持滚动
    m_fileNameLabel->setText("未选择媒体文件");
    mainLayout->addWidget(m_fileNameLabel);

    // 进度条和时间显示
    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressLayout->addWidget(m_positionSlider);
    progressLayout->addWidget(m_timeLabel);
    mainLayout->addLayout(progressLayout);

    // 控制按钮和音量
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(m_openButton);
    controlLayout->addWidget(m_playPauseButton);
    controlLayout->addWidget(m_stopButton);
    controlLayout->addWidget(new QLabel("音量:", this));
    controlLayout->addWidget(m_volumeSlider);

    mainLayout->addLayout(controlLayout);

    // 设置控件属性
    m_positionSlider->setRange(0, 0);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setFixedWidth(80);

    m_playPauseButton->setEnabled(false);
    m_stopButton->setEnabled(false);

    // 时间标签设置固定样式和大小
    m_timeLabel->setStyleSheet("QLabel { background-color: transparent; }");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setFixedWidth(180); // 固定宽度避免晃动
}

void MediaPlayerWidget::setupConnections()
{
    // 按钮连接
    connect(m_openButton, &QPushButton::clicked, this, &MediaPlayerWidget::onOpenFile);
    connect(m_playPauseButton, &QPushButton::clicked, this, &MediaPlayerWidget::onPlayPause);
    connect(m_stopButton, &QPushButton::clicked, this, &MediaPlayerWidget::onStop);

    // 滑块连接
    connect(m_positionSlider, &QSlider::sliderPressed, this, &MediaPlayerWidget::onSliderPressed);
    connect(m_positionSlider, &QSlider::sliderReleased, this, &MediaPlayerWidget::onSliderReleased);
    connect(m_positionSlider, &QSlider::sliderMoved, this, &MediaPlayerWidget::onSliderMoved);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MediaPlayerWidget::onVolumeChanged);

    // 播放器连接
    connect(m_player, &QMediaPlayer::positionChanged, this, &MediaPlayerWidget::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MediaPlayerWidget::onDurationChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayerWidget::onStateChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MediaPlayerWidget::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::errorOccurred, this,
            &MediaPlayerWidget::onPlayerError);
    connect(m_positionTimer, &QTimer::timeout, this, &MediaPlayerWidget::updateTimeDisplay);
}

void MediaPlayerWidget::onOpenFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "选择媒体文件",
                                                    "",
                                                    "媒体文件 (*.mp3 *.wav *.flac *.ogg *.mp4 *.avi *.mkv *.mov);;"
                                                    "音频文件 (*.mp3 *.wav *.flac *.ogg);;"
                                                    "视频文件 (*.mp4 *.avi *.mkv *.mov);;"
                                                    "所有文件 (*.*)");

    if (!filePath.isEmpty())
    {
        playMedia(filePath);
    }
}

void MediaPlayerWidget::playMedia(const QString &filePath)
{
    if (!checkFormatSupport(filePath))
    {
        qDebug() << "[MediaPlayer] Unsupported media format:" << filePath;
        m_fileNameLabel->setText("不支持的媒体格式: " + QFileInfo(filePath).fileName());
        return;
    }

    // 停止当前播放
    m_player->stop();

    m_currentMedia = filePath;

    // 根据文件扩展名判断是否有视频
    QString extension = QFileInfo(filePath).suffix().toLower();
    m_hasVideo = (extension == "mp4" || extension == "avi" || extension == "mkv" || extension == "mov");

    if (m_hasVideo)
    {
        setupVideoOutput();
    }
    else
    {
        cleanupVideoOutput();
    }

    m_player->setSource(QUrl::fromLocalFile(filePath));

    // 设置滚动文件名
    QString displayName = "正在播放: " + QFileInfo(filePath).fileName();
    m_fileNameLabel->setText(displayName);

    m_playPauseButton->setEnabled(true);
    m_stopButton->setEnabled(true);

    m_player->play();
}

void MediaPlayerWidget::onPlayPause()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
    {
        m_player->pause();
        m_positionTimer->stop();
    }
    else
    {
        m_player->play();
        m_positionTimer->start();
        if (m_videoWidget && m_videoWidget->isHidden())
            m_videoWidget->show();
    }
}

void MediaPlayerWidget::onStop()
{
    m_player->stop();
    m_player->setSource(QUrl());
    m_positionTimer->stop();
    m_positionSlider->setValue(0);
    m_timeLabel->setText("00:00/00:00");
    m_fileNameLabel->setText("未选择媒体文件");
    if (m_videoWidget && m_videoWidget->isVisible())
        m_videoWidget->hide();
    m_playPauseButton->setEnabled(false);
    m_stopButton->setEnabled(false);
}

void MediaPlayerWidget::onPositionChanged(qint64 position)
{
    if (!m_isSeeking)
    {
        m_positionSlider->setValue(position);
        updatePositionDisplay(position, m_player->duration());
    }
}

void MediaPlayerWidget::onDurationChanged(qint64 duration)
{
    m_positionSlider->setRange(0, duration);
    updatePositionDisplay(m_player->position(), duration);
}

void MediaPlayerWidget::onStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state)
    {
    case QMediaPlayer::PlayingState:
        m_playPauseButton->setText("暂停");
        m_positionTimer->start();
        break;
    case QMediaPlayer::PausedState:
        m_playPauseButton->setText("播放");
        m_positionTimer->stop();
        break;
    case QMediaPlayer::StoppedState:
        m_playPauseButton->setText("播放");
        m_positionTimer->stop();
        m_positionSlider->setValue(0);
        updatePositionDisplay(0, m_player->duration());
        break;
    }
}

void MediaPlayerWidget::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    // 当媒体状态变为无效媒体时，提示用户
    if (status == QMediaPlayer::InvalidMedia)
    {
        qDebug() << "[MediaPlayer] Invalid media file:" << m_currentMedia;

        // 如果是视频文件，提示下载解码器
        // if (m_hasVideo) {
        //     QTimer::singleShot(100, this, &MediaPlayerWidget::showLAVFiltersDownloadDialog);
        // }
    }
}

void MediaPlayerWidget::onPlayerError(QMediaPlayer::Error error, const QString &errorString)
{
    qDebug() << "[MediaPlayer] Player error:" << error << errorString << "file:" << m_currentMedia;

    // 如果是视频文件且出现格式错误，提示下载解码器
    if (m_hasVideo && (error == QMediaPlayer::FormatError || error == QMediaPlayer::ResourceError))
    {
        QTimer::singleShot(100, this, &MediaPlayerWidget::showLAVFiltersDownloadDialog);
    }
}

void MediaPlayerWidget::onVolumeChanged(int volume)
{
    m_audioOutput->setVolume(volume / 100.0);
}

void MediaPlayerWidget::onSliderPressed()
{
    m_isSeeking = true;
    m_positionTimer->stop(); // 停止定时器，避免干扰
}

void MediaPlayerWidget::onSliderReleased()
{
    m_isSeeking = false;
    m_player->setPosition(m_positionSlider->value());

    // 如果正在播放，重新启动定时器
    if (m_player->playbackState() == QMediaPlayer::PlayingState)
    {
        m_positionTimer->start();
    }
}

void MediaPlayerWidget::onSliderMoved(int position)
{
    Q_UNUSED(position)
    // 拖动时更新时间显示，但不更新进度条位置
    updatePositionDisplay(m_positionSlider->value(), m_player->duration());
}

void MediaPlayerWidget::updateTimeDisplay()
{
    // 这个函数现在只由定时器调用，用于平滑更新时间显示
    if (!m_isSeeking && m_player->playbackState() == QMediaPlayer::PlayingState)
    {
        updatePositionDisplay(m_player->position(), m_player->duration());
    }
}

void MediaPlayerWidget::updatePositionDisplay(qint64 position, qint64 duration)
{
    // 统一更新时间显示，避免多处调用导致的不一致
    m_timeLabel->setText(formatTime(position) + "/" + formatTime(duration));
}

QString MediaPlayerWidget::formatTime(qint64 milliseconds)
{
    qint64 seconds = milliseconds / 1000;
    qint64 minutes = seconds / 60;
    seconds = seconds % 60;
    qint64 hours = minutes / 60;
    minutes = minutes % 60;

    if (hours > 0)
    {
        return QString("%1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }
    else
    {
        return QString("%1:%2")
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }
}

bool MediaPlayerWidget::checkFormatSupport(const QString &filePath)
{
    QString extension = QFileInfo(filePath).suffix().toLower();

    // 支持的格式列表
    static const QStringList supportedFormats = {
        "mp3", "wav", "flac", "ogg", "aac",
        "mp4", "avi", "mkv", "mov", "wmv"};

    return supportedFormats.contains(extension);
}

void MediaPlayerWidget::setupVideoOutput()
{
    if (!m_videoWidget)
    {
        m_videoWidget = new QVideoWidget();
        m_videoWidget->setWindowTitle("视频播放 - 媒体播放器");
        m_videoWidget->resize(800, 600);
    }
    m_player->setVideoOutput(m_videoWidget);
    m_videoWidget->show();
}

void MediaPlayerWidget::cleanupVideoOutput()
{
    if (m_videoWidget)
    {
        m_videoWidget->hide();
        // 使用静态转换解决歧义
        m_player->setVideoOutput(static_cast<QVideoWidget *>(nullptr));
    }
}

void MediaPlayerWidget::setVolume(int volume)
{
    m_audioOutput->setVolume(volume / 100.0);
    m_volumeSlider->setValue(volume);
}
