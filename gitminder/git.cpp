#include "git.h"
#include <QSettings>
#include <QDateTime>
#include <git2.h>
#include <QDebug>

int getRepoStatus(QString repoPath){
    qDebug() << "Updating repo status...";
    git_repository * repo;

    int openRet = git_repository_open(&repo, repoPath.toStdString().c_str());
    if (openRet < 0){
        return INVALID;
    }

    git_status_list *status;
    git_status_options statusopt = GIT_STATUS_OPTIONS_INIT;
    statusopt.show  = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX | GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

    git_status_list_new(&status, repo, &statusopt);
    int numDirty = git_status_list_entrycount(status);

    git_status_list_free(status);
    git_repository_free(repo);

    if (numDirty>0){
        return DIRTY;
    }
    else{
        return CLEAN;
    }
}


char * getCommitID(QString repoPath){
    git_revwalk * walk;
    git_repository * repo;
    git_oid oid;

    git_repository_open(&repo, repoPath.toStdString().c_str());
    git_revwalk_new(&walk, repo);
    git_revwalk_push_head(walk);
    git_revwalk_next(&oid, walk);
    git_revwalk_free(walk);
    git_repository_free(repo);


    char * commit_id = new char[41]; //Commit ID
    return git_oid_tostr(commit_id, 41, &oid);
}


QString getFileStatus(QString repoPath, QString filePath){
    git_repository * repo;
    unsigned int status_int=0;

    //Get file name alone
    QStringList repoList = repoPath.split("/");
    QStringList fileList = filePath.split("/");
    while(repoList.length() > 0 && fileList.length() > 0 && repoList[0] == fileList[0]){
        repoList.removeFirst();
        fileList.removeFirst();
    }
    QString truncFile;
    for (int i=0; i<fileList.length(); i++){
        truncFile += fileList[i] + "/";
    }
    truncFile=truncFile.remove(truncFile.length()-1, 1);


    //Determine status
    git_repository_open(&repo, repoPath.toStdString().c_str());
    git_status_file(&status_int, repo, truncFile.toStdString().c_str());

    git_repository_free(repo);


    qDebug() << repoPath << truncFile << status_int;
    if (status_int == 0){
        return "Clean";
    }
    else{
        return "Dirty";
    }
}


QString repoStatusToText(int repoStatus){
    if(repoStatus == CLEAN){
        return "Clean";
    }else if(repoStatus == DIRTY){
        return "Dirty";
    }else if(repoStatus == INVALID){
        return "Invalid";
    }
    return "";
}
