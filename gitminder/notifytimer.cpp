#include "notifytimer.h"

NotifyTimer::NotifyTimer(QSystemTrayIcon *trayIcon, QString repoPath, QString timestamp)
{
    this->repoPath = repoPath;
    this->trayIcon = trayIcon;
    QSettings settings;

    int delayNotification = settings.value("commit_reminder_time").toInt()*60;
    qDebug() << "notify in" << timestamp.toInt() + delayNotification - QDateTime::currentMSecsSinceEpoch ()/1000;

    if (QDateTime::currentMSecsSinceEpoch ()/1000> timestamp.toInt() + delayNotification ){
        showMessage();
    }
    else{
        startTimer(timestamp.toInt() + delayNotification - QDateTime::currentMSecsSinceEpoch ()/1000);
    }
}

NotifyTimer::~NotifyTimer(){
    delete &repoPath;
}

void NotifyTimer::startTimer(int num_seconds){
    this->timer = new QTimer();
    this->timer->setSingleShot(true);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(num_seconds*1000);
}


void NotifyTimer::timeout(){
    showMessage();
}


void NotifyTimer::showMessage(){
    QSettings settings;
    qDebug() << "message popped up";
    this->trayIcon->showMessage(repoPath, "This repository hasn't been committed in over " + settings.value("commit_reminder_time").toString() + " minutes.");
    startTimer(5*60);
}
