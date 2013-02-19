#include "mainwindow.h"

//MainWindow Constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Used for registry entry locations
    QCoreApplication::setOrganizationName("shookit");
    QCoreApplication::setOrganizationDomain("shookit.com");
    QCoreApplication::setApplicationName("gitminder");

    ui->setupUi(this);
    setupSystemTray();
    populateOptionsFromSettings();
    getWatchDirectoryNamesFromSettings();
    updateAllWatchDirectoryStatus();
    setupFileWatcher();
    setupNotifyTimers();
}


//MainWindow Destructor
MainWindow::~MainWindow()
{
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
    ui->treeWidget->resizeColumnToContents(0);
}


void MainWindow::setupNotifyTimers(){
    notifyTimers.clear();
    QSettings settings;
    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        if (settings.value("status").toString() == "dirty"){
            NotifyTimer * timer = new NotifyTimer(&trayIcon, settings.value("directory").toString(), settings.value("timestamp").toString());
            notifyTimers.append(timer);
        }
    }
    settings.endArray();
}


void MainWindow::setupSystemTray(){
    QIcon icon("stop.png");
    trayIcon.setIcon(icon);

    QAction * openAction = new QAction(tr("Open"), this);
    QAction * closeAction = new QAction(tr("Close"), this);
    QMenu * trayMenu = new QMenu(this);
    trayMenu->addAction(openAction);
    trayMenu->addAction(closeAction);
    trayIcon.setContextMenu(trayMenu);

    trayIcon.show();
    connect(&trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(systemTrayClickedSlot(QSystemTrayIcon::ActivationReason)));
}


void MainWindow::populateOptionsFromSettings()
{
    QSettings settings;

    ui->commit_reminder->setChecked(settings.value("commit_reminder").toBool());
    ui->commit_reminder_time->setValue(settings.value("commit_reminder_time").toInt());

    ui->update_notification->setChecked(settings.value("update_notification").toBool());
    ui->update_notification_frequency->setValue(settings.value("update_notification_frequency").toInt());
}


void MainWindow::getWatchDirectoryNamesFromSettings()
{
    ui->treeWidget->clear();
    QSettings settings;
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
    ui->treeWidget->resizeColumnToContents(0);
}


void MainWindow::updateWatchDirectoryStatus(QString repoPath){
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem * item = ui->treeWidget->topLevelItem(i);

        if (repoPath == item->text(0)){
            git_revwalk * walk;
            git_repository * repo;
            git_oid oid;

            git_repository_open(&repo, item->text(0).toStdString().c_str());
            git_revwalk_new(&walk, repo);
            git_revwalk_push_head(walk);
            git_revwalk_next(&oid, walk);

            char * commit_id = new char[41]; //Commit ID
            git_oid_tostr(commit_id, 41, &oid);

            //git_commit * commit;
            //git_commit_lookup(&commit, repo, &oid);
            //git_time_t tyme;
            //tyme = git_commit_time(commit);

            QSettings settings;
            int size = settings.beginReadArray("watch_directories");
            for (int i = 0; i < size; ++i) {
                settings.setArrayIndex(i);
                if (settings.value("directory").toString() == repoPath){
                    break;
                }
            }

            int repoStatus = gitRecursiveStatus(item->text(0));

            if (repoStatus == -1){
                item->setText(1, "Invalid");
            }else if(repoStatus == 0){
                //If clean
                item->setText(1, "Clean");
                settings.setValue("status", "clean");
                settings.setValue("commit", commit_id);
            }else if(repoStatus == 1){
                //If dirty
                if (strcmp(settings.value("commit").toString().toStdString().c_str(), commit_id)){
                    //If different commit ID
                    settings.setValue("commit", commit_id);
                    settings.setValue("timestamp", QDateTime::currentMSecsSinceEpoch()/1000);
                }
                else if (!strcmp(settings.value("commit").toString().toStdString().c_str(), commit_id)){
                    //If same commit ID
                    if (settings.value("status").toString().compare("dirty") == 0){
                        //If previously dirty:
                    }
                    else if (settings.value("status").toString().compare("clean") == 0){
                        //If previously clean:
                        settings.setValue("commit", commit_id);
                        settings.setValue("timestamp", QDateTime::currentMSecsSinceEpoch()/1000);
                    }
                }
                settings.setValue("status", "dirty");
                item->setText(1, "Dirty");

            }
            settings.endArray();
        }
    }
    setupNotifyTimers();
}


void MainWindow::systemTrayClickedSlot(QSystemTrayIcon::ActivationReason activationType)
{
    switch(activationType)
    {
    case QSystemTrayIcon::DoubleClick:
        this->show();
        this->raise();
        this->setFocus();
        break;
    case QSystemTrayIcon::Context:
        trayIcon.contextMenu()->show();
        break;
    case QSystemTrayIcon::MiddleClick:
        break;
    case QSystemTrayIcon::Trigger:
        break;
    }
}


//Click handlers
void MainWindow::on_remove_clicked()
{
    qDeleteAll(ui->treeWidget->selectedItems());
}


void MainWindow::on_browse_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.exec();
    for (int i=0; i<dialog.selectedFiles().count(); i++){
        ui->lineEdit->setText(dialog.selectedFiles()[i]);
    }
    this->raise();
    this->setFocus();
}


void MainWindow::on_add_clicked()
{
    QTreeWidgetItem * item = new QTreeWidgetItem();
    item->setText(0,ui->lineEdit->text());
    ui->treeWidget->addTopLevelItem(item);
    updateWatchDirectoryStatus(ui->lineEdit->text());
    ui->lineEdit->clear();
}


void MainWindow::on_buttonBox_accepted()
{
    QSettings settings;
    //Watch directories
    settings.beginWriteArray("watch_directories");
    for (int i=0; i < ui->treeWidget->topLevelItemCount(); ++i){
        settings.setArrayIndex(i);
        settings.setValue("directory", ui->treeWidget->topLevelItem(i)->text(0));
    }
    settings.endArray();

    //Options page
    settings.setValue("commit_reminder", ui->commit_reminder->checkState());
    settings.setValue("commit_reminder_time", ui->commit_reminder_time->value()) ;
    settings.setValue("update_notification", ui->update_notification->checkState()) ;
    settings.setValue("update_notification_frequency", ui->update_notification_frequency->value());

    //Hide
    this->hide();
}


void MainWindow::on_buttonBox_rejected()
{
    this->hide();
}
