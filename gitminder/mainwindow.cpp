#include <QSettings>
#include <QFileDialog>
#include <QMenu>
#include <QDateTime>
#include <QDebug>
#include <git2.h>
#include "mainwindow.h"
#include "git.h"
#include "ui_mainwindow.h"
#include "filewatcher.h"

//MainWindow Constructor
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    //Used for registry entry locations
    QCoreApplication::setOrganizationName("shookit");
    QCoreApplication::setOrganizationDomain("shookit.com");
    QCoreApplication::setApplicationName("gitminder");

    ui->setupUi(this);

    //lsRemote();

    setupSystemTray();

    updateAllWatchDirectoryRegistry();
    populateUIFromRegistry();
    updateSystemTray();
    setupFileWatchers();
    setupNotifyTimers();

    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    ui->treeWidget->resizeColumnToContents(0);
}

int cred_acquire_cb(git_cred **out, const char * url, const char * username_from_url, unsigned intallowed_types, void * payload) {
    //Creates a populated git_cred on the 'out' variable
    //qDebug() << "Credding.";
    return 0;
    //return git_cred_ssh_key_new(out, "git", NULL, "C:/Users/Shook/Documents/id_rsa", NULL);
    //git_cred_userpass_plaintext_new(out, username, password);
}


int MainWindow::lsRemote(){
    //Print remote repository information; this depends on libssh2, which is difficult to compile on windows. Required to check remote dirs.
    git_remote *remote = NULL;
    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
    git_repository *repo;
    //const git_remote_head **refs;
    //size_t refs_len, i;

    QString repoPath =  "C:/Users/Shook/Documents/git/gitminder";
    git_repository_open(&repo,repoPath.toStdString().c_str());
    if (giterr_last()!=NULL)
        qDebug() << giterr_last()->message;

    git_remote_load(&remote, repo, "origin");
    if (giterr_last()!=NULL)
        qDebug() << giterr_last()->message;

    callbacks.credentials = cred_acquire_cb;

    git_remote_set_callbacks(remote, &callbacks);
    if (giterr_last()!=NULL)
        qDebug() << giterr_last()->message;

    git_remote_connect(remote, GIT_DIRECTION_FETCH);
    if (giterr_last()!=NULL)
        qDebug() << giterr_last()->message;

    /*if (git_remote_ls(&refs, &refs_len, remote) < 0)
        qDebug() << "blergd" << error;

    for (i = 0; i < refs_len; i++) {
        char oid[GIT_OID_HEXSZ + 1] = {0};
        git_oid_fmt(oid, &refs[i]->oid);
        qDebug()<<oid << refs[i]->name;
    }*/
    exit(0);
    return 0;
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
        connect(gitWatchers.last(), SIGNAL(fileChangedSignal(QString ,QString)), this, SLOT(fileChangedSlot(QString, QString)));
        connect(gitWatchers.last(), SIGNAL(periodicSignal(QString)), this, SLOT(periodicWatcherSlot(QString)));
    }
    settings.endArray();
}


void MainWindow::setupNotifyTimers(){
    qDeleteAll(notifyTimers);
    notifyTimers.clear();

    QSettings settings;
    int commitReminderTime = settings.value("commit_reminder_time").toInt();

    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        if (settings.value("status") == "Dirty"){
            notifyTimers.append(new NotifyTimer(settings.value("directory").toString(), settings.value("timestamp").toInt(), commitReminderTime));
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

    QIcon icon(":/images/icon_green.png");
    trayIcon.setIcon(icon);
    trayIcon.setContextMenu(trayMenu);
    trayIcon.show();
    connect(&trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(systemTrayClickedSlot(QSystemTrayIcon::ActivationReason)));
}


void MainWindow::updateSystemTray(){
    QSettings settings;

    int delayNotification = settings.value("commit_reminder_time").toInt();
    int size = settings.beginReadArray("watch_directories");
    bool changed = false;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        if (settings.value("status") == "Dirty"){
            int duration_dirty = QDateTime::currentMSecsSinceEpoch()/1000 - settings.value("timestamp").toInt() ;
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


void MainWindow::populateUIFromRegistry(){
    QSettings settings;
    ui->treeWidget->clear();
    ui->commit_reminder->setChecked(settings.value("commit_reminder").toBool());
    ui->commit_reminder_time->setValue(settings.value("commit_reminder_time").toInt()/60);

    ui->update_notification->setChecked(settings.value("update_notification").toBool());
    ui->update_notification_frequency->setValue(settings.value("update_notification_frequency").toInt());

    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i){
        settings.setArrayIndex(i);
        QTreeWidgetItem * item = new QTreeWidgetItem();

        item->setText(0,settings.value("directory").toString());
        item->setText(1, settings.value("status").toString());
        ui->treeWidget->addTopLevelItem(item);
    }
    settings.endArray();
    ui->treeWidget->resizeColumnToContents(0);
}


QMap<QString, QString> MainWindow::getRepoSettings(QString repoPath){
    QSettings settings;
    QMap<QString, QString> repoSettings;

    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        if (settings.value("directory").toString().compare(repoPath)==0){
            repoSettings.insert("status", settings.value("status").toString());
            repoSettings.insert("commit", settings.value("commit").toString());
            repoSettings.insert("timestamp", settings.value("timestamp").toString());
        }
    }
    settings.endArray();
    return repoSettings;
}


void MainWindow::updateAllWatchDirectoryRegistry(){
    QSettings settings;
    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        updateWatchDirectoryRegistry(settings.value("directory").toString());
    }
    settings.endArray();
    qDebug()<<"Done updating statuses.";
}


