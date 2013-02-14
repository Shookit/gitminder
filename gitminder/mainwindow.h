#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "ui_mainwindow.h"
#include <QSettings>
#include <QFileDialog>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QMenu>

namespace Ui {
class MainWindow;
}

static int checkDirtyCallback(const char *, unsigned int, void *);


class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QSystemTrayIcon trayIcon;
    void MainWindow::populateOptions();
    void MainWindow::populateWatchDirectories();

private slots:
    void checkDirty(QString changedFile);
    void iconActivated(QSystemTrayIcon::ActivationReason);
    void on_remove_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_browse_clicked();
    void on_add_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
