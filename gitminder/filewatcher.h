#ifndef TEST_H
#define TEST_H

#include<QTimer>
#include<QFileSystemWatcher>

class MainWindow;

class FileWatcher : public QObject
{
    Q_OBJECT

public:
    FileWatcher(QString repoPath);

signals:
     void fileChangedSignal(QString repoPath, QString changedFile);
     void periodicSignal(QString repoPath);

private:
    QString repoPath;
    QTimer *periodic_file_checker;
    QFileSystemWatcher dirWatcher;
    QStringList recursiveDirectorySearch(QString folder);

private slots:
    void alternative_file_checker_timeout();
    void fileChangedSlot(QString changedFile);

};
#endif
