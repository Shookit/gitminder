#ifndef TEST_H
#define TEST_H

#include <git2.h>
#include <QString>
#include <QDebug>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QMainWindow>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include "mainwindow.h"
#include "git.h"

int directoryChangedCallback(const char *, unsigned int status_flags, void *repoPath);
int gitRecursiveStatus(QString repoPath);
int gitFlatStatus(QString repoPath, QString changedDirectory);

class MainWindow;

class FileWatcher : public QObject
{
    Q_OBJECT

public:
    FileWatcher(QString repoPath);
    ~FileWatcher();

signals:
     void fileChangedSignal(QString repoPath);

private:
    QString repoPath;
    QTimer *timer;
    QFileSystemWatcher dirWatcher;
    QStringList recursiveDirectorySearch(QString folder);

private slots:
    void timeout();
    void directoryChangedSlot(QString changedFile);

};
#endif
