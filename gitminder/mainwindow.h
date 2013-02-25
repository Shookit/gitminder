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
#include "ui_mainwindow.h"
#include "gitwatcher.h"
#include "notifytimer.h"

namespace Ui {
class MainWindow;
}

class GitWatcher;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void updateWatchDirectoryStatus(QString repoPath);

private:
    QLinkedList<GitWatcher*> gitWatchers;
    QLinkedList<NotifyTimer*> notifyTimers;
    QSystemTrayIcon trayIcon;

    QMap<QString, QString> getRepoSettings(QString repoPath);
    void MainWindow::updateRepoSettings(QMap<QString, QString> repoSettings);
    void setupSystemTray();
    void setupFileWatcher();
    void populateFromSettings();
    void updateAllWatchDirectoryStatus();
    void setupNotifyTimers();

public slots:

private slots:
    void systemTrayClickedSlot(QSystemTrayIcon::ActivationReason);
    void openApp();
    void exitApp();
    void on_remove_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_browse_clicked();
    void on_add_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
