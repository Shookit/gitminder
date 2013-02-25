#include "git.h"

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

QMap<QString, QString> getRepoSettings(QString repoPath){
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


void updateRepoSettings(QMap<QString, QString> repoSettings){
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


void updateAllWatchDirectoryData(){
    QSettings settings;
    int size = settings.beginReadArray("watch_directories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        updateWatchDirectoryData(settings.value("directory").toString());
    }
    settings.endArray();
}


void updateWatchDirectoryData(QString repoPath){
    QMap<QString,QString> repoSettings;
    repoSettings["directory"] = repoPath;

    int repoStatus = gitRecursiveStatus(repoPath);

    if (repoStatus == -1){
        //If invalid
        repoSettings["status"] = "invalid";
    }
    else{
        //If valid
        git_revwalk * walk;
        git_repository * repo;
        git_oid oid;

        git_repository_open(&repo, repoPath.toStdString().c_str());
        git_revwalk_new(&walk, repo);
        git_revwalk_push_head(walk);
        git_revwalk_next(&oid, walk);
        git_revwalk_free(walk);

        char * commit_id = new char[41]; //Commit ID
        git_oid_tostr(commit_id, 41, &oid);

        if(repoStatus == 0){
            //If clean
            repoSettings["commit"] = commit_id;
            repoSettings["status"] = "clean";
        }else if(repoStatus == 1){
            //If dirty
            repoSettings["status"] = "dirty";
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
        delete[] commit_id;
        git_repository_free(repo);
    }
    updateRepoSettings(repoSettings);
}

QString repoStatusToText(int repoStatus){
    if(repoStatus == 0){
        return "Clean";
    }else if(repoStatus == 1){
        return "Dirty";
    }else if(repoStatus == -1){
        return "Invalid";
    }
    return "";
}
