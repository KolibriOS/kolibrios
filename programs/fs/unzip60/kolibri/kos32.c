/*
Kolibri OS port for gcc 5.4

Started by Siemargl @Nov 2016
Borrowed code parts from other unzip ports

howto make:
go in unzip60 directory (below this file) and
>make -f kolibri\makefile.gcc

Contains:
    version()
    mapattr()
    mapname()
    checkdir()
    close_outfile()
    get_extattribs()
    do_wild()
    set_direc_attribs()
    defer_dir_attribs()

todo
	datetime restore for dirs. buf in unzip - crash when SET_DIR_ATTRIB
fixed partial
	-d dir error (only in unicode).  Use -ddir or -d dir/
	russian filenames in arhives. Now works cp866 version, unicode need to fix @kos32.c:470 (GETPATH)
*/

#define FATTR   FS_HIDDEN+FS_SYSTEM+FS_SUBDIR



#define UNZIP_INTERNAL
#include "unzip.h"

// Siemargl fork of Kolibri system API
#include "kos32sys1.h"

static ZCONST char CannotSetItemTimestamps[] =
  "warning:  cannot set modif./access times for %s\n          %s\n";
static ZCONST char CannotGetTimestamps[] =
  " (warning) cannot get fileinfo for %s\n";


/********************************************************************************************************************/
/***  Function version()  */
/********************************************************************************************************************/

void version(__G)
    __GDEF
{
    sprintf((char *)slide, LoadFarString(CompiledWith),
#if defined(__TINYC__)
      "TinyC", "",
#elif defined(__GNUC__)
      "GNU C ", __VERSION__,
#else
      "(unknown compiler) ","",
#endif
      "KolibriOS ",

#ifdef __POWERPC__
      "(PowerPC)",
#else
# ifdef __INTEL__
      "(x86)",
# else
      "(unknown)",   /* someday we may have other architectures... */
# endif
#endif

#ifdef __DATE__
      " on ", __DATE__
#else
      "", ""
#endif
    );

    (*G.message)((zvoid *)&G, slide, (ulg)strlen((char *)slide), 0);

} /* end function version() */


/********************************************************************************************************************/
/*** Function mapattr() */
/********************************************************************************************************************/

/* Identical to MS-DOS, OS/2 versions.  However, NT has a lot of extra
 * permission stuff, so this function should probably be extended in the
 * future. */

int mapattr(__G)
    __GDEF
{
    /* set archive bit for file entries (file is not backed up): */
    G.pInfo->file_attr = ((unsigned)G.crec.external_file_attributes |
      (G.crec.external_file_attributes & FS_SUBDIR ?
       0 : FS_ARCHIV)) & 0xff;
    return 0;

} /* end function mapattr() */


/********************************************************************************************************************/
/***  Function mapname()  */
/********************************************************************************************************************/

int mapname(__G__ renamed)
    __GDEF
    int renamed;
