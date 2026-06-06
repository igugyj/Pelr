
#pragma once

#include <QDialog>
#include <QDateTime>
#include <QDateEdit>
#include <QTimeEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class DateTimePickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DateTimePickerDialog(QWidget *parent = nullptr,
                                  const QDateTime &initialDateTime = QDateTime::currentDateTime(),
                                  const QString &title = "Select Date and Time");

    ~DateTimePickerDialog();

    QDateTime getSelectedDateTime() const;

    static QDateTime getDateTime(QWidget *parent = nullptr,
                                 const QDateTime &initialDateTime = QDateTime::currentDateTime(),
                                 bool *ok = nullptr);

private slots:

    void retranslateUI();

    void onTodayClicked();

    void onTomorrowClicked();

    void onNextWeekClicked();

    void onDateChanged(const QDate &date);

private:
    void setupUI();

    void setupShortcuts();

    QPushButton *m_todayBtn;
    QPushButton *m_tomorrowBtn;
    QPushButton *m_nextWeekBtn;
    QLabel *m_labelDate;
    QLabel *m_labelTime;
    QLabel *m_labelSelectDate;
    QLabel *m_labelSelectTime;
    QDateEdit *m_dateEdit;
    QTimeEdit *m_timeEdit;
    QDateTime m_selectedDateTime;
};
