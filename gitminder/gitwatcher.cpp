#include "gitwatcher.h"
#include <QDir>
#include <QDirIterator>
#include <QSettings>

//Constructor
GitWatcher::GitWatcher(MainWindow * window, QString repoPath){
    this->repoPath = repoPath;
    this->window = window;
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
GitWatcher::~GitWatcher(){
    delete &repoPath;
    delete timer;
    delete &fileWatcher;
    delete &dirWatcher;
}


//Functions
QStringList GitWatcher::recursiveDirectorySearch(QString folder) {
    QStringList fileList;
    QDirIterator dirIt(folder, QDir::NoDotAndDotDot|QDir::Dirs , QDirIterator::Subdirectories );
    while (dirIt.hasNext()) {
        dirIt.next();
        fileList.append(dirIt.filePath());
    }
    return fileList;
} 


void GitWatcher::timeout(){
    this->window->updateWatchDirectoryStatus(this->repoPath);
}


void GitWatcher::directoryChangedSlot(QString)  //QString is the changed directory
{
    timer->start(5000);
}


int gitFlatStatus(QString repoPath, QString changedDirectory){
    git_repository * repo;

    int openRet = git_repository_open(&repo, repoPath.toStdString().c_str());
    if (openRet < 0){
        //Invalid
        git_repository_free(repo);
        return -1;
    }

    QDirIterator dirIt(changedDirectory, QDir::NoDotAndDotDot|QDir::Files);
    unsigned int status_flags;
    while (dirIt.hasNext()) {
        dirIt.next();
        git_status_file(&status_flags, repo, dirIt.filePath().toStdString().c_str());
    }
    git_repository_free(repo);
    return 0;
}


int gitRecursiveStatus(QString repoPath){
    git_repository * repo;

    int openRet = git_repository_open(&repo, repoPath.toStdString().c_str());
    if (openRet < 0){
        //Invalid
        return -1;
    }

    int numDirty = 0;
    git_status_foreach(repo, &directoryChangedCallback, &numDirty);
    git_repository_free(repo);

    if (numDirty>0){
        //Dirty
        return 1;
    }
    else{
        //Clean
        return 0;
    }
}


int directoryChangedCallback(const char *fileName, unsigned int git_status_t, void * numDirty){
    /* A combination of these values will be returned to indicate the status of
    * a file.  Status compares the working directory, the index, and the
    * current HEAD of the repository.  The `GIT_STATUS_INDEX` set of flags
    * represents the status of file in the index relative to the HEAD, and the
    * `GIT_STATUS_WT` set of flags represent the status of the file in the
    * working directory relative to the index.*/

    if(git_status_t!=GIT_STATUS_IGNORED){
        int *num = (int*) numDirty;
        *num = *num + 1;

        if (git_status_t == (1u << 0)){
            qDebug() << "none"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_INDEX_NEW){
            qDebug() << "GIT_STATUS_INDEX_NEW"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_INDEX_MODIFIED){
            qDebug() << "GIT_STATUS_INDEX_MODIFIED"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_INDEX_DELETED){
            qDebug() << "GIT_STATUS_INDEX_DELETED"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_INDEX_RENAMED){
            qDebug() << "GIT_STATUS_INDEX_RENAMED"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_INDEX_TYPECHANGE){
            qDebug() << "GIT_STATUS_INDEX_TYPECHANGE"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_WT_NEW){
            qDebug() << "GIT_STATUS_WT_NEW"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_WT_MODIFIED){
            qDebug() << "GIT_STATUS_WT_MODIFIED"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_WT_DELETED){
            qDebug() << "GIT_STATUS_WT_DELETED"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_WT_TYPECHANGE){
            qDebug() << "GIT_STATUS_WT_TYPECHANGE"  << git_status_t << fileName ;
        }else if (git_status_t == GIT_STATUS_IGNORED){
            qDebug() << "GIT_STATUS_IGNORED"  << git_status_t << fileName ;
        }
    }
    return 0;
}
