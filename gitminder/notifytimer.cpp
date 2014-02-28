#include "notifytimer.h"
#include <QDateTime>
#include <QDebug>

NotifyTimer::NotifyTimer(QString repoPath, int becameDirtyTimestamp, int notificationDelay){
    this->timer = NULL;
    this->repoPath = repoPath;

    qDebug() << "notify in" << becameDirtyTimestamp + notificationDelay - QDateTime::currentMSecsSinceEpoch()/1000;

    if (QDateTime::currentMSecsSinceEpoch()/1000> becameDirtyTimestamp + notificationDelay ){
        startTimer(10);
    }
    else{
        startTimer(becameDirtyTimestamp + notificationDelay - QDateTime::currentMSecsSinceEpoch()/1000);
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
