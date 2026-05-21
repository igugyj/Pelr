
#include "datetimepickerdialog.h"
#include <QShortcut>
#include <QKeySequence>
#include <QApplication>

DateTimePickerDialog::DateTimePickerDialog(
    QWidget *parent,
    const QDateTime &initialDateTime,
    const QString &title)
    : QDialog(parent), m_selectedDateTime(initialDateTime)
{
    setWindowTitle(title);
    setupUI();
    setupShortcuts();
}

DateTimePickerDialog::~DateTimePickerDialog()
{
}

void DateTimePickerDialog::setupUI()
{
    setModal(true);
    // 大幅缩小对话框尺寸，因为使用了更紧凑的 QDateEdit
    setFixedSize(600, 450); // 从800x600缩小到400x300

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // 快速选择按钮
    QHBoxLayout *quickSelectLayout = new QHBoxLayout();
    QPushButton *todayBtn = new QPushButton(tr("今天"), this);
    QPushButton *tomorrowBtn = new QPushButton(tr("明天"), this);
    QPushButton *nextWeekBtn = new QPushButton(tr("下周"), this);

    // 设置按钮尺寸
    todayBtn->setFixedHeight(30);
    tomorrowBtn->setFixedHeight(30);
    nextWeekBtn->setFixedHeight(30);

    connect(todayBtn, &QPushButton::clicked, this, &DateTimePickerDialog::onTodayClicked);
    connect(tomorrowBtn, &QPushButton::clicked, this, &DateTimePickerDialog::onTomorrowClicked);
    connect(nextWeekBtn, &QPushButton::clicked, this, &DateTimePickerDialog::onNextWeekClicked);

    quickSelectLayout->addWidget(todayBtn);
    quickSelectLayout->addWidget(tomorrowBtn);
    quickSelectLayout->addWidget(nextWeekBtn);
    quickSelectLayout->addStretch();

    // 日期选择 - 使用 QDateEdit
    QHBoxLayout *dateLayout = new QHBoxLayout();
    dateLayout->addWidget(new QLabel(tr("日期:"), this));

    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setDate(m_selectedDateTime.date());
    m_dateEdit->setCalendarPopup(true); // 启用日历弹出
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    m_dateEdit->setFixedWidth(150);

    dateLayout->addWidget(m_dateEdit);
    dateLayout->addStretch();

    // 时间选择
    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->addWidget(new QLabel(tr("时间:"), this));

    m_timeEdit = new QTimeEdit(this);
    m_timeEdit->setTime(m_selectedDateTime.time());
    m_timeEdit->setDisplayFormat("HH:mm");
    m_timeEdit->setTimeRange(QTime(0, 0), QTime(23, 59));
    m_timeEdit->setFixedWidth(120);

    // 时间预设按钮 - 使用更紧凑的文本
    QPushButton *morningBtn = new QPushButton("9:00", this);
    QPushButton *afternoonBtn = new QPushButton("14:00", this);
    QPushButton *eveningBtn = new QPushButton("19:00", this);

    // 设置按钮尺寸
    morningBtn->setFixedSize(50, 25);
    afternoonBtn->setFixedSize(50, 25);
    eveningBtn->setFixedSize(50, 25);

    connect(morningBtn, &QPushButton::clicked, [this]()
            { m_timeEdit->setTime(QTime(9, 0)); });

    connect(afternoonBtn, &QPushButton::clicked, [this]()
            { m_timeEdit->setTime(QTime(14, 0)); });

    connect(eveningBtn, &QPushButton::clicked, [this]()
            { m_timeEdit->setTime(QTime(19, 0)); });

    timeLayout->addWidget(m_timeEdit);
    timeLayout->addStretch();
    timeLayout->addWidget(morningBtn);
    timeLayout->addWidget(afternoonBtn);
    timeLayout->addWidget(eveningBtn);

    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // 添加到主布局
    mainLayout->addLayout(quickSelectLayout);
    mainLayout->addWidget(new QLabel(tr("选择日期:"), this));
    mainLayout->addLayout(dateLayout);
    mainLayout->addWidget(new QLabel(tr("选择时间:"), this));
    mainLayout->addLayout(timeLayout);
    mainLayout->addStretch(); // 添加弹性空间
    mainLayout->addWidget(buttonBox);

    // 修正信号连接 - 连接 QDateEdit 的信号
    connect(m_dateEdit, &QDateEdit::dateChanged, this, &DateTimePickerDialog::onDateChanged);
    connect(m_timeEdit, &QTimeEdit::timeChanged, [this](const QTime &time)
            { m_selectedDateTime.setTime(time); });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void DateTimePickerDialog::setupShortcuts()
{
    // 快捷键支持
    new QShortcut(QKeySequence(Qt::Key_Return), this, [this]() { accept(); });
    new QShortcut(QKeySequence(Qt::Key_Enter), this, [this]() { accept(); });
    new QShortcut(QKeySequence(Qt::Key_Escape), this, [this]() { reject(); });
}

QDateTime DateTimePickerDialog::getSelectedDateTime() const
{
    return m_selectedDateTime;
}

QDateTime DateTimePickerDialog::getDateTime(QWidget *parent,
                                            const QDateTime &initialDateTime,
                                            bool *ok)
{
    DateTimePickerDialog dialog(parent, initialDateTime);
    bool accepted = (dialog.exec() == QDialog::Accepted);

    if (ok)
    {
        *ok = accepted;
    }

    return accepted ? dialog.getSelectedDateTime() : initialDateTime;
}

void DateTimePickerDialog::onDateChanged(const QDate &date)
{
    m_selectedDateTime.setDate(date);
}

void DateTimePickerDialog::onTodayClicked()
{
    QDate today = QDate::currentDate();
    m_dateEdit->setDate(today); // 改为设置 QDateEdit 的日期
    m_selectedDateTime.setDate(today);
}

void DateTimePickerDialog::onTomorrowClicked()
{
    QDate tomorrow = QDate::currentDate().addDays(1);
    m_dateEdit->setDate(tomorrow); // 改为设置 QDateEdit 的日期
    m_selectedDateTime.setDate(tomorrow);
}

void DateTimePickerDialog::onNextWeekClicked()
{
    QDate nextWeek = QDate::currentDate().addDays(7);
    m_dateEdit->setDate(nextWeek); // 改为设置 QDateEdit 的日期
    m_selectedDateTime.setDate(nextWeek);
}

void DateTimePickerDialog::retranslateUI()
{
}