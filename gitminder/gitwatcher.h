#ifndef TEST_H
#define TEST_H

#include <git2.h>
#include <QString>
#include <QDebug>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QMainWindow>
#include "mainwindow.h"

int directoryChangedCallback(const char *, unsigned int status_flags, void *repoPath);
int gitRecursiveStatus(QString repoPath);
int gitFlatStatus(QString repoPath, QString changedDirectory);

class MainWindow;

class GitWatcher : QObject
{
    Q_OBJECT

public:
    GitWatcher(MainWindow * window, QString repoPath);
    ~GitWatcher();

private:
    MainWindow * window;
    QString repoPath;
    QTimer *timer;
    QFileSystemWatcher fileWatcher;
    QFileSystemWatcher dirWatcher;
    git_repository * gitRepo;

    QStringList recursiveFileSearch(QString folder);
    QStringList recursiveDirectorySearch(QString folder);

private slots:
    void timeout();
    void directoryChangedSlot(QString changedFile);

};
#endif
