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
#include "git.h"

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
    QTimer *file_changed_delay_timer;
    QTimer *alternative_file_checker;
    QFileSystemWatcher dirWatcher;
    QStringList recursiveDirectorySearch(QString folder);

private slots:
    void file_changed_delay_timer_timeout();
    void alternative_file_checker_timeout();
    void directoryChangedSlot(QString changedFile);

};
#endif