void MainWindow::updateWatchDirectoryRegistry(QString repoPath){
    QMap<QString,QString> repoSettings;
    repoSettings["directory"] = repoPath;

    int repoStatus = getRepoStatus(repoPath);

    if (repoStatus == INVALID){
        repoSettings["status"] = "Invalid";
    }else if(repoStatus == CLEAN){
        char * cur_commit_id = getCommitID(repoPath);
        repoSettings["commit"] = cur_commit_id;
        repoSettings["status"] = "Clean";
        delete[] cur_commit_id;
    }else if(repoStatus == DIRTY){
        char * cur_commit_id = getCommitID(repoPath);
        repoSettings["status"] = "Dirty";
        QString prevCommitId = getRepoSettings(repoPath)["commit"];

        if (strcmp(prevCommitId.toStdString().c_str(), cur_commit_id) != 0){
            //If different commit ID
            repoSettings["commit"] = cur_commit_id;
            repoSettings["timestamp"] = QString::number(QDateTime::currentMSecsSinceEpoch()/1000);
        }
        else{
            //If same commit ID
            QString prevStatus = getRepoSettings(repoPath)["status"];
            if (prevStatus.compare("Dirty") == 0){
                //If previously dirty
            }
            else if (prevStatus.compare("Clean") == 0){
                //If previously clean
                repoSettings["commit"] = cur_commit_id;
                repoSettings["timestamp"] = QString::number(QDateTime::currentMSecsSinceEpoch()/1000);
            }
        }
        delete[] cur_commit_id;
    }
    updateRepoSettings(repoSettings);
}

void MainWindow::updateRepoSettings(QMap<QString, QString> repoSettings){
    QSettings settings;

    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        if (settings.value("directory").toString().compare(repoSettings["directory"])==0){
            if (repoSettings.contains("status"))
                settings.setValue("status", repoSettings["status"]);

            if (repoSettings.contains("commit"))
                settings.setValue("commit", repoSettings["commit"]);

            if (repoSettings.contains("timestamp"))
                settings.setValue("timestamp", repoSettings["timestamp"]);
        }
    }
    settings.endArray();
}


//Slots
void MainWindow::trayNotifySlot(QString repoPath){
    QSettings settings;
    qDebug() << "message popped up";
    updateSystemTray();

    QString numMins = QString::number(settings.value("commit_reminder_time").toInt()/60);

    this->trayIcon.showMessage(repoPath, "This repository hasn't been committed in over " + numMins + " minutes.");
}


void MainWindow::fileChangedSlot(QString repoPath, QString changedFile){  //QString is the repoPath
    char * cur_commit_id = getCommitID(repoPath);
    QString prevCommitId = getRepoSettings(repoPath)["commit"];

    //If different commit_id, just rescan whole repo
    if (strcmp(prevCommitId.toStdString().c_str(), cur_commit_id) != 0){
        updateWatchDirectoryRegistry(repoPath);
        populateUIFromRegistry();
        updateSystemTray();
        setupFileWatchers();
        setupNotifyTimers();
    }
    else{
        QString prevRepoStatus = getRepoSettings(repoPath)["status"];
        QString curChangedFileStatus= getFileStatus(repoPath, changedFile);

        qDebug() << curChangedFileStatus;

        //If file is clean, rescan repo (unsure about other files)
        if (curChangedFileStatus=="Clean"){
            updateWatchDirectoryRegistry(repoPath);
            populateUIFromRegistry();
            updateSystemTray();
            setupFileWatchers();
            setupNotifyTimers();
        }


        //If file is currently dirty & previously clean, mark dirty (became dirty)
        else if (curChangedFileStatus=="Dirty" && prevRepoStatus == "Clean"){
            updateWatchDirectoryRegistry(repoPath);
            populateUIFromRegistry();
            updateSystemTray();
            setupFileWatchers();
            setupNotifyTimers();
        }

        //If file is currently and previously dirty, do nothing (was already dirty)
        else if (curChangedFileStatus=="Dirty" && prevRepoStatus == "Dirty"){

        }

        else{
            qDebug()<<"illegal state" << prevRepoStatus << curChangedFileStatus;
        }
    }
}

void MainWindow::periodicWatcherSlot(QString repoPath){  //QString is the repoPath
    updateWatchDirectoryRegistry(repoPath);
    populateUIFromRegistry();
    updateSystemTray();
    setupFileWatchers();
    setupNotifyTimers();
}


void MainWindow::openAppSlot(){
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
    if (ui->checkBox->isChecked()){
        QDir dir(ui->lineEdit->text());
        QStringList entryList = dir.entryList(QDir::NoDotAndDotDot|QDir::Dirs);
        for(int i = 0; i<entryList.count(); ++i){
            QTreeWidgetItem * item = new QTreeWidgetItem();
            if (ui->treeWidget->findItems(dir.absolutePath() + "/" + entryList.at(i),0,0).count()==0){
                item->setText(0,dir.absolutePath() + "/" + entryList.at(i));
                ui->treeWidget->addTopLevelItem(item);
            }
        }
    }
    else{
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0,ui->lineEdit->text());
        ui->treeWidget->addTopLevelItem(item);
    }
    ui->lineEdit->clear();
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    ui->treeWidget->resizeColumnToContents(0);
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
    settings.setValue("commit_reminder_time", ui->commit_reminder_time->value()*60) ;
    settings.setValue("update_notification", ui->update_notification->checkState()) ;
    settings.setValue("update_notification_frequency", ui->update_notification_frequency->value());

    //Hide
    this->hide();

    updateAllWatchDirectoryRegistry();
    populateUIFromRegistry();
    updateSystemTray();
    setupFileWatchers();
    setupNotifyTimers();
}


void MainWindow::on_buttonBox_rejected(){
    this->hide();
}
