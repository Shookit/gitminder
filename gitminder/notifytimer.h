#ifndef NOTIFYTIMER_H
#define NOTIFYTIMER_H

#include <QString>
#include <QDateTime>
#include <QDebug>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>


class NotifyTimer : QObject
{
    Q_OBJECT

public:
    NotifyTimer(QSystemTrayIcon * trayIcon, QString repoPath, QString timestamp);

private:
    QString repoPath;
    QSystemTrayIcon * trayIcon;
    QTimer * timer;
    void NotifyTimer::showMessage();
    void NotifyTimer::startTimer(int num_seconds);

private slots:
    void timeout();
};

#endif // NOTIFYTIMER_H