/*
 * returns:
 *  MPN_OK          - no problem detected
 *  MPN_INF_TRUNC   - caution (truncated filename)
 *  MPN_INF_SKIP    - info "skip entry" (dir doesn't exist)
 *  MPN_ERR_SKIP    - error -> skip entry
 *  MPN_ERR_TOOLONG - error -> path is too long
 *  MPN_NOMEM       - error (memory allocation failed) -> skip entry
 *  [also MPN_VOL_LABEL, MPN_CREATED_DIR]
 */
{
    char pathcomp[FILNAMSIZ];      /* path-component buffer */
    char *pp, *cp=(char *)NULL;    /* character pointers */
    char *lastsemi=(char *)NULL;   /* pointer to last semi-colon in pathcomp */
#ifdef ACORN_FTYPE_NFS
    char *lastcomma=(char *)NULL;  /* pointer to last comma in pathcomp */
    RO_extra_block *ef_spark;      /* pointer Acorn FTYPE ef block */
#endif
    int killed_ddot = FALSE;       /* is set when skipping "../" pathcomp */
    int error = MPN_OK;
    register unsigned workch;      /* hold the character being tested */


/*---------------------------------------------------------------------------
    Initialize various pointers and counters and stuff.
  ---------------------------------------------------------------------------*/

    if (G.pInfo->vollabel)
        return MPN_VOL_LABEL;   /* can't set disk volume labels in Unix */

    /* can create path as long as not just freshening, or if user told us */
    G.create_dirs = (!uO.fflag || renamed);

    G.created_dir = FALSE;      /* not yet */

    /* user gave full pathname:  don't prepend rootpath */
    G.renamed_fullpath = (renamed && (*G.filename == '/'));

    if (checkdir(__G__ (char *)NULL, INIT) == MPN_NOMEM)
        return MPN_NOMEM;       /* initialize path buffer, unless no memory */

    *pathcomp = '\0';           /* initialize translation buffer */
    pp = pathcomp;              /* point to translation buffer */
    if (uO.jflag)               /* junking directories */
        cp = (char *)strrchr(G.filename, '/');
    if (cp == (char *)NULL)     /* no '/' or not junking dirs */
        cp = G.filename;        /* point to internal zipfile-member pathname */
    else
        ++cp;                   /* point to start of last component of path */

    Trace((stderr, "mapname start[%s]\n", cp));

/*---------------------------------------------------------------------------
    Begin main loop through characters in filename.
  ---------------------------------------------------------------------------*/

    while ((workch = (uch)*cp++) != 0) {

        switch (workch) {
            case '/':             /* can assume -j flag not given */
                *pp = '\0';
                if (strcmp(pathcomp, ".") == 0) {
                    /* don't bother appending "./" to the path */
                    *pathcomp = '\0';
                } else if (!uO.ddotflag && strcmp(pathcomp, "..") == 0) {
                    /* "../" dir traversal detected, skip over it */
                    *pathcomp = '\0';
                    killed_ddot = TRUE;     /* set "show message" flag */
                }
                /* when path component is not empty, append it now */
                if (*pathcomp != '\0' &&
                    ((error = checkdir(__G__ pathcomp, APPEND_DIR))
                     & MPN_MASK) > MPN_INF_TRUNC)
                    return error;
                pp = pathcomp;    /* reset conversion buffer for next piece */
                lastsemi = (char *)NULL; /* leave direct. semi-colons alone */
                break;

#ifdef KOS32   /* Cygwin runs on Win32, apply FAT/NTFS filename rules */
            case ':':         /* drive spec not stored, so no colon allowed */
            case '\\':        /* '\\' may come as normal filename char (not */
            case '<':         /*  dir sep char!) from unix-like file system */
            case '>':         /* no redirection symbols allowed either */
            case '|':         /* no pipe signs allowed */
            case '"':         /* no double quotes allowed */
            case '?':         /* no wildcards allowed */
            case '*':
                *pp++ = '_';  /* these rules apply equally to FAT and NTFS */
                break;
#endif

            case ';':             /* VMS version (or DEC-20 attrib?) */
                lastsemi = pp;
                *pp++ = ';';      /* keep for now; remove VMS ";##" */
                break;            /*  later, if requested */

#ifdef ACORN_FTYPE_NFS
            case ',':             /* NFS filetype extension */
                lastcomma = pp;
                *pp++ = ',';      /* keep for now; may need to remove */
                break;            /*  later, if requested */
#endif

#ifdef KOS32
            case ' ':             /* change spaces to underscore under */
                *pp++ = '_';      /*  MTS; leave as spaces under Unix */
                break;
#endif

            default:
                /* disable control character filter when requested,
                 * else allow 8-bit characters (e.g. UTF-8) in filenames:
                 */
                if ((isprint(workch) || (128 <= workch && workch <= 254)))
                    *pp++ = (char)workch;
        } /* end switch */

    } /* end while loop */

    /* Show warning when stripping insecure "parent dir" path components */
    if (killed_ddot && QCOND2) {
        Info(slide, 0, ((char *)slide,
          "warning:  skipped \"../\" path component(s) in %s\n",
          FnFilter1(G.filename)));
        if (!(error & ~MPN_MASK))
            error = (error & MPN_MASK) | PK_WARN;
    }

/*---------------------------------------------------------------------------
    Report if directory was created (and no file to create:  filename ended
    in '/'), check name to be sure it exists, and combine path and name be-
    fore exiting.
  ---------------------------------------------------------------------------*/

    if (G.filename[strlen(G.filename) - 1] == '/') {
        checkdir(__G__ G.filename, GETPATH);
        if (G.created_dir) {
            if (QCOND2) {
                Info(slide, 0, ((char *)slide, "   creating: %s\n",
                  FnFilter1(G.filename)));
            }
#ifndef NO_CHMOD
            /* Filter out security-relevant attributes bits. */
            G.pInfo->file_attr = filtattr(__G__ G.pInfo->file_attr);
            /* When extracting non-UNIX directories or when extracting
             * without UID/GID restoration or SGID preservation, any
             * SGID flag inherited from the parent directory should be
             * maintained to allow files extracted into this new folder
             * to inherit the GID setting from the parent directory.
             */
            if (G.pInfo->hostnum != UNIX_ || !(uO.X_flag || uO.K_flag)) {
                /* preserve SGID bit when inherited from parent dir */
                if (!SSTAT(G.filename, &G.statbuf)) {
                    G.pInfo->file_attr |= G.statbuf.st_mode & S_ISGID;
                } else {
                    perror("Could not read directory attributes");
                }
            }

            /* set approx. dir perms (make sure can still read/write in dir) */
            if (chmod(G.filename, G.pInfo->file_attr | 0700))
                perror("chmod (directory attributes) error");
#endif
            /* set dir time (note trailing '/') */
            return (error & ~MPN_MASK) | MPN_CREATED_DIR;
        }
        /* dir existed already; don't look for data to extract */
        return (error & ~MPN_MASK) | MPN_INF_SKIP;
    }

    *pp = '\0';                   /* done with pathcomp:  terminate it */

    /* if not saving them, remove VMS version numbers (appended ";###") */
    if (!uO.V_flag && lastsemi) {
        pp = lastsemi + 1;
        while (isdigit((uch)(*pp)))
            ++pp;
        if (*pp == '\0')          /* only digits between ';' and end:  nuke */
            *lastsemi = '\0';
    }

    /* On UNIX (and compatible systems), "." and ".." are reserved for
     * directory navigation and cannot be used as regular file names.
     * These reserved one-dot and two-dot names are mapped to "_" and "__".
     */
    if (strcmp(pathcomp, ".") == 0)
        *pathcomp = '_';
    else if (strcmp(pathcomp, "..") == 0)
        strcpy(pathcomp, "__");

#ifdef ACORN_FTYPE_NFS
    /* translate Acorn filetype information if asked to do so */
    if (uO.acorn_nfs_ext &&
        (ef_spark = (RO_extra_block *)
                    getRISCOSexfield(G.extra_field, G.lrec.extra_field_length))
        != (RO_extra_block *)NULL)
    {
        /* file *must* have a RISC OS extra field */
        long ft = (long)makelong(ef_spark->loadaddr);
        /*32-bit*/
        if (lastcomma) {
            pp = lastcomma + 1;
            while (isxdigit((uch)(*pp))) ++pp;
            if (pp == lastcomma+4 && *pp == '\0') *lastcomma='\0'; /* nuke */
        }
        if ((ft & 1<<31)==0) ft=0x000FFD00;
        sprintf(pathcomp+strlen(pathcomp), ",%03x", (int)(ft>>8) & 0xFFF);
    }
#endif /* ACORN_FTYPE_NFS */

    if (*pathcomp == '\0') {
        Info(slide, 1, ((char *)slide, "mapname:  conversion of %s failed\n",
          FnFilter1(G.filename)));
        return (error & ~MPN_MASK) | MPN_ERR_SKIP;
    }

    checkdir(__G__ pathcomp, APPEND_NAME);  /* returns 1 if truncated: care? */
    checkdir(__G__ G.filename, GETPATH);

    Trace((stderr, "mapname end[%s]\n", pathcomp));


    return error;

} /* end function mapname() */




