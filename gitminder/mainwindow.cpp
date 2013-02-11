#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QFileDialog>
#include <QDebug>

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
    QCoreApplication::setOrganizationName("shookit");
    QCoreApplication::setOrganizationDomain("shookit.com");
    QCoreApplication::setApplicationName("gitminder");


    QSettings settings;
    settings.setValue("commit_reminders", true) ;
    settings.setValue("remote_update_notifications", true) ;

    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ui->listWidget->addItem( new QListWidgetItem(settings.value("directory").toString()));
    }
    settings.endArray();

}

//Destructor
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_4_clicked()
{
    qDeleteAll(ui->listWidget->selectedItems());
}

void MainWindow::on_buttonBox_accepted()
{
    QSettings settings;
    settings.beginWriteArray("watch_directories");
    for (int i=0; i < ui->listWidget->count(); ++i){
        settings.setArrayIndex(i);
        settings.setValue("directory", ui->listWidget->item(i)->text());
    }
    settings.endArray();
    QCoreApplication::instance()->quit();
}

void MainWindow::on_buttonBox_rejected()
{
    QCoreApplication::instance()->quit();
}

void MainWindow::on_pushButton_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.exec();
    for (int i=0; i<dialog.selectedFiles().count(); i++){
        ui->lineEdit->setText(dialog.selectedFiles()[i]);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->listWidget->addItem(ui->lineEdit->text());
    ui->listWidget->sortItems();
    ui->lineEdit->clear();
}
