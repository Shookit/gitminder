#ifndef GIT_H
#define GIT_H

#include <git2.h>
#include <QString>
#include <QDebug>
#include <QStringList>
#include <QString>
#include <QFileSystemWatcher>
#include <QMainWindow>
#include "mainwindow.h"

int gitRecursiveStatus(QString repoPath);
int directoryChangedCallback(const char *fileName, unsigned int git_status_t, void * numDirty);
QString repoStatusToText(int repoStatus);
QMap<QString, QString> getRepoSettings(QString repoPath);

void updateRepoSettings(QMap<QString, QString> repoSettings);
void updateAllWatchDirectoryData();
void updateWatchDirectoryData(QString repoPath);

#endif // GIT_H
