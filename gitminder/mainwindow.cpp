#include "mainwindow.h"


//MainWindow Constructor
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    //Used for registry entry locations
    QCoreApplication::setOrganizationName("shookit");
    QCoreApplication::setOrganizationDomain("shookit.com");
    QCoreApplication::setApplicationName("gitminder");

    ui->setupUi(this);

    setupSystemTray();

    updateAllWatchDirectoryData();
    setupFileWatchers();
    setupNotifyTimers();
    updateSystemTray();

    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    ui->treeWidget->resizeColumnToContents(0);
}


//MainWindow Destructor
MainWindow::~MainWindow(){
    delete ui;

    qDeleteAll(notifyTimers);
    notifyTimers.clear();

    qDeleteAll(gitWatchers);
    gitWatchers.clear();
}


//Functions
void MainWindow::setupFileWatchers(){
    qDeleteAll(gitWatchers);
    gitWatchers.clear();

    QSettings settings;
    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        gitWatchers.append(new FileWatcher(settings.value("directory").toString()));
        connect(gitWatchers.last(), SIGNAL(fileChangedSignal(QString)), this, SLOT(fileChangedSlot(QString)));
    }
    settings.endArray();
}


void MainWindow::setupNotifyTimers(){
    qDeleteAll(notifyTimers);
    notifyTimers.clear();

    QSettings settings;
    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        if (settings.value("status") == "dirty"){
            notifyTimers.append(new NotifyTimer(settings.value("directory").toString(), settings.value("timestamp").toString()));
            connect(notifyTimers.last(), SIGNAL(notifyTimeoutSignal(QString)), this, SLOT(trayNotifySlot(QString)));
        }
    }
    settings.endArray();
}


void MainWindow::setupSystemTray(){
    QAction * openAction = new QAction("Open", this);
    connect(openAction, SIGNAL(triggered()), this, SLOT(openAppSlot()));

    QAction * closeAction = new QAction("Close", this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(exitAppSlot()));

    QMenu * trayMenu = new QMenu(this);
    trayMenu->addAction(openAction);
    trayMenu->addAction(closeAction);
    trayIcon.setContextMenu(trayMenu);

    trayIcon.show();
    connect(&trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(systemTrayClickedSlot(QSystemTrayIcon::ActivationReason)));
}


void MainWindow::updateSystemTray(){
    QSettings settings;

    int delayNotification = settings.value("commit_reminder_time").toInt()*60;
    int size = settings.beginReadArray("watch_directories");
    bool changed = false;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        if (settings.value("status") == "dirty"){
            int duration_dirty = QDateTime::currentMSecsSinceEpoch ()/1000 - settings.value("timestamp").toInt() ;
            if (duration_dirty > delayNotification){
                QIcon icon(":/images/icon_red.png");
                trayIcon.setIcon(icon);
                changed = true;
            }
        }
    }
    settings.endArray();

    if (changed == false){
        QIcon icon(":/images/icon_green.png");
        trayIcon.setIcon(icon);
    }
}


void MainWindow::populateUI(){
    QSettings settings;
    ui->treeWidget->clear();
    ui->commit_reminder->setChecked(settings.value("commit_reminder").toBool());
    ui->commit_reminder_time->setValue(settings.value("commit_reminder_time").toInt());

    ui->update_notification->setChecked(settings.value("update_notification").toBool());
    ui->update_notification_frequency->setValue(settings.value("update_notification_frequency").toInt());

    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i){
        settings.setArrayIndex(i);
        QTreeWidgetItem * item = new QTreeWidgetItem();

        item->setText(0,settings.value("directory").toString());

        int gitStatus = gitRecursiveStatus(settings.value("directory").toString());
        if(gitStatus == 0){
            item->setText(1, "Clean");
        }else if(gitStatus == 1){
            item->setText(1, "Dirty");
        }else if(gitStatus == -1){
            item->setText(1, "Invalid");
        }

        ui->treeWidget->addTopLevelItem(item);
    }
    settings.endArray();
    ui->treeWidget->resizeColumnToContents(0);
}


//Slots
void MainWindow::trayNotifySlot(QString repoPath){
    QSettings settings;
    qDebug() << "message popped up";
    updateSystemTray();
    this->trayIcon.showMessage(repoPath, "This repository hasn't been committed in over " + settings.value("commit_reminder_time").toString() + " minutes.");
}


void MainWindow::fileChangedSlot(QString){  //QString is the repoPath
    populateUI();
    updateAllWatchDirectoryData();
    setupFileWatchers();
    setupNotifyTimers();
    updateSystemTray();
}


void MainWindow::openAppSlot(){
    populateUI();
    this->show();
    this->raise();
    this->setFocus();
}


void MainWindow::exitAppSlot(){
    qApp->exit();
}


void MainWindow::systemTrayClickedSlot(QSystemTrayIcon::ActivationReason activationType){
    switch(activationType){
    case QSystemTrayIcon::DoubleClick:
        openAppSlot();
        break;
    case QSystemTrayIcon::Context:
        trayIcon.contextMenu()->show();
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    case QSystemTrayIcon::Trigger:
        break;
    default:
        break;
    }
}


//Click handlers
void MainWindow::on_remove_clicked(){
    qDeleteAll(ui->treeWidget->selectedItems());
}


void MainWindow::on_browse_clicked(){
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.exec();
    for (int i=0; i<dialog.selectedFiles().count(); i++){
        ui->lineEdit->setText(dialog.selectedFiles()[i]);
    }
    this->raise();
    this->setFocus();
}


void MainWindow::on_add_clicked(){
    QTreeWidgetItem * item = new QTreeWidgetItem();
    item->setText(0,ui->lineEdit->text());
    item->setText(1,repoStatusToText(gitRecursiveStatus(ui->lineEdit->text())));
    ui->treeWidget->addTopLevelItem(item);
    ui->lineEdit->clear();
}


void MainWindow::on_buttonBox_accepted(){
    QSettings settings;
    QList<QMap<QString, QString> > settingsList;
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);

    //Watch directories
    for (int i=0; i < ui->treeWidget->topLevelItemCount(); ++i){
        settingsList.push_back(getRepoSettings(ui->treeWidget->topLevelItem(i)->text(0)));
        if (settingsList[i]["directory"]==""){
            settingsList[i]["directory"] = ui->treeWidget->topLevelItem(i)->text(0);
        }
    }

    settings.beginWriteArray("watch_directories");
    for (int i=0; i < settingsList.count(); ++i){
        settings.setArrayIndex(i);
        settings.setValue("directory", settingsList[i]["directory"]);
        settings.setValue("status", settingsList[i]["status"]);
        settings.setValue("commit", settingsList[i]["commit"]);
        settings.setValue("timestamp", settingsList[i]["timestamp"]);
    }
    settings.endArray();

    //Options page
    settings.setValue("commit_reminder", ui->commit_reminder->checkState());
    settings.setValue("commit_reminder_time", ui->commit_reminder_time->value()) ;
    settings.setValue("update_notification", ui->update_notification->checkState()) ;
    settings.setValue("update_notification_frequency", ui->update_notification_frequency->value());

    //Hide
    this->hide();

    updateAllWatchDirectoryData();
    setupFileWatchers();
    setupNotifyTimers();
    updateSystemTray();
}


void MainWindow::on_buttonBox_rejected(){
    this->hide();
}
