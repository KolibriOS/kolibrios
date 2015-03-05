
#ifndef __filescanner_h__
#define __filescanner_h__

#ifdef WIN32
#include <io.h>
#include <direct.h>
#define DIRSEP "\\"
#endif

#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_DIRENT_H
#define _GCC_BUILD_
#include <dirent.h>
#define DIRSEP "/"
#endif //HAVE_DIRENT_H

#include <string.h>
#include "lispassert.h"

class CFileNode
{
public:
  inline CFileNode() : iIsDir(0),iName(""),iDir("") {};
  inline void Set(int aIsDir,char* aName)
  {
    iIsDir = aIsDir;
    iName  = aName;
    strcpy(iFullName,iDir);
    if (strlen(iDir))
      strcat(iFullName,DIRSEP);
    strcat(iFullName,aName);
  }
  inline int IsDirectory() {return iIsDir;};
  inline char* FullName() {return iFullName;};
  inline void SetRoot(char* aDir) {iDir=aDir;};
private:
  int iIsDir;
  char *iName;
  char iFullName[500];
  char* iDir;
};

class CFileScanner
{
public:
  CFileScanner();
  ~CFileScanner();
  CFileNode* First(char* base,char* dir);
  CFileNode* Next();
private:
  CFileScanner(const CFileScanner& aFileScanner)
    : iCurNode()
#ifdef _GCC_BUILD_
  ,dp(NULL),entry(NULL),statbuf()
#endif
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  }
  CFileScanner& operator=(const CFileScanner& aFileScanner)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
private:
  CFileNode iCurNode;

  char fulldir[500];

#ifdef _GCC_BUILD_
  DIR *dp;
  struct dirent* entry;
  struct stat statbuf;
#endif

#ifdef WIN32
  long handle;
  struct _finddata_t info;
  int first;
#endif
};

#endif // __scanfiles_h__
