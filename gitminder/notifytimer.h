#ifndef NOTIFYTIMER_H
#define NOTIFYTIMER_H

#include <QString>
#include <QDateTime>
#include <QDebug>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>


class NotifyTimer : public QObject
{
    Q_OBJECT

public:
    NotifyTimer(QString repoPath, QString timestamp);
    ~NotifyTimer();

private:
    QString repoPath;
    QTimer * timer;

    void startTimer(int num_seconds);

signals:
    void notifyTimeoutSignal(QString repoPath);

private slots:
    void timeout();
};

#endif // NOTIFYTIMER_H
