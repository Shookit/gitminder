#ifndef NOTIFYTIMER_H
#define NOTIFYTIMER_H

#include <QTimer>

class NotifyTimer : public QObject
{
    Q_OBJECT

public:
    NotifyTimer(QString repoPath, int becameDirtyTimestamp, int notificationDelay);
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
