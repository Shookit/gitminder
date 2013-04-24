#include "filewatcher.h"

//Constructor
FileWatcher::FileWatcher(QString repoPath){
    this->repoPath = repoPath;
    git_repository * gitRepo;

    this->file_changed_delay_timer = new QTimer(this);
    this->file_changed_delay_timer->setSingleShot(true);

    this->alternative_file_checker = new QTimer(this);
    this->alternative_file_checker->setSingleShot(true);

    git_repository_open(&gitRepo, repoPath.toStdString().c_str());

    dirWatcher.addPath(repoPath);
    if (recursiveDirectorySearch(repoPath).length() > 0){
        dirWatcher.addPaths(recursiveDirectorySearch(repoPath));
    }

    QObject::connect(&dirWatcher,SIGNAL(directoryChanged(QString)), this, SLOT(directoryChangedSlot(QString)));
    QObject::connect(file_changed_delay_timer, SIGNAL(timeout()), this, SLOT(file_changed_delay_timer_timeout()));
    QObject::connect(alternative_file_checker, SIGNAL(timeout()), this, SLOT(alternative_file_checker_timeout()));
    alternative_file_checker->start(10*60*1000);

    git_repository_free(gitRepo);
}


//Destructor
FileWatcher::~FileWatcher(){
    delete this->file_changed_delay_timer;
    delete this->alternative_file_checker;
}


//Functions
QStringList FileWatcher::recursiveDirectorySearch(QString folder) {
    QStringList fileList;
    QDirIterator dirIt(folder, QDir::NoDotAndDotDot|QDir::Dirs , QDirIterator::Subdirectories );
    while (dirIt.hasNext()) {
        dirIt.next();
        fileList.append(dirIt.filePath());
    }
    return fileList;
}

void FileWatcher::directoryChangedSlot(QString)  //QString is the changed directory
{
    //Called whenever the QFileSystemWatcher dirWatcher detects a changed file
    file_changed_delay_timer->start(5000);
    alternative_file_checker->start(10*60*1000);
}
void FileWatcher::file_changed_delay_timer_timeout(){
    //Only emit the signal after 5 seconds so you don't have a bunch of unnecessary calls
    emit fileChangedSignal(this->repoPath);
}
void FileWatcher::alternative_file_checker_timeout(){
    //If no files have been changed for 10 minutes, just run a check to make sure everything's in order (sometimes the file watcher misses changes if the disk write is buffered)
    alternative_file_checker->start(10*60*1000);
    emit fileChangedSignal(this->repoPath);
}
