#include "mainwindow.h"


//MainWindow Constructor
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    //Used for registry entry locations
    QCoreApplication::setOrganizationName("shookit");
    QCoreApplication::setOrganizationDomain("shookit.com");
    QCoreApplication::setApplicationName("gitminder");

    ui->setupUi(this);

    setupSystemTray();
    populateFromSettings();
    updateAllWatchDirectoryStatus();
    setupFileWatcher();
    setupNotifyTimers();
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    ui->treeWidget->resizeColumnToContents(0);
}


//MainWindow Destructor
MainWindow::~MainWindow(){
    delete ui;
}


//Functions
void MainWindow::setupFileWatcher(){
    gitWatchers.clear();
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem * item =  ui->treeWidget->topLevelItem(i);
        GitWatcher * watcher = new GitWatcher(this,item->text(0).toStdString().c_str());
        gitWatchers.append(watcher);
    }
}


void MainWindow::setupNotifyTimers(){
    notifyTimers.clear();
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem * item =  ui->treeWidget->topLevelItem(i);
        if (item->text(1) == "Dirty"){
            QMap<QString, QString> repoSettings = getRepoSettings(item->text(0));
            NotifyTimer * timer = new NotifyTimer(&trayIcon, item->text(0), repoSettings["timestamp"]);
            notifyTimers.append(timer);
        }
    }
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
    return repoSettings;
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
}


void MainWindow::setupSystemTray(){
    QIcon icon(":/images/icon_green.png");

    trayIcon.setIcon(icon);

    QAction * openAction = new QAction("Open", this);
    connect(openAction, SIGNAL(triggered()), this, SLOT(openApp()));

    QAction * closeAction = new QAction("Close", this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(exitApp()));

    QMenu * trayMenu = new QMenu(this);
    trayMenu->addAction(openAction);
    trayMenu->addAction(closeAction);
    trayIcon.setContextMenu(trayMenu);

    trayIcon.show();
    connect(&trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(systemTrayClickedSlot(QSystemTrayIcon::ActivationReason)));
}


void MainWindow::populateFromSettings(){
    QSettings settings;

    ui->commit_reminder->setChecked(settings.value("commit_reminder").toBool());
    ui->commit_reminder_time->setValue(settings.value("commit_reminder_time").toInt());

    ui->update_notification->setChecked(settings.value("update_notification").toBool());
    ui->update_notification_frequency->setValue(settings.value("update_notification_frequency").toInt());

    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QTreeWidgetItem * item = new QTreeWidgetItem();

        item->setText(0,settings.value("directory").toString());

        ui->treeWidget->addTopLevelItem(item);
    }
    settings.endArray();
}

void MainWindow::updateAllWatchDirectoryStatus(){
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QString itemText =  ui->treeWidget->topLevelItem(i)->text(0);
        updateWatchDirectoryStatus(itemText);
    }
}


void MainWindow::updateWatchDirectoryStatus(QString repoPath){
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem * item = ui->treeWidget->topLevelItem(i);

        if (repoPath == item->text(0)){
            int repoStatus = gitRecursiveStatus(repoPath);

            if (repoStatus == -1){
                item->setText(1, "Invalid");
                break;
            }

            git_revwalk * walk;
            git_repository * repo;
            git_oid oid;

            git_repository_open(&repo, item->text(0).toStdString().c_str());
            git_revwalk_new(&walk, repo);
            git_revwalk_push_head(walk);
            git_revwalk_next(&oid, walk);
            git_revwalk_free(walk);

            char * commit_id = new char[41]; //Commit ID
            git_oid_tostr(commit_id, 41, &oid);

            QMap<QString,QString> repoSettings;
            repoSettings["directory"] = repoPath;

            if(repoStatus == 0){
                //If clean
                item->setText(1, "Clean");
                repoSettings["commit"] = commit_id;
                repoSettings["status"] = "clean";
            }else if(repoStatus == 1){
                //If dirty
                repoSettings["status"] = "dirty";
                item->setText(1, "Dirty");
                QString prevCommitId = getRepoSettings(repoPath)["commit"];

                if (strcmp(prevCommitId.toStdString().c_str(), commit_id) != 0){
                    //If different commit ID
                    repoSettings["commit"] = commit_id;
                    repoSettings["timestamp"] = QString::number(QDateTime::currentMSecsSinceEpoch()/1000);
                }
                else{
                    //If same commit ID
                    QString prevStatus = getRepoSettings(repoPath)["status"];
                    if (prevStatus.compare("dirty") == 0){
                        //If previously dirty
                    }
                    else if (prevStatus.compare("clean") == 0){
                        //If previously clean
                        repoSettings["commit"] = commit_id;
                        repoSettings["timestamp"] = QString::number(QDateTime::currentMSecsSinceEpoch()/1000);
                    }
                }
            }

            updateRepoSettings(repoSettings);
            delete[] commit_id;
            git_repository_free(repo);

        }
    }
}


void MainWindow::openApp(){
    this->show();
    this->raise();
    this->setFocus();
}

void MainWindow::exitApp(){
    qApp->exit();
}

void MainWindow::systemTrayClickedSlot(QSystemTrayIcon::ActivationReason activationType){
    switch(activationType){
    case QSystemTrayIcon::DoubleClick:
        openApp();
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
    ui->treeWidget->addTopLevelItem(item);
    ui->treeWidget->resizeColumnToContents(0);
    updateWatchDirectoryStatus(ui->lineEdit->text());
    ui->lineEdit->clear();
}


void MainWindow::on_buttonBox_accepted(){
    QSettings settings;
    QList<QMap<QString, QString>> settingsList;
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

    updateAllWatchDirectoryStatus();
    setupFileWatcher();
    setupNotifyTimers();
}


void MainWindow::on_buttonBox_rejected(){
    this->hide();
}
