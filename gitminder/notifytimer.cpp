#include "notifytimer.h"

NotifyTimer::NotifyTimer(QString repoPath, QString timestamp)
{
    this->timer = NULL;
    this->repoPath = repoPath;
    QSettings settings;

    int delayNotification = settings.value("commit_reminder_time").toInt()*60;
    qDebug() << "notify in" << timestamp.toInt() + delayNotification - QDateTime::currentMSecsSinceEpoch ()/1000;

    if (QDateTime::currentMSecsSinceEpoch ()/1000> timestamp.toInt() + delayNotification ){
        startTimer(10);
    }
    else{
        startTimer(timestamp.toInt() + delayNotification - QDateTime::currentMSecsSinceEpoch ()/1000);
    }
}


NotifyTimer::~NotifyTimer(){
    delete this->timer;
}


void NotifyTimer::startTimer(int num_seconds){
    if (this->timer!=NULL){
        delete this->timer;
    }
    this->timer = new QTimer();
    this->timer->setSingleShot(true);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));

    timer->start(num_seconds*1000);
}


void NotifyTimer::timeout(){
    emit notifyTimeoutSignal(this->repoPath);
    startTimer(15*60);
}
