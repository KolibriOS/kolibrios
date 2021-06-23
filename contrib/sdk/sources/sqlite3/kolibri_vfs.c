#if !defined(SQLITE_TEST) || _KOLIBRI

#include "sqlite3.h"

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/param.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

/*
** Size of the write buffer used by journal files in bytes.
*/
#ifndef SQLITE_DEMOVFS_BUFFERSZ
# define SQLITE_DEMOVFS_BUFFERSZ 8192
#endif

/*
** The maximum pathname length supported by this VFS.
*/
#define MAXPATHNAME 512

sqlite3_vfs *sqlite3_kosvfs(void);

/*
** When using this VFS, the sqlite3_file* handles that SQLite uses are
** actually pointers to instances of type KosFile.
*/
typedef struct KosFile KosFile;
struct KosFile {
  sqlite3_file base;              /* Base class. Must be first. */
  int fd;                         /* File descriptor */

  char *aBuffer;                  /* Pointer to malloc'd buffer */
  int nBuffer;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 iBufferOfst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (KosFile.aBuffer), ignore it.
*/
static int kosDirectWrite(
  KosFile *p,                    /* File handle */
  const void *zBuf,               /* Buffer containing data to write */
  int iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  if( ofst!=iOfst ){
    return SQLITE_IOERR_WRITE;
  }

  nWrite = write(p->fd, zBuf, iAmt);
  if( nWrite!=iAmt ){
    return SQLITE_IOERR_WRITE;
  }

  return SQLITE_OK;
}

/*
** Flush the contents of the KosFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/

int sqlite3_os_init(){
   return sqlite3_vfs_register(sqlite3_kosvfs(), 0);
}

static int kosFlushBuffer(KosFile *p){
  int rc = SQLITE_OK;
  if( p->nBuffer ){
    rc = kosDirectWrite(p, p->aBuffer, p->nBuffer, p->iBufferOfst);
    p->nBuffer = 0;
  }
  return rc;
}

/*
** Close a file.
*/
static int kosClose(sqlite3_file *pFile){
  int rc;
  KosFile *p = (KosFile*)pFile;
  rc = kosFlushBuffer(p);
  sqlite3_free(p->aBuffer);
  close(p->fd);
  return rc;
}

/*
** Read data from a file.
*/
static int kosRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  int iAmt, 
  sqlite_int64 iOfst
){
  KosFile *p = (KosFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  int nRead;                      /* Return value from read() */
  int rc;                         /* Return code from kosFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = kosFlushBuffer(p);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  if( ofst!=iOfst ){
    return SQLITE_IOERR_READ;
  }
  nRead = read(p->fd, zBuf, iAmt);

  if( nRead==iAmt ){
    return SQLITE_OK;
  }else if( nRead>=0 ){
    if( nRead<iAmt ){
      memset(&((char*)zBuf)[nRead], 0, iAmt-nRead);
    }
    return SQLITE_IOERR_SHORT_READ;
  }

  return SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
static int kosWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  int iAmt, 
  sqlite_int64 iOfst
){
  KosFile *p = (KosFile*)pFile;
  
  if( p->aBuffer ){
    char *z = (char *)zBuf;       /* Pointer to remaining data to write */
    int n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    while( n>0 ){
      int nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      if( p->nBuffer==SQLITE_DEMOVFS_BUFFERSZ || p->iBufferOfst+p->nBuffer!=i ){
        int rc = kosFlushBuffer(p);
        if( rc!=SQLITE_OK ){
          return rc;
        }
      }
      assert( p->nBuffer==0 || p->iBufferOfst+p->nBuffer==i );
      p->iBufferOfst = i - p->nBuffer;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->nBuffer;
      if( nCopy>n ){
        nCopy = n;
      }
      memcpy(&p->aBuffer[p->nBuffer], z, nCopy);
      p->nBuffer += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }else{
    return kosDirectWrite(p, zBuf, iAmt, iOfst);
  }

  return SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
static int kosTruncate(sqlite3_file *pFile, sqlite_int64 size){
#if 0
  if( ftruncate(((KosFile *)pFile)->fd, size) ) return SQLITE_IOERR_TRUNCATE;
#endif
  return SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
static int kosSync(sqlite3_file *pFile, int flags){
  KosFile *p = (KosFile*)pFile;
  int rc;

  rc = kosFlushBuffer(p);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  rc = fsync(p->fd);
  return (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
static int kosFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  KosFile *p = (KosFile*)pFile;
  int rc;                         /* Return code from fstat() call */
  struct stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** kosRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = kosFlushBuffer(p);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  rc = fstat(p->fd, &sStat);
  if( rc!=0 ) return SQLITE_IOERR_FSTAT;
  *pSize = sStat.st_size;
  return SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
static int kosLock(sqlite3_file *pFile, int eLock){
  return SQLITE_OK;
}
static int kosUnlock(sqlite3_file *pFile, int eLock){
  return SQLITE_OK;
}
static int kosCheckReservedLock(sqlite3_file *pFile, int *pResOut){
  *pResOut = 0;
  return SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
static int kosFileControl(sqlite3_file *pFile, int op, void *pArg){
  return SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
static int kosSectorSize(sqlite3_file *pFile){
  return 0;
}
static int kosDeviceCharacteristics(sqlite3_file *pFile){
  return 0;
}

/*
** Open a file handle.
*/
static int kosOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  const char *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to KosFile struct to populate */
  int flags,                      /* Input SQLITE_OPEN_XXX flags */
  int *pOutFlags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  static const sqlite3_io_methods kosio = {
    1,                            /* iVersion */
    kosClose,                    /* xClose */
    kosRead,                     /* xRead */
    kosWrite,                    /* xWrite */
    kosTruncate,                 /* xTruncate */
    kosSync,                     /* xSync */
    kosFileSize,                 /* xFileSize */
    kosLock,                     /* xLock */
    kosUnlock,                   /* xUnlock */
    kosCheckReservedLock,        /* xCheckReservedLock */
    kosFileControl,              /* xFileControl */
    kosSectorSize,               /* xSectorSize */
    kosDeviceCharacteristics     /* xDeviceCharacteristics */
  };

  KosFile *p = (KosFile*)pFile; /* Populate this structure */
  int oflags = 0;                 /* flags to pass to open() call */
  char *aBuf = 0;

  if( zName==0 ){
    return SQLITE_IOERR;
  }

  if( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (char *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    if( !aBuf ){
      return SQLITE_NOMEM;
    }
  }

  if( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  if( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  if( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  if( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, sizeof(KosFile));
  p->fd = open(zName, oflags, 0600);
  if( p->fd<0 ){
    sqlite3_free(aBuf);
    return SQLITE_CANTOPEN;
  }
  p->aBuffer = aBuf;

  if( pOutFlags ){
    *pOutFlags = flags;
  }
  p->base.pMethods = &kosio;
  return SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
static int kosDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync){
  int rc;                         /* Return code */

  rc = unlink(zPath);
  if( rc!=0 && errno==ENOENT ) return SQLITE_OK;

  if( rc==0 && dirSync ){
    int dfd;                      /* File descriptor open on directory */
    int i;                        /* Iterator variable */
    char zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", zPath);
    zDir[MAXPATHNAME] = '\0';
    for(i=strlen(zDir); i>1 && zDir[i]!='/'; i++);
    zDir[i] = '\0';

    /* Open a file-descriptor on the directory. Sync. Close. */
    dfd = open(zDir, O_RDONLY, 0);
    if( dfd<0 ){
      rc = -1;
    }else{
      rc = fsync(dfd);
      close(dfd);
    }
  }
  return (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

#ifndef F_OK
# define F_OK 0
#endif
#ifndef R_OK
# define R_OK 4
#endif
#ifndef W_OK
# define W_OK 2
#endif

/*
** Query the file-system to see if the named file exists, is readable or
** is both readable and writable.
*/
static int kosAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  int flags, 
  int *pResOut
){
  int rc;                         /* access() return code */
  int eAccess = F_OK;             /* Second argument to access() */

  assert( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  if( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  if( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(zPath, eAccess);
  *pResOut = (rc==0);
  return SQLITE_OK;
}

/*
** Argument zPath points to a nul-terminated string containing a file path.
** If zPath is an absolute path, then it is copied as is into the output 
** buffer. Otherwise, if it is a relative path, then the equivalent full
** path is written to the output buffer.
**
** This function assumes that paths are UNIX style. Specifically, that:
**
**   1. Path components are separated by a '/'. and 
**   2. Full paths begin with a '/' character.
*/
static int kosFullPathname(
  sqlite3_vfs *pVfs,              /* VFS */
  const char *zPath,              /* Input path (possibly a relative path) */
  int nPathOut,                   /* Size of output buffer in bytes */
  char *zPathOut                  /* Pointer to output buffer */
){
  char zDir[MAXPATHNAME+1];
  if( zPath[0]=='/' ){
    zDir[0] = '\0';
  }else{
    if( getcwd(zDir, sizeof(zDir))==0 ) return SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(nPathOut, zPathOut, "%s/%s", zDir, zPath);
  zPathOut[nPathOut-1] = '\0';

  return SQLITE_OK;
}

/*
** The following four VFS methods:
**
**   xDlOpen
**   xDlError
**   xDlSym
**   xDlClose
**
** are supposed to implement the functionality needed by SQLite to load
** extensions compiled as shared objects. This simple VFS does not support
** this functionality, so the following functions are no-ops.
*/
static void *kosDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  return 0;
}
static void kosDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg){
  sqlite3_snprintf(nByte, zErrMsg, "Loadable extensions are not supported");
  zErrMsg[nByte-1] = '\0';
}
static void (*kosDlSym(sqlite3_vfs *pVfs, void *pH, const char *z))(void){
  return 0;
}
static void kosDlClose(sqlite3_vfs *pVfs, void *pHandle){
  return;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
static int kosRandomness(sqlite3_vfs *pVfs, int nByte, char *zByte){
  return SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
static int kosSleep(sqlite3_vfs *pVfs, int nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  return nMicro;
}

/*
** Set *pTime to the current UTC time expressed as a Julian day. Return
** SQLITE_OK if successful, or an error code otherwise.
**
**   http://en.wikipedia.org/wiki/Julian_day
**
** This implementation is not very good. The current time is rounded to
** an integer number of seconds. Also, assuming time_t is a signed 32-bit 
** value, it will stop working some time in the year 2038 AD (the so-called
** "year 2038" problem that afflicts systems that store time this way). 
*/
static int kosCurrentTime(sqlite3_vfs *pVfs, double *pTime){
  time_t t = time(0);
  *pTime = t/86400.0 + 2440587.5; 
  return SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_kosvfs(), 0);
*/
sqlite3_vfs *sqlite3_kosvfs(void){
  static sqlite3_vfs kosvfs = {
    1,                            /* iVersion */
    sizeof(KosFile),             /* szOsFile */
    MAXPATHNAME,                  /* mxPathname */
    0,                            /* pNext */
    "kos",                       /* zName */
    0,                            /* pAppData */
    kosOpen,                     /* xOpen */
    kosDelete,                   /* xDelete */
    kosAccess,                   /* xAccess */
    kosFullPathname,             /* xFullPathname */
    kosDlOpen,                   /* xDlOpen */
    kosDlError,                  /* xDlError */
    kosDlSym,                    /* xDlSym */
    kosDlClose,                  /* xDlClose */
    kosRandomness,               /* xRandomness */
    kosSleep,                    /* xSleep */
    kosCurrentTime,              /* xCurrentTime */
  };
  return &kosvfs;
}

SQLITE_API int sqlite3_os_end(void){
  return SQLITE_OK;
}

void _ksys_sleep(unsigned time)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(5), "b"(time)
    :"memory");
};

unsigned sleep(unsigned time){
  _ksys_sleep(time*100);
  return 0;
}

int usleep(useconds_t usec){ 
   _ksys_sleep(usec);
   return 0;
}

int fsync(int fd){
  return 0;
}


#endif /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#ifdef SQLITE_TEST

#if defined(INCLUDE_SQLITE_TCL_H)
#  include "sqlite_tcl.h"
#else
#  include "tcl.h"
#  ifndef SQLITE_TCLAPI
#    define SQLITE_TCLAPI
#  endif
#endif

#if SQLITE_OS_UNIX
static int SQLITE_TCLAPI register_kosvfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  int objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_register(sqlite3_kosvfs(), 1);
  return TCL_OK;
}
static int SQLITE_TCLAPI unregister_kosvfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  int objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_kosvfs());
  return TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
int Sqlitetest_kosvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_kosvfs", register_kosvfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_kosvfs", unregister_kosvfs, 0, 0);
  return TCL_OK;
}

#else
int Sqlitetest_kosvfs_Init(Tcl_Interp *interp){ return TCL_OK; }
#endif

#endif /* SQLITE_TEST */
