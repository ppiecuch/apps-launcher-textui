// kate: replace-tabs on; tab-indents on; tab-width 4; indent-width 4; indent-mode cstyle;

#include "textUI_support.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#if defined __MSDOS__ || defined __WIN32__ || defined _Windows
# define DIRSEP_CHAR '\\'
#elif defined macintosh /* only the original Macintosh uses ':', OSX uses the '/' */
# define DIRSEP_CHAR ':'
#else
# define DIRSEP_CHAR '/'
#endif

/// BEGIN Host-system integration

BOOL system_keyhit(void) { return FALSE; }

int system_getkey(void) { return 0; }

int system_getshift(void) { return 0; }

void system_resetmouse(void) { }

int system_mousebuttons(void) { return 0; }

static struct {
    int x, y;
} _pointer_position;

void system_get_mouseposition(int *x, int *y) {
    *x = _pointer_position.x;
    *y = _pointer_position.y;
}

int system_button_releases(void) { return 0; }

/// END Host-system integration

#include <ctype.h>

#ifndef _tolower
#  define _tolower(c) ((c) + 'a' - 'A')
#endif
#ifndef _toupper
#  define _toupper(c) ((c) + 'A' - 'a')
#endif

#if defined(UNIX)
/*  some compilers don't check if a character is already upper or lower
 *  case in the toupper/tolower functions, so we define our own here.
 */
#  define ToLower(c)   (isupper((int)c) ? _tolower(c) : (c))
#  define ToUpper(c)   (islower((int)c) ? _toupper(c) : (c))
#else
#  define ToLower      tolower          /* assumed "smart"; used in match() */
#endif

#ifndef S_ISDIR
#  define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif

/* MS-DOS flags equivalent to "dos.h" file attribute definitions */

#define DD_NORMAL   0x00    /* Normal file, no attributes */
#define DD_RDONLY   0x01    /* Read only attribute */
#define DD_HIDDEN   0x02    /* Hidden file */
#define DD_SYSTEM   0x04    /* System file */
#define DD_LABEL    0x08    /* Volume label */
#define DD_DIREC    0x10    /* Directory */
#define DD_ARCH	    0x20    /* Archive */
#define DD_DEVICE   0x40    /* Device */

#define DD_DOSATTRIBS 0x3f          /* DOS ATTRIBUTE MASK */

/*
 *  note that all DOS file attributes defined above do not overlap the
 *  DOS stat definitions, but will conflict will non-DOS machines, so
 *  use following macros to access the flags instead.
 */

#define DD_ISNORMAL(m)   ((m) & S_IFREG)
#ifdef MSDOS
#  define DD_ISRDONLY(m) ((m) & DD_RDONLY)
#  define DD_ISHIDDEN(m) ((m) & DD_HIDDEN)
#  define DD_ISSYSTEM(m) ((m) & DD_SYSTEM)
#  define DD_ISLABEL(m)  ((m) & DD_LABEL)
#  define DD_ISDIREC(m)  ((m) & (DD_DIREC | S_IFDIR))
#  define DD_ISARCH(m)   ((m) & DD_ARCH)
#else
#  define DD_ISRDONLY(m) !((m) & S_IWRITE)
#  define DD_ISHIDDEN(m) (0)
#  define DD_ISSYSTEM(m) (0)
#  define DD_ISLABEL(m)  (0)
#  define DD_ISDIREC(m)  ((m) & S_IFDIR)
#  define DD_ISARCH(m)   (0)
#endif /* ?MSDOS */

#ifdef UNIX
#  include <errno.h>
#  ifndef ENOENT
#    define ENOENT -1
#  endif
#  ifndef ENMFILE
#    define ENMFILE ENOENT
#  endif
#endif /* ?UNIX/VMS */

