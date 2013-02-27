#include "filewatcher.h"

//Constructor
FileWatcher::FileWatcher(QString repoPath){
    this->repoPath = repoPath;
    this->timer = new QTimer(this);
    this->timer->setSingleShot(true);
    git_repository * gitRepo;

    git_repository_open(&gitRepo, repoPath.toStdString().c_str());

    dirWatcher.addPath(repoPath);
    if (recursiveDirectorySearch(repoPath).length() > 0){
        dirWatcher.addPaths(recursiveDirectorySearch(repoPath));
    }

    QObject::connect(&dirWatcher,SIGNAL(directoryChanged(QString)), this, SLOT(directoryChangedSlot(QString)));
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    git_repository_free(gitRepo);
}


//Destructor
FileWatcher::~FileWatcher(){
    delete this->timer;
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


void FileWatcher::timeout(){
    emit fileChangedSignal(this->repoPath);
}


void FileWatcher::directoryChangedSlot(QString)  //QString is the changed directory
{
    timer->start(5000);
}
