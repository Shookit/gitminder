#ifndef GIT_H
#define GIT_H

#include<QString>

enum {INVALID, DIRTY, CLEAN};

int getRepoStatus(QString repoPath);
QString repoStatusToText(int repoStatus);
char * getCommitID(QString repoPath);
QString getFileStatus(QString repoPath, QString relFilePath);

#endif // GIT_H
