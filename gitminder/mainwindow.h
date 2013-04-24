#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QFileDialog>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QFuture>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QLinkedList>
#include <QMap>
#include <QFileSystemWatcher>
#include <git2.h>
#include "git.h"
#include "ui_mainwindow.h"
#include "filewatcher.h"
#include "notifytimer.h"

namespace Ui {
class MainWindow;
}

class FileWatcher;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void updateWatchDirectoryData(QString repoPath);

private:
    QLinkedList<FileWatcher*> gitWatchers;
    QLinkedList<NotifyTimer*> notifyTimers;
    QSystemTrayIcon trayIcon;

    void setupSystemTray();
    void setupFileWatchers();
    void populateUI();
    void setupNotifyTimers();
    void updateSystemTray();

private slots:
    void trayNotifySlot(QString);
    void fileChangedSlot(QString);
    void systemTrayClickedSlot(QSystemTrayIcon::ActivationReason);
    void openAppSlot();
    void exitAppSlot();

    void on_remove_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_browse_clicked();
    void on_add_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
