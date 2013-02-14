#include "gitwatcher.h"
#include <QDir>
#include <QDirIterator>
#include <QSettings>

//Constructor
GitWatcher::GitWatcher(MainWindow * window, QString repoPath){
    this->repoPath = repoPath;
    this->window = window;
    git_repository_open(&gitRepo, repoPath.toStdString().c_str());
    fileWatcher.addPaths(recursiveDirectorySearch(repoPath));
    QObject::connect(&fileWatcher,SIGNAL(fileChanged(QString)), this, SLOT(directoryChangedSlot(QString)));
    QObject::connect(&fileWatcher,SIGNAL(directoryChanged(QString)), this, SLOT(directoryChangedSlot(QString)));
}

//Destructor
GitWatcher::~GitWatcher(){

}

//Functions
QStringList GitWatcher::recursiveFileSearch(QString folder) {
    QStringList fileList;
    QDirIterator dirIt(folder,QDirIterator::Subdirectories);
    while (dirIt.hasNext()) {
        dirIt.next();
        if (QFileInfo(dirIt.filePath()).isFile())
            fileList.append(dirIt.filePath());
    }
    return fileList;
}

QStringList GitWatcher::recursiveDirectorySearch(QString folder) {
    QStringList fileList;
    QDirIterator dirIt(folder,QDirIterator::Subdirectories);
    while (dirIt.hasNext()) {
        dirIt.next();
        fileList.append(dirIt.filePath());
    }
    return fileList;
}


void GitWatcher::directoryChangedSlot(QString changedFile)
{
    window->updateWatchDirectoriesUI();
}


int gitStatus(QString repoPath){
    git_repository * repo;
    int openRet = git_repository_open(&repo, repoPath.toStdString().c_str());
    if (openRet < 0){
        //Invalid
        return -1;
    }

    int numDirty = 0;
    git_status_foreach(repo, &directoryChangedCallback, &numDirty);
    if (numDirty>0){
        //Dirty
        return 1;
    }
    else{
        //Clean
        return 0;
    }
}


int directoryChangedCallback(const char *, unsigned int, void * numDirty){
    int *num = (int*) numDirty;
    *num = *num + 1;
    return 0;
}
