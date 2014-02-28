#include "filewatcher.h"
#include <QDirIterator>

//Constructor
FileWatcher::FileWatcher(QString repoPath){
    this->repoPath = repoPath;

    this->periodic_file_checker = new QTimer(this);
    this->periodic_file_checker->setSingleShot(true);

    dirWatcher.addPath(repoPath);
    if (recursiveDirectorySearch(repoPath).length() > 0){
        dirWatcher.addPaths(recursiveDirectorySearch(repoPath));
    }

    QObject::connect(&dirWatcher,SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));
    QObject::connect(periodic_file_checker, SIGNAL(timeout()), this, SLOT(alternative_file_checker_timeout()));
    periodic_file_checker->start(10*60*1000);
}


//Functions
QStringList FileWatcher::recursiveDirectorySearch(QString folder) {
    QStringList fileList;
    QDirIterator dirIt(folder, QDir::NoDotAndDotDot|QDir::Files, QDirIterator::Subdirectories );
    while (dirIt.hasNext()) {
        dirIt.next();
        fileList.append(dirIt.filePath());
    }
    return fileList;
}



//Slots
void FileWatcher::fileChangedSlot(QString changedFile){
    //Called whenever the QFileSystemWatcher dirWatcher detects a changed file
    emit fileChangedSignal(this->repoPath, changedFile);
    periodic_file_checker->start(10*60*1000);
}


void FileWatcher::alternative_file_checker_timeout(){
    //If no files have been changed for 10 minutes, just run a check to make sure everything's in order (sometimes the file watcher misses changes if the disk write is buffered)
    periodic_file_checker->start(10*60*1000);
    emit periodicSignal(this->repoPath);
}