#if 0  /*========== NOTES ==========*/

  extract-to dir:      a:path/
  buildpath:           path1/path2/ ...   (NULL-terminated)
  pathcomp:                filename

  mapname():
    loop over chars in zipfile member name
      checkdir(path component, COMPONENT | CREATEDIR) --> map as required?
        (d:/tmp/unzip/)                    (disk:[tmp.unzip.)
        (d:/tmp/unzip/jj/)                 (disk:[tmp.unzip.jj.)
        (d:/tmp/unzip/jj/temp/)            (disk:[tmp.unzip.jj.temp.)
    finally add filename itself and check for existence? (could use with rename)
        (d:/tmp/unzip/jj/temp/msg.outdir)  (disk:[tmp.unzip.jj.temp]msg.outdir)
    checkdir(name, GETPATH)     -->  copy path to name and free space

#endif /* 0 */




/********************************************************************************************************************/
/*** Function checkdir() */
/********************************************************************************************************************/


int checkdir(__G__ pathcomp, flag)
    __GDEF
    char *pathcomp;
    int flag;
/*
 * returns:
 *  MPN_OK          - no problem detected
 *  MPN_INF_TRUNC   - (on APPEND_NAME) truncated filename
 *  MPN_INF_SKIP    - path doesn't exist, not allowed to create
 *  MPN_ERR_SKIP    - path doesn't exist, tried to create and failed; or path
 *                    exists and is not a directory, but is supposed to be
 *  MPN_ERR_TOOLONG - path is too long
 *  MPN_NOMEM       - can't allocate memory for filename buffers
 */
{
 /* static int rootlen = 0; */  /* length of rootpath */
 /* static char *rootpath;  */  /* user's "extract-to" directory */
 /* static char *buildpath; */  /* full path (so far) to extracted file */
 /* static char *end;       */  /* pointer to end of buildpath ('\0') */

#   define FN_MASK   7
#   define FUNCTION  (flag & FN_MASK)



/*---------------------------------------------------------------------------
    APPEND_DIR:  append the path component to the path being built and check
    for its existence.  If doesn't exist and we are creating directories, do
    so for this one; else signal success or error as appropriate.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_DIR) {
        int too_long = FALSE;
#ifdef SHORT_NAMES
        char *old_end = end;
#endif

        Trace((stderr, "appending dir segment [%s]\n", FnFilter1(pathcomp)));
        while ((*G.end = *pathcomp++) != '\0')
            ++G.end;
#ifdef SHORT_NAMES   /* path components restricted to 14 chars, typically */
        if ((G.end-old_end) > FILENAME_MAX)  /* GRR:  proper constant? */
            *(G.end = old_end + FILENAME_MAX) = '\0';
#endif

        /* GRR:  could do better check, see if overrunning buffer as we go:
         * check end-buildpath after each append, set warning variable if
         * within 20 of FILNAMSIZ; then if var set, do careful check when
         * appending.  Clear variable when begin new path. */

        /* next check: need to append '/', at least one-char name, '\0' */
        if ((G.end-G.buildpath) > FILNAMSIZ-3)
            too_long = TRUE;                    /* check if extracting dir? */
        if (SSTAT(G.buildpath, &G.statbuf)) {   /* path doesn't exist */
            if (!G.create_dirs) { /* told not to create (freshening) */
                free(G.buildpath);
                return MPN_INF_SKIP;    /* path doesn't exist: nothing to do */
            }
            if (too_long) {
                Info(slide, 1, ((char *)slide,
                  "checkdir error:  path too long: %s\n",
                  FnFilter1(G.buildpath)));
                free(G.buildpath);
                /* no room for filenames:  fatal */
                return MPN_ERR_TOOLONG;
            }
            if (mkdir(G.buildpath, 0777) == -1) {   /* create the directory */
                Info(slide, 1, ((char *)slide,
                  "checkdir error:  cannot create %s\n\
                 %s\n\
                 unable to process %s.\n",
                  FnFilter2(G.buildpath),
                  strerror(errno),
                  FnFilter1(G.filename)));
                free(G.buildpath);
                /* path didn't exist, tried to create, failed */
                return MPN_ERR_SKIP;
            }
            G.created_dir = TRUE;
        } else if (!S_ISDIR(G.statbuf.st_mode)) {
            Info(slide, 1, ((char *)slide,
              "checkdir error:  %s exists but is not directory\n\
                 unable to process %s.\n",
              FnFilter2(G.buildpath), FnFilter1(G.filename)));
            free(G.buildpath);
            /* path existed but wasn't dir */
            return MPN_ERR_SKIP;
        }
        if (too_long) {
            Info(slide, 1, ((char *)slide,
              "checkdir error:  path too long: %s\n", FnFilter1(G.buildpath)));
            free(G.buildpath);
            /* no room for filenames:  fatal */
            return MPN_ERR_TOOLONG;
        }
        *G.end++ = '/';
        *G.end = '\0';
        Trace((stderr, "buildpath now = [%s]\n", FnFilter1(G.buildpath)));
        return MPN_OK;

    } /* end if (FUNCTION == APPEND_DIR) */

/*---------------------------------------------------------------------------
    GETPATH:  copy full path to the string pointed at by pathcomp, and free
    G.buildpath.
  ---------------------------------------------------------------------------*/
    if (FUNCTION == GETPATH) {
// kolibri UTf8 support
#ifdef UNICODE_SUPPORT
        if(G.native_is_utf8)
        {
			if (G.buildpath[0] == '/')
			{
	            pathcomp[0] = '/';  // kolibri utf8 flag
    	        pathcomp[1] = 3;  // kolibri utf8 flag
        	    strcpy(pathcomp + 2, G.buildpath);
			} else
			{
    	        pathcomp[0] = 3;  // kolibri utf8 flag
        	    strcpy(pathcomp + 1, G.buildpath);
			}
        }
            else
#endif // UNICODE_SUPPORT
        strcpy(pathcomp, G.buildpath);
        Trace((stderr, "getting and freeing path [%s]\n",
          FnFilter1(pathcomp)));
        free(G.buildpath);
        G.buildpath = G.end = (char *)NULL;
        return MPN_OK;
    }

/*---------------------------------------------------------------------------
    APPEND_NAME:  assume the path component is the filename; append it and
    return without checking for existence.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_NAME) {
#ifdef SHORT_NAMES
        char *old_end = end;
#endif

        Trace((stderr, "appending filename [%s]\n", FnFilter1(pathcomp)));
        while ((*G.end = *pathcomp++) != '\0') {
            ++G.end;
#ifdef SHORT_NAMES  /* truncate name at 14 characters, typically */
            if ((G.end-old_end) > FILENAME_MAX)    /* GRR:  proper constant? */
                *(G.end = old_end + FILENAME_MAX) = '\0';
#endif
            if ((G.end-G.buildpath) >= FILNAMSIZ) {
                *--G.end = '\0';
                Info(slide, 0x201, ((char *)slide,
                  "checkdir warning:  path too long; truncating\n\
                   %s\n                -> %s\n",
                  FnFilter1(G.filename), FnFilter2(G.buildpath)));
                return MPN_INF_TRUNC;   /* filename truncated */
            }
        }
        Trace((stderr, "buildpath now = [%s]\n", FnFilter1(G.buildpath)));
        /* could check for existence here, prompt for new name... */
        return MPN_OK;
    }

/*---------------------------------------------------------------------------
    INIT:  allocate and initialize buffer space for the file currently being
    extracted.  If file was renamed with an absolute path, don't prepend the
    extract-to path.
  ---------------------------------------------------------------------------*/

/* GRR:  for VMS and TOPS-20, add up to 13 to strlen */

    if (FUNCTION == INIT) {
        Trace((stderr, "initializing buildpath to "));
#ifdef ACORN_FTYPE_NFS
        if ((G.buildpath = (char *)malloc(strlen(G.filename)+G.rootlen+
                                          (uO.acorn_nfs_ext ? 5 : 1)))
#else
        if ((G.buildpath = (char *)malloc(strlen(G.filename)+G.rootlen+1))
#endif
            == (char *)NULL)
            return MPN_NOMEM;
        if ((G.rootlen > 0) && !G.renamed_fullpath) {
            strcpy(G.buildpath, G.rootpath);
            G.end = G.buildpath + G.rootlen;
        } else {
            *G.buildpath = '\0';
            G.end = G.buildpath;
        }
        Trace((stderr, "[%s]\n", FnFilter1(G.buildpath)));
        return MPN_OK;
    }

/*---------------------------------------------------------------------------
    ROOT:  if appropriate, store the path in rootpath and create it if
    necessary; else assume it's a zipfile member and return.  This path
    segment gets used in extracting all members from every zipfile specified
    on the command line.
  ---------------------------------------------------------------------------*/

#if (!defined(SFX) || defined(SFX_EXDIR))
    if (FUNCTION == ROOT) {
        Trace((stderr, "initializing root path to [%s]\n",
          FnFilter1(pathcomp)));
        if (pathcomp == (char *)NULL) {
            G.rootlen = 0;
            return MPN_OK;
        }
        if (G.rootlen > 0)      /* rootpath was already set, nothing to do */
            return MPN_OK;
        if ((G.rootlen = strlen(pathcomp)) > 0) {
            char *tmproot;

            if ((tmproot = (char *)malloc(G.rootlen+2)) == (char *)NULL) {
                G.rootlen = 0;
                return MPN_NOMEM;
            }
            strcpy(tmproot, pathcomp);
            if (tmproot[G.rootlen-1] == '/') {
                tmproot[--G.rootlen] = '\0';
            }
            if (G.rootlen > 0 && (SSTAT(tmproot, &G.statbuf) ||
                                  !S_ISDIR(G.statbuf.st_mode)))
            {   /* path does not exist */
                if (!G.create_dirs /* || iswild(tmproot) */ ) {
                    free(tmproot);
                    G.rootlen = 0;
                    /* skip (or treat as stored file) */
                    return MPN_INF_SKIP;
                }
                /* create the directory (could add loop here scanning tmproot
                 * to create more than one level, but why really necessary?) */
                if (mkdir(tmproot, 0777) == -1) {
                    Info(slide, 1, ((char *)slide,
                      "checkdir:  cannot create extraction directory: %s\n\
           %s\n",
                      FnFilter1(tmproot), strerror(errno)));
                    free(tmproot);
                    G.rootlen = 0;
                    /* path didn't exist, tried to create, and failed: */
                    /* file exists, or 2+ subdir levels required */
                    return MPN_ERR_SKIP;
                }
            }
            tmproot[G.rootlen++] = '/';
            tmproot[G.rootlen] = '\0';
            if ((G.rootpath = (char *)realloc(tmproot, G.rootlen+1)) == NULL) {
                free(tmproot);
                G.rootlen = 0;
                return MPN_NOMEM;
            }
            Trace((stderr, "rootpath now = [%s]\n", FnFilter1(G.rootpath)));
        }
        return MPN_OK;
    }
#endif /* !SFX || SFX_EXDIR */

/*---------------------------------------------------------------------------
    END:  free rootpath, immediately prior to program exit.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == END) {
        Trace((stderr, "freeing rootpath\n"));
        if (G.rootlen > 0) {
            free(G.rootpath);
            G.rootlen = 0;
        }
        return MPN_OK;
    }

    return MPN_INVALID; /* should never reach */

} /* end function checkdir() */


/****************************/
/* Function dos_to_kolibri_time() */
/****************************/
void dos_to_kolibri_time(unsigned dos_datetime, unsigned *fdat, unsigned *ftim)
{
    char* pc = (char*)fdat;
    *pc++ = (dos_datetime >> 16) & 0x1F; // day
    *pc++ = (dos_datetime >> 21) & 0xF; // month
    short *ps = (short*)pc;
    *ps = (dos_datetime >> 25) + 1980; // year
    *ftim = 0;
    pc = (char*)ftim;
    *pc++ = (dos_datetime & 0x1F) << 1; // sec
    *pc++ = (dos_datetime >> 5) & 0x3F; // min
    *pc = (dos_datetime >> 11) & 0x1F; // hour
} /* end function dos_to_kolibri_time() */


/****************************/
/* Function close_outfile() */
/****************************/

void close_outfile(__G)    /* GRR: change to return PK-style warning level */
    __GDEF
{

#if (defined(NO_FCHOWN))
    fclose(G.outfile);
    Trace((stdout, "File (%s) closed\n", FnFilter1(G.filename)));
#endif

    /* skip restoring time stamps on user's request */
    if (uO.D_flag <= 1) {
        /* set the file's access and modification times */
        /* siemargl: dont using extra fields */
        unsigned ftim, fdat;
        dos_to_kolibri_time(G.lrec.last_mod_dos_datetime, &fdat, &ftim);

        fileinfo_t  finfo;
        if (get_fileinfo(G.filename, &finfo))
        {
            Info(slide, 1, ((char *)slide, CannotGetTimestamps,
                  FnFilter1(G.filename)));
            return;
        }
        finfo.flags = G.pInfo->file_attr;
        finfo.cr_time = finfo.acc_time = finfo.mod_time = ftim;
        finfo.cr_date = finfo.acc_date = finfo.mod_date = fdat;
        Trace((stderr, "Trying to set fileinfo[%s] date %X, time %X, attr %X\n", FnFilter1(G.filename), fdat, ftim, finfo.flags));

        if (set_fileinfo(G.filename, &finfo))
        {
            Info(slide, 1, ((char *)slide, CannotSetItemTimestamps,
                  FnFilter1(G.filename), strerror(errno)));
            return;
        }
    }

#if (defined(NO_FCHOWN) || defined(NO_FCHMOD))
/*---------------------------------------------------------------------------
    Change the file permissions from default ones to those stored in the
    zipfile.
  ---------------------------------------------------------------------------*/

#ifndef NO_CHMOD
    if (chmod(G.filename, filtattr(__G__ G.pInfo->file_attr)))
        perror("chmod (file attributes) error");
#endif
#endif /* NO_FCHOWN || NO_FCHMOD */


} /* end function close_outfile() */


/**********************/
/* Function do_wild() */   /* for porting: dir separator; match(ignore_case) */
/**********************/

char *do_wild(__G__ wildspec)
    __GDEF
    ZCONST char *wildspec;  /* only used first time on a given dir */
{
/* these statics are now declared in SYSTEM_SPECIFIC_GLOBALS in unxcfg.h:
    static DIR *wild_dir = (DIR *)NULL;
    static ZCONST char *wildname;
    static char *dirname, matchname[FILNAMSIZ];
    static int notfirstcall=FALSE, have_dirname, dirnamelen;
*/
    struct dirent *file;

    /* Even when we're just returning wildspec, we *always* do so in
     * matchname[]--calling routine is allowed to append four characters
     * to the returned string, and wildspec may be a pointer to argv[].
     */
    if (!G.notfirstcall) {  /* first call:  must initialize everything */
        G.notfirstcall = TRUE;

        if (!iswild(wildspec)) {
            strncpy(G.matchname, wildspec, FILNAMSIZ);
            G.matchname[FILNAMSIZ-1] = '\0';
            G.have_dirname = FALSE;
            G.wild_dir = NULL;
            return G.matchname;
        }

        /* break the wildspec into a directory part and a wildcard filename */
        if ((G.wildname = (ZCONST char *)strrchr(wildspec, '/')) == NULL) {
            G.dirname = ".";
            G.dirnamelen = 1;
            G.have_dirname = FALSE;
            G.wildname = wildspec;
        } else {
            ++G.wildname;     /* point at character after '/' */
            G.dirnamelen = G.wildname - wildspec;
            if ((G.dirname = (char *)malloc(G.dirnamelen+1)) == (char *)NULL) {
                Info(slide, 0x201, ((char *)slide,
                  "warning:  cannot allocate wildcard buffers\n"));
                strncpy(G.matchname, wildspec, FILNAMSIZ);
                G.matchname[FILNAMSIZ-1] = '\0';
                return G.matchname; /* but maybe filespec was not a wildcard */
            }
            strncpy(G.dirname, wildspec, G.dirnamelen);
            G.dirname[G.dirnamelen] = '\0';   /* terminate for strcpy below */
            G.have_dirname = TRUE;
        }

        if ((G.wild_dir = (zvoid *)opendir(G.dirname)) != (zvoid *)NULL) {
            while ((file = readdir((DIR *)G.wild_dir)) !=
                   (struct dirent *)NULL) {
                Trace((stderr, "do_wild:  readdir returns %s\n",
                  FnFilter1(file->d_name)));
                if (file->d_name[0] == '.' && G.wildname[0] != '.')
                    continue; /* Unix:  '*' and '?' do not match leading dot */
                if (match(file->d_name, G.wildname, 0 WISEP) &&/*0=case sens.*/
                    /* skip "." and ".." directory entries */
                    strcmp(file->d_name, ".") && strcmp(file->d_name, "..")) {
                    Trace((stderr, "do_wild:  match() succeeds\n"));
                    if (G.have_dirname) {
                        strcpy(G.matchname, G.dirname);
                        strcpy(G.matchname+G.dirnamelen, file->d_name);
                    } else
                        strcpy(G.matchname, file->d_name);
                    return G.matchname;
                }
            }
            /* if we get to here directory is exhausted, so close it */
            closedir((DIR *)G.wild_dir);
            G.wild_dir = (zvoid *)NULL;
        }
        Trace((stderr, "do_wild:  opendir(%s) returns NULL\n",
          FnFilter1(G.dirname)));

        /* return the raw wildspec in case that works (e.g., directory not
         * searchable, but filespec was not wild and file is readable) */
        strncpy(G.matchname, wildspec, FILNAMSIZ);
        G.matchname[FILNAMSIZ-1] = '\0';
        return G.matchname;
    }

    /* last time through, might have failed opendir but returned raw wildspec */
    if ((DIR *)G.wild_dir == (DIR *)NULL) {
        G.notfirstcall = FALSE; /* nothing left--reset for new wildspec */
        if (G.have_dirname)
            free(G.dirname);
        return (char *)NULL;
    }

    /* If we've gotten this far, we've read and matched at least one entry
     * successfully (in a previous call), so dirname has been copied into
     * matchname already.
     */
    while ((file = readdir((DIR *)G.wild_dir)) != (struct dirent *)NULL) {
        Trace((stderr, "do_wild:  readdir returns %s\n",
          FnFilter1(file->d_name)));
        if (file->d_name[0] == '.' && G.wildname[0] != '.')
            continue;   /* Unix:  '*' and '?' do not match leading dot */
        if (match(file->d_name, G.wildname, 0 WISEP)) { /* 0 == case sens. */
            Trace((stderr, "do_wild:  match() succeeds\n"));
            if (G.have_dirname) {
                /* strcpy(G.matchname, G.dirname); */
                strcpy(G.matchname+G.dirnamelen, file->d_name);
            } else
                strcpy(G.matchname, file->d_name);
            return G.matchname;
        }
    }

    closedir((DIR *)G.wild_dir);  /* at least one entry read; nothing left */
    G.wild_dir = (zvoid *)NULL;
    G.notfirstcall = FALSE;       /* reset for new wildspec */
    if (G.have_dirname)
        free(G.dirname);
    return (char *)NULL;

} /* end function do_wild() */

#ifdef SET_DIR_ATTRIB
int defer_dir_attribs(__G__ pd)
    __GDEF
    direntry **pd;
{
    // do nothing
    return PK_OK;
}

int set_direc_attribs(__G__ d)
    __GDEF
    direntry *d;
{
    /* skip restoring time stamps on user's request */
    if (uO.D_flag <= 0) {
        /* set the file's access and modification times */
        /* siemargl: dont using extra fields */
        unsigned ftim, fdat;
        dos_to_kolibri_time(G.lrec.last_mod_dos_datetime, &fdat, &ftim);

        fileinfo_t  finfo;
        if (get_fileinfo(G.filename, &finfo))
        {
            Info(slide, 1, ((char *)slide, CannotGetTimestamps,
                  FnFilter1(G.filename)));
            return PK_WARN;
        }
        finfo.flags = G.pInfo->file_attr;
        finfo.cr_time = finfo.acc_time = finfo.mod_time = ftim;
        finfo.cr_date = finfo.acc_date = finfo.mod_date = fdat;
        Trace((stderr, "Trying to set dirinfo[%s] date %X, time %X, attr %X\n", FnFilter1(G.filename), fdat, ftim, finfo.flags));

        if (set_fileinfo(G.filename, &finfo))
        {
            Info(slide, 1, ((char *)slide, CannotSetItemTimestamps,
                  FnFilter1(G.filename), strerror(errno)));
            return PK_WARN;
        }
    }

    return PK_OK;
} /* end function set_direc_attribs() */

#endif // SET_DIR_ATTRIB