#ifdef UNIX
#  include <pwd.h>
#  define DF_MAXUSERNAME 100
#  define STAT lstat /* don't expand symbolic links */
#  include <unistd.h>
#  ifndef STDIN_FILENO
#   define STDIN_FILENO 0
#  endif
#  ifndef STDERR_FILENO
#   define STDERR_FILENO 2
#  endif
#  ifndef O_RDONLY
#   define O_RDONLY 0
#  endif
#  ifndef O_WRONLY
#   define O_WRONLY 1
#  endif
#else /* ?MSDOS\VMS */
#  define STAT stat
#endif

/* flags used by fnsplit */

#ifndef __TURBOC__
#  define WILDCARDS 0x01
#  define EXTENSION 0x02
#  define FILENAME  0x04
#  define DIRECTORY 0x08
#  define DRIVE     0x10
#endif

/* copy_string - copies a string to another */
static void copy_string(char *dst, const char *src, unsigned maxlen) {
    if (dst) {
        if (strlen(src) >= maxlen) {
            strncpy(dst, src, maxlen);
            dst[maxlen] = 0;
        } else {
            strcpy(dst, src);
        }
    }
}

/* dot_found - checks for special directory names */
static  int dot_found(char *pB) {
    if (*(pB-1) == '.') {
        pB--;
    }
    switch (*--pB) {
#ifdef MSDOS
    case ':':
        if (*(pB-2) != '\0') {
            break;
        }
    case '\\':
#endif
    case '/'  :
    case '\0' :
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------*
Name            path_split - splits a full path name into its components

Description     path_split takes a file's full path name (path) as a
                string in the form

    /DIR/SUBDIR/FILENAME                (UNIX)
    X:\DIR\SUBDIR\NAME.EXT              (MS-DOS)

    and splits path into its four components. It then stores
    those components in the strings pointed to by dir and
    ext.  (Each component is required but can be a NULL,
    which means the corresponding component will be parsed
    but not stored.)

    The maximum sizes for these strings are given by the
    constants MAXDRIVE, MAXDIR, MAXPATH, MAXNAME and MAXEXT,
    (defined in dosdir.h) and each size includes space for
    the null-terminator.

    Constant        String

    DD_MAXPATH      path
    DD_MAXDRIVE     drive; includes colon; not used by UNIX
    DD_MAXDIR       dir; includes leading and trailing
                    backslashes for DOS or slashes for UNIX
    DD_MAXFILE      filename
    DD_MAXEXT       ext; includes leading dot (.)
                    (not used by UNIX)

    path_split assumes that there is enough space to store each
    non-NULL component. fnmerge assumes that there is enough
    space for the constructed path name. The maximum constructed
    length is DD_MAXPATH.

    When path_split splits path, it treats the punctuation as
    follows:

    * drive keeps the colon attached (C:, A:, etc.).
      It is not applicable to unix file system.

    * dir keeps the leading and trailing slashes
      (/usr/local/bin/, /src/, etc.)

    * ext keeps the dot preceding the extension (.c, .doc, etc.)
      It is not applicable to unix file system.

Return value    path_split returns an integer (composed of five flags,
        defined in dosdir.h) indicating which of the full path name
        components were present in path; these flags and the components
        they represent are:

        EXTENSION       an extension
        FILENAME        a filename
        DIRECTORY       a directory (and possibly sub-directories)
        DRIVE           a drive specification (see dir.h)
        WILDCARDS       wildcards (* or ? cards)

*---------------------------------------------------------------------*/

int path_split(const char *pathP, char *driveP, char *dirP, char *nameP, char *extP) {
    char buf[ DF_MAXPATH+2 ];

    /* Set all string to default value zero */
    int Ret = 0;
    if (driveP) *driveP = 0;
    if (dirP) *dirP = 0;
    if (nameP) *nameP = 0;
    if (extP) *extP = 0;

    /* Copy filename into template up to DD_MAXPATH characters */
    int Wrk;
    char *pB = buf;
    while (*pathP == ' ') pathP++;
    if ((Wrk = strlen(pathP)) > DF_MAXPATH)
        Wrk = DF_MAXPATH;
    *pB++ = 0;
    strncpy(pB, pathP, Wrk);
    *(pB += Wrk) = 0;

    /* Split the filename and fill corresponding nonzero pointers */
    Wrk = 0;
    for (; ; ) {
        switch (*--pB) {
        case '.':
            if (!Wrk && (*(pB+1) == '\0')) Wrk = dot_found(pB);
#ifdef MSDOS
            if ((!Wrk) && ((Ret & EXTENSION) == 0)) {
                Ret |= EXTENSION;
                copy_string(extP, pB, DF_MAXEXT - 1);
                *pB = 0;
            }
#endif
            continue;
#if defined(MSDOS)
        case ':'  :
            if (pB != &buf[2])
                continue;
#elif defined(UNIX)
        case '~' :
            if (pB != &buf[1])
                continue;
            else {
                /* expand path as appropriate */
                struct passwd *pw = NULL;
                char* tail = strchr (pB, '/');
                int len;
                if (tail != NULL) {
                    len = tail - (pB+1);
                    if (len > 0) {
                        char username[DF_MAXUSERNAME];
                        if (len <= DF_MAXUSERNAME) {
                            strncpy(username, pB+1, len);
                            username[len] = 0;
                            pw = getpwnam(username);
                        }
                    } else {
                        pw = getpwuid (getuid());
                    }
                    if (pw != NULL && (len=strlen(pw->pw_dir)) < DF_MAXDIR) {
                        strcpy(dirP, pw->pw_dir);
                        dirP += len;
                    } else
                        strcpy (dirP++, "?");
                    copy_string(dirP, tail, DF_MAXDIR - len - 1);
                    dirP += strlen(dirP);
                } else {
                    Wrk = 1;
                    if (pB[1] != 0)
                        pw = getpwnam (pB + 1);
                    else
                        pw = getpwuid (getuid());

                    if (pw != NULL && (len=strlen(pw->pw_dir)) < DF_MAXDIR) {
                        strcpy(dirP, pw->pw_dir);
                        dirP += len;
                    } else
                        strcpy (dirP++, "?");
                }
                *pB-- = 0;
                Ret |= DIRECTORY;
            }
#endif /* ?MSDOS */
        case '\0' :
            if (Wrk) {
                if (*++pB)
                    Ret |= DIRECTORY;
                copy_string(dirP, pB, DF_MAXDIR - 1);
#ifdef MSDOS
                *pB-- = 0;
#endif
                break;
            }
#ifdef MSDOS
        case '\\' :
#endif
#if (defined(MSDOS) || defined(UNIX))
        case '/':
#endif
            if (!Wrk) {
                Wrk++;
                if (*++pB) Ret |= FILENAME;
                copy_string(nameP, pB, DF_MAXFILE - 1);
                *pB-- = 0;
#ifdef MSDOS
                if (*pB == 0 || (*pB == ':' && pB == &buf[2]))
#else
                if (*pB == 0)
#endif
                break;
            }
            continue;
        case '[' :
        case '*' :
        case '?' :
            if (!Wrk) Ret |= WILDCARDS;
                default :
            continue;
        }
        break;
    }

#ifdef MSDOS
    if (*pB == ':') {
        if (buf[1]) Ret |= DRIVE;
        copy_string(driveP, &buf[1], DD_MAXDRIVE - 1);
    }
#endif

    return (Ret);
}

/*---------------------------------------------------------------------*
Name            path_merge - portable replacement for fnmerge(), _makepath(), etc

Description     forms a full DOS pathname from drive, path, file, and extension
                specifications.

Arguments:      1 - Buffer to receive full pathname
                2 - Drive
                3 - Path
                4 - Name
                5 - Extension

Returns:        nothing
*---------------------------------------------------------------------*/

#define LAST_CHAR(s) ((s)[strlen(s) - 1])

void path_merge(char *path, char *drive, char *dir, char *fname, char *ext) {
    *path = '\0';

    if (drive && *drive) {
        strcat(path, drive);
        if (':' != LAST_CHAR(path))
            strcat(path, ":");
    }

    if (dir && *dir) {
        strcat(path, dir);
        for (char *p = path; *p; ++p)
            if ('/' == *p)
                *p = '\\';
        if ('\\' != LAST_CHAR(path))
            strcat(path, "\\");
    }

    if (fname && *fname) {
        strcat(path, fname);

        if (ext && *ext) {
            if ('.' != *ext)
                strcat(path, ".");
            strcat(path, ext);
        }
    }
}


/*---------------------------------------------------------------------*

Name        IO file access layer.

*---------------------------------------------------------------------*/

#include <stdio.h>

GFILE *file_open(const char *name, const char *mode) { return (GFILE *)fopen(name, mode); }
int file_stat(const char *path, struct stat *buf) { return stat(path, buf); }
int file_fclose(GFILE *f) { return fclose((FILE *)f); }
int file_fseek(GFILE *f, off_t offset, int whence) { return fseek((FILE *)f, offset, whence); }
off_t file_ftell(GFILE *f) { return ftell((FILE *)f); }
ssize_t file_fread(void* buf, size_t len, size_t cnt, GFILE *f) { return fread(buf, len, cnt, (FILE *)f); }
ssize_t file_fwrite(const void* buf, size_t len, size_t cnt, GFILE *f) { return fwrite(buf, len, cnt, (FILE *)f); }

/*
  DOSDIR V2.1a: A Portable DOS/UNIX/VMS Directory Interface

  Implementation of the DOS directory functions (findfirst and findnext)
  on MS-DOS, UNIX and VMS platforms using the appropriate file & directory
  structure.

  Provides the DOS directory framework for MS-DOS/UNIX/VMS application
  portability.

  Supports MS-DOS with Borland C++, Turbo C, or Microsoft C V6.0/7.0,
  Sun with GNU C compiler, DEC Alpha (OSF-1), VAX/VMS C,
  and other comparable platforms.

  Written by: Jason Mathews <mathews@mitre.org>

  ---------------------------------------------------------------------------

 Modification history:
   V1.0  02-May-91,  Original version.
   V2.0  13-May-94,  Reimplemented findfirst/findnext with ffblk structure
                     to match MS-DOS externally, fixed wildcard evaluation
                     function.
   V2.1  08-Jun-94,  Replaced wildcard evaluation function with recursive
                     function provided by Info-ZIP's portable UnZip,
                     added dd_ prefix to most functions & constants,
                     added VMS functions + MSC/TURBOC macros.
   V2.1a 16-Oct-96,  Call lstat() instead of stat() to avoid expanding on
                     symbolic linked directories.
*/

/* global stat structure of last successful file
* returned by findfirst/findnext functions available
* to query for more detailed information.
*/
struct stat dir_sstat;

#ifdef MSDOS
#  ifdef __TURBOC__
#    define FSTRUCT       struct ffblk
#    define FATTR         FA_HIDDEN+FA_SYSTEM+FA_DIREC
#    define FFIRST(n,d,a) findfirst(n,d,a)
#    define FNEXT(d)      findnext(d)
#    define FNAME         ff_name
#    define FATTRIB       ff_attrib
#    define FSIZE         ff_fsize
#    define FDATE         ff_fdate
#    define FTIME         ff_ftime
#  else /* !__TURBOC__ */
#    define FSTRUCT       struct find_t
#    define FATTR         _A_HIDDEN+_A_SYSTEM+_A_SUBDIR
#    define FFIRST(n,d,a) _dos_findfirst(n,a,d)
#    define FNEXT(d)      _dos_findnext(d)
#    define FNAME         name
#    define FATTRIB       attrib
#    define FSIZE         size
#    define FDATE         wr_date
#    define FTIME         wr_time
#  endif /* ?__TURBOC__ */
#else /* ?UNIX/VMS */

/* stub functions for get/set disk
* fake MS-DOS functions that do not apply to unix or vms:
*/

int getdisk() { return 0; }
int setdisk(int drive) { return 0; }

#endif /* ?MSDOS */

#ifdef MSDOS

static int dir_initstruct( dir_ffblk* fb )
{
    fb->dd_name = fb->dos_fb.FNAME;

    /*  ".." entry refers to the directory entry of the cwd and *NOT* the
    *   parent directory, so we use "." instead.
    */
    if (STAT(!strcmp(fb->dd_name, "..") ? "." : fb->dd_name, &dir_sstat))
        return -1; /* stat failed! */

    fb->dd_time = dir_sstat.st_mtime;
    fb->dd_size = fb->dos_fb.FSIZE;
    fb->dd_mode = fb->dos_fb.FATTRIB & DD_DOSATTRIBS | dir_sstat.st_mode & ~DD_DOSATTRIBS;
    return 0;
}

/*
* Function: dir_findnext
*
* Purpose:  Use dir_findnext after dir_findfirst to find the remaining files
*
* Returns:  zero if successful, otherwise, it returns a -1
*           and sets errno either to the constant ENOENT indicating
*           that the file could not be found, or to ENMFILE
*           signifying that there are no more files.
*
*  Parms:
*      dir_ffblk* fb  = structure to hold results of search
*/
int dir_findnext(dir_ffblk* fb)
{
    int rc;
    /* repeat until file info is initialized or no more files are left */
    while ((rc=FNEXT(&fb->dos_fb)) == 0 && (rc=dd_initstruct(fb)) != 0);
    return rc;
}

/*
* Function: dir_findfirst
*
* Purpose: Find file matching specification in specified directory
*          given directory information
*
* Returns: zero if successful, otherwise, it returns a -1
*          and sets errno either to the constant ENOENT indicating
*          that the file could not be found, or to ENMFILE
*          signifying that there are no more files.
*
*  Parms:
*      char *filespec    = filename to search for including path
*      dd_ ffblk* fb     = structure to hold results of search
*      int attrib        = file attributes to match
*/
int dir_findfirst(const char *path, dir_ffblk *fb, int attrib)
{
    int rc;
    if ((rc = FFIRST( path, &fb->dos_fb, attrib & DD_DOSATTRIBS)) == 0)
    {
        if ((rc = dd_initstruct(fb)) != 0) /* initialization failed? */
            rc = dd_findnext( fb );
    }
    return rc;
}

#else /* ?UNIX */

int dir_findnext(dir_ffblk *fb)
{
    if (!fb->dd_dirp) goto findnext_err;
    while ((fb->dd_dp = readdir(fb->dd_dirp)) != NULL)
    {
        if (STAT(fb->dd_dp->d_name, &dir_sstat))
            continue;
        if (dir_sstat.st_mode & S_IFDIR && !(fb->dd_attribs & DD_DIREC))
            continue;
        if (dir_match(fb->dd_dp->d_name, fb->dd_filespec, 0))
        {
            /* fill in file info */
            fb->dd_name  = fb->dd_dp->d_name;
            fb->dd_size  = dir_sstat.st_size;
            fb->dd_time  = dir_sstat.st_mtime;
            fb->dd_mode  = dir_sstat.st_mode;
            return 0;       /* successful match */
        }
    }  /* while */

    closedir(fb->dd_dirp);

    findnext_err:

    memset(fb, 0x0, sizeof(dir_ffblk)); /* invalidate structure */
    errno = ENOENT;       /* no file found */
    return -1;
}  /** dd_findnext **/

int dir_findfirst(const char *pathname, dir_ffblk* fb, int attrib)
{
    char *s = strrchr(pathname, DIR_END);
    char dir[DF_MAXDIR]; /* directory path */
    if (s)
    {
        strcpy(fb->dd_filespec, s+1);
        strncpy(dir, pathname, s-pathname);
    }
    else
    {
        strcpy(dir, ".");		/* use current directory */
        strcpy(fb->dd_filespec, pathname);
    }
    fb->dd_attribs = attrib;
    fb->dd_dirp    = opendir(dir);
    return dir_findnext(fb);
}  /** dd_findfirst **/

#endif /* ?MSDOS */

#ifdef UNIX

#define error(status, errcode, msg) { \
    fprintf(stderr, "(%s/%d) %s, code: %d", __FILE__, __LINE__, msg, errcode); \
    fflush (stderr); \
    if (status) \
        exit (status); \
}


/* Replacement for Solaris' function by the same name.
* <http://www.google.com/search?q=fstatat+site:docs.sun.com>
* Simulate it by doing save_cwd/fchdir/(stat|lstat)/restore_cwd.
* If either the save_cwd or the restore_cwd fails (relatively unlikely,
* and usually indicative of a problem that deserves close attention),
* then give a diagnostic and exit nonzero.
* Otherwise, this function works just like Solaris' fstatat.
*/
int dir_fstatat (int fd, char const *file, struct stat *st, int flag)
{
    struct saved_cwd saved_cwd;
    int saved_errno;
    int err;

    if (fd == AT_FDCWD)
        return (flag == AT_SYMLINK_NOFOLLOW
            ? lstat (file, st)
            : stat (file, st));
    if (save_cwd (&saved_cwd) != 0)
        error (EXIT_FAILURE, errno, "fstatat: unable to record current working directory");
    if (fchdir (fd) != 0)
    {
        saved_errno = errno;
        free_cwd (&saved_cwd);
        errno = saved_errno;
        return -1;
    }
    err = (flag == AT_SYMLINK_NOFOLLOW
        ? lstat (file, st)
        : stat (file, st));
    saved_errno = errno;

    if (restore_cwd (&saved_cwd) != 0)
        error (EXIT_FAILURE, errno, "fstatat: unable to restore working directory");

    free_cwd (&saved_cwd);

    errno = saved_errno;
    return err;
}


/*---------------------------------------------------------------------------

  The match() routine recursively compares a string to a "pattern" (regular
  expression), returning TRUE if a match is found or FALSE if not.  This
  version is specifically for use with unzip.c:  as did the previous match()
  routines from SEA and J. Kercheval, it leaves the case (upper, lower, or
  mixed) of the string alone, but converts any uppercase characters in the
  pattern to lowercase if indicated by the global var pInfo->lcflag (which
  is to say, string is assumed to have been converted to lowercase already,
  if such was necessary).

  GRR:  reversed order of text, pattern in matche() (now same as match());
        added ignore_case/ic flags, Case() macro.

  PK:   replaced matche() with recmatch() from Zip, modified to have an
        ignore_case argument; replaced test frame with simpler one.

  ---------------------------------------------------------------------------

  Match the pattern (wildcard) against the string (fixed):

     match(string, pattern, ignore_case);

  returns TRUE if string matches pattern, FALSE otherwise.  In the pattern:

     `*' matches any sequence of characters (zero or more)
     `?' matches any character
     [SET] matches any character in the specified set,
     [!SET] or [^SET] matches any character not in the specified set.

  A set is composed of characters or ranges; a range looks like ``character
  hyphen character'' (as in 0-9 or A-Z).  [0-9a-zA-Z_] is the minimal set of
  characters allowed in the [..] pattern construct.  Other characters are
  allowed (ie. 8 bit characters) if your system will support them.

  To suppress the special syntactic significance of any of ``[]*?!^-\'', in-
  side or outside a [..] construct and match the character exactly, precede
  it with a ``\'' (backslash).

  Note that "*.*" and "*." are treated specially under MS-DOS if DOSWILD is
  defined.  See the DOSWILD section below for an explanation.

  ---------------------------------------------------------------------------*/

#define Case(x)  (ic? ToLower(x) : (x))

typedef unsigned char   uch;  /* code assumes unsigned bytes; these type-  */
typedef unsigned short  ush;  /*  defs replace byte/UWORD/ULONG (which are */
typedef unsigned long   ulg;  /*  predefined on some systems) & match zip  */

static int rec_match(uch *pattern, uch *string, int ignore_case);

/* dir_match() is a shell to recmatch() to return only Boolean values. */

int dir_match(const char *string, const char *pattern, int ignore_case)
{
#if (defined(MSDOS) && defined(DOSWILD))
    char *dospattern;
    int j = strlen(pattern);

/*---------------------------------------------------------------------------
    Optional MS-DOS preprocessing section:  compare last three chars of the
    wildcard to "*.*" and translate to "*" if found; else compare the last
    two characters to "*." and, if found, scan the non-wild string for dots.
    If in the latter case a dot is found, return failure; else translate the
    "*." to "*".  In either case, continue with the normal (Unix-like) match
    procedure after translation.  (If not enough memory, default to normal
    match.)  This causes "a*.*" and "a*." to behave as MS-DOS users expect.
  ---------------------------------------------------------------------------*/

    if ((dospattern = (char *)malloc(j+1)) != NULL) {
        strcpy(dospattern, pattern);
        if (!strcmp(dospattern+j-3, "*.*")) {
            dospattern[j-2] = '\0';                    /* nuke the ".*" */
        } else if (!strcmp(dospattern+j-2, "*.")) {
            char *p = strchr(string, '.');

            if (p) {   /* found a dot:  match fails */
                free(dospattern);
                return 0;
            }
            dospattern[j-1] = '\0';                    /* nuke the end "." */
        }
        j = rec_match((uch *)dospattern, (uch *)string, ignore_case);
        free(dospattern);
        return j == 1;
    } else
#endif /* MSDOS && DOSWILD */
    return rec_match((uch *)pattern, (uch *)string, ignore_case) == 1;
}


/* Recursively compare the sh pattern p with the string s and return 1 if
   they match, and 0 or 2 if they don't or if there is a syntax error in the
   pattern.  This routine recurses on itself no more deeply than the number
   of characters in the pattern. */
static int rec_match(
    uch *p,  /* sh pattern to match */
    uch *s,  /* string to which to match it */
    int ic)  /* true for case insensitivity */
{
    /* Get first character, the pattern for new recmatch calls follows
       (pattern char or start of range in [-] loop) */
    unsigned int c = *p++;

    /* If that was the end of the pattern, match if string empty too */
    if (c == 0)
        return *s == 0;

    /* '?' (or '%') matches any character (but not an empty string) */
#ifdef VMS
    if (c == '%')         /* GRR:  make this conditional, too? */
#else /* !VMS */
    if (c == '?')
#endif /* ?VMS */
        return *s ? rec_match(p, s + 1, ic) : 0;

    /* '*' matches any number of characters, including zero */
#ifdef AMIGA
    if (c == '#' && *p == '?')     /* "#?" is Amiga-ese for "*" */
        c = '*', p++;
#endif /* AMIGA */
    if (c == '*') {
        if (*p == 0)
            return 1;
        for (; *s; s++)
            if ((c = rec_match(p, s, ic)) != 0)
                return (int)c;
        return 2;       /* 2 means give up--match will return false */
    }

#ifndef VMS             /* No bracket matching in VMS */
    /* Parse and process the list of characters and ranges in brackets */
    if (c == '[') {
        int e;          /* flag true if next char to be taken literally */
        uch *q;         /* pointer to end of [-] group */
        int r;          /* flag true to match anything but the range */

        if (*s == 0)                           /* need a character to match */
            return 0;
        p += (r = (*p == '!' || *p == '^'));   /* see if reverse */
        for (q = p, e = 0; *q; q++)            /* find closing bracket */
            if (e)
                e = 0;
            else
                if (*q == '\\')      /* GRR:  change to ^ for MS-DOS, OS/2? */
                    e = 1;
                else if (*q == ']')
                    break;
        if (*q != ']')               /* nothing matches if bad syntax */
            return 0;
        for (c = 0, e = *p == '-'; p < q; p++) {  /* go through the list */
            if (e == 0 && *p == '\\')             /* set escape flag if \ */
                e = 1;
            else if (e == 0 && *p == '-')         /* set start of range if - */
                c = *(p-1);
            else {
                unsigned int cc = Case(*s);

                if (*(p+1) != '-')
                    for (c = c ? c : *p; c <= *p; c++)  /* compare range */
                        if (Case(c) == cc)
                            return r ? 0 : rec_match(q + 1, s + 1, ic);
                c = e = 0;   /* clear range, escape flags */
            }
        }
        return r ? rec_match(q + 1, s + 1, ic) : 0;  /* bracket match failed */
    }
#endif /* !VMS */

    /* if escape ('\'), just compare next character */
    if (c == '\\' && (c = *p++) == 0)     /* if \ at end, then syntax error */
        return 0;

    /* just a character--compare it */
    return Case((uch)c) == Case(*s) ? rec_match(p, ++s, ic) : 0;

} /* end function recmatch() */


/* Record the location of the current working directory in CWD so that
* the program may change to other directories and later use restore_cwd
* to return to the recorded location.  This function may allocate
* space using malloc (via xgetcwd) or leave a file descriptor open;
* use free_cwd to perform the necessary free or close.  Upon failure,
* no memory is allocated, any locally opened file descriptors are
* closed;  return non-zero -- in that case, free_cwd need not be
* called, but doing so is ok.  Otherwise, return zero.
*
* The `raison d'etre' for this interface is that the working directory
* is sometimes inaccessible, and getcwd is not robust or as efficient.
* So, we prefer to use the open/fchdir approach, but fall back on
* getcwd if necessary.
*
* Some systems lack fchdir altogether: e.g., OS/2, pre-2001 Cygwin,
* SCO Xenix.  Also, SunOS 4 and Irix 5.3 provide the function, yet it
* doesn't work for partitions on which auditing is enabled.  If
* you're still using an obsolete system with these problems, please
* send email to the maintainer of this code.
*/

/* Return the current directory, newly allocated.
   Upon an out-of-memory error, call exit.
   Upon any other type of error, return NULL. */
static char *xgetcwd (void)
{
    char *cwd = getcwd (NULL, 0);
    if (! cwd && errno == ENOMEM)
        error (EXIT_FAILURE, 0, "memory exhausted");
    return cwd;
}

int save_cwd (struct saved_cwd *cwd)
{
    cwd->name = NULL;

    cwd->desc = open (".", O_RDONLY);
    if (cwd->desc < 0)
    {
        cwd->desc = open (".", O_WRONLY);
        if (cwd->desc < 0)
        {
            cwd->name = xgetcwd ();
            return cwd->name ? 0 : -1;
        }
    }
    return 0;
}

/* Change to recorded location, CWD, in directory hierarchy.
* Upon failure, return -1 (errno is set by chdir or fchdir).
* Upon success, return zero.
*/
int restore_cwd (const struct saved_cwd *cwd)
{
    if (0 <= cwd->desc)
        return fchdir (cwd->desc);
    else
        return chdir (cwd->name); /* may fail for long path names */
}

void free_cwd (struct saved_cwd *cwd)
{
    if (cwd->desc >= 0)
        close (cwd->desc);
    if (cwd->name)
        free (cwd->name);
}

#endif /* UNIX */
