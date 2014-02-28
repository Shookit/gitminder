#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLinkedList>
#include <QSystemTrayIcon>
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

private:
    QLinkedList<FileWatcher*> gitWatchers;
    QLinkedList<NotifyTimer*> notifyTimers;
    QSystemTrayIcon trayIcon;

    void setupSystemTray();
    void setupFileWatchers();
    void populateUIFromRegistry();
    void setupNotifyTimers();
    void updateSystemTray();
    void updateWatchDirectoryRegistry(QString repoPath);
    void updateRepoSettings(QMap<QString, QString> repoSettings);
    void updateAllWatchDirectoryRegistry();
    QMap<QString, QString> getRepoSettings(QString repoPath);

private slots:
    void trayNotifySlot(QString);
    void fileChangedSlot(QString, QString);
    void periodicWatcherSlot(QString repoPath);
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
