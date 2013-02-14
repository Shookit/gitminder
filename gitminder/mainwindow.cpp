#include "mainwindow.h"
#include <git2.h>
#include <QFuture>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QFileSystemWatcher>

//Colon after constructor is an 'initialization list'
//Double colon is scope modifier
//Classname::Classname is constructor
//Classname::Function is a class function (defined outside of the actual class definition)
//qDebug() << "hi";

//Constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //QFuture<void> checkDirty =  QtConcurrent::run(this, &MainWindow::checkDirty);
    //QFuture<void> checkRemoteUpdate=  QtConcurrent::run(this, &MainWindow::checkRemoteUpdate);

    QFileSystemWatcher * fileWatcher = new QFileSystemWatcher(this);
    fileWatcher->addPath("C:\\Users\\Matthew\\Documents\\git\\gitminder");
    QObject::connect(fileWatcher,SIGNAL(directoryChanged(QString)), this, SLOT(checkDirty(QString)));

    QCoreApplication::setOrganizationName("shookit");
    QCoreApplication::setOrganizationDomain("shookit.com");
    QCoreApplication::setApplicationName("gitminder");

    //Create system tray
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
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

//Destructor
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::checkDirty(QString changedFile)
{
    git_repository *repo;
    git_repository_open(&repo, changedFile.toStdString().c_str());
    git_status_foreach(repo, &checkDirtyCallback, NULL);
}

static int checkDirtyCallback(const char * fileName, unsigned int, void *){
    qDebug() << fileName;
    return 0;
}

void MainWindow::populateWatchDirectories()
{
    QSettings settings;

    //Populate watch directories
    ui->treeWidget->clear();
    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QTreeWidgetItem * item = new QTreeWidgetItem();

        item->setText(0,settings.value("directory").toString());

        git_repository *repo;
        int result = git_repository_open(&repo, settings.value("directory").toString().toStdString().c_str());

        if (result == 0){
            item->setText(1, "Valid");
        }
        else {
            item->setText(1, "Invalid");
        }

        ui->treeWidget->addTopLevelItem(item);
    }
    ui->treeWidget->resizeColumnToContents(0);
    settings.endArray();
}

void MainWindow::populateOptions()
{
    //Populate options page
    QSettings settings;

    ui->commit_reminder->setChecked(settings.value("commit_reminder").toBool());
    ui->commit_reminder_time->setValue(settings.value("commit_reminder_time").toInt());

    ui->update_notification->setChecked(settings.value("update_notification").toBool());
    ui->update_notification_frequency->setValue(settings.value("update_notification_frequency").toInt());
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason activationType)
{
    switch(activationType)
    {
    case QSystemTrayIcon::DoubleClick:
        this->show();
        this->raise();
        this->setFocus();
        populateOptions();
        populateWatchDirectories();
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
}

void MainWindow::on_add_clicked()
{
    QTreeWidgetItem * item = new QTreeWidgetItem();
    item->setText(0,ui->lineEdit->text());
    git_repository *repo;
    int result = git_repository_open(&repo, ui->lineEdit->text().toStdString().c_str());

    if (result == 0){
        item->setText(1, "Valid");
    }
    else {
        item->setText(1, "Invalid");
    }

    ui->treeWidget->addTopLevelItem(item);
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->sortByColumn(0,Qt::AscendingOrder);
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
