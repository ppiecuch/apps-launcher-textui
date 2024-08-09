#ifndef TEXTUI_SUPPORT_H
#define TEXTUI_SUPPORT_H

/* Get types and stat */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif
#ifndef YES
# define YES TRUE
#endif
#ifndef NO
# define NO FALSE
#endif
#ifndef BOOL
# define BOOL bool
#endif

#if (defined(__MSDOS__) && !defined(MSDOS))
#  define MSDOS
#endif

#if defined(unix) || defined(M_XENIX) || defined(COHERENT) || defined(__hpux) || defined(__QNXNTO__) || defined(__APPLE__)
#  ifndef UNIX
#    define UNIX
#  endif
#endif /* unix || M_XENIX || COHERENT || __hpux */
#if defined(__convexc__) || defined(MINIX) || defined(sgi)
#  ifndef UNIX
#    define UNIX
#  endif
#endif /* __convexc__ || MINIX || sgi */
/* End of set up portability */

#if (defined(UNIX) || defined(VMS))
#  include <errno.h>
#  ifndef ENOENT
#    define ENOENT -1
#  endif
#  ifndef ENMFILE
#    define ENMFILE ENOENT
#  endif
#endif /* ?UNIX/VMS */

#if defined (MSDOS)
#    include <dos.h>
#  ifdef __TURBOC__
#    include <dir.h>
#  else /* ?!__TURBOC__ */
#    include <direct.h>
#  endif /* ?TURBOC */
#  define ALL_FILES_MASK "*.*"
#  define DIR_PARENT ".."
#  define DIR_END '\\'
#elif defined(VMS)
#  include <rms.h>
#  define ALL_FILES_MASK "*.*"
#  define DIR_PARENT "[-]"
#  define DIR_END ']'
#else /* ?UNIX */
#  include <memory.h>
#  include <dirent.h>
#  define ALL_FILES_MASK "*"
#  define DIR_PARENT ".."
#  define DIR_END '/'
#endif /* ?__TURBOC__ */

#if defined (MSDOS)
# define DF_MAXDRIVE   3
# ifndef __FLAT__
#  define DF_MAXPATH  80
#  define DF_MAXDIR   66
#  define DF_MAXFILE  16
#  define DF_MAXEXT   10 /* allow for wildcards .[ch]*, .etc */
# else
#  define DF_MAXPATH  260
#  define DF_MAXDIR   256
#  define DF_MAXFILE  256
#  define DF_MAXEXT   256
# endif /* ?__FLAT__ */
   typedef long off_t;
# ifdef __TURBOC__
     typedef short mode_t;
# else /* ?!__TURBOC__ */
     typedef unsigned short mode_t;
# endif /* ?__TURBOC__ */
#else /* ?unix */
/* _MAX_PATH is sometimes called differently and it may be in limits.h or stdlib.h instead of stdio.h. */
# if !defined _MAX_PATH
/* not defined, perhaps stdio.h was not included */
#  if !defined PATH_MAX
#   include <stdio.h>
#  endif
#  if !defined _MAX_PATH && !defined PATH_MAX
/* no _MAX_PATH and no MAX_PATH, perhaps it is in limits.h */
#   include <limits.h>
#  endif
#  if !defined _MAX_PATH && !defined PATH_MAX
/* no _MAX_PATH and no MAX_PATH, perhaps it is in stdlib.h */
#   include <stdlib.h>
#  endif
/* if _MAX_PATH is undefined, try common alternative names */
#  if !defined _MAX_PATH
#   if defined MAX_PATH
#    define _MAX_PATH    MAX_PATH
#   elif defined _POSIX_PATH_MAX
#    define _MAX_PATH  _POSIX_PATH_MAX
#   else
/* everything failed, actually we have a problem here... */
#    define _MAX_PATH  1024
#   endif
#  endif
# endif
/* DD_MAXPATH defines the longest permissable path length,
 * including the terminating null. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly. For now we realy on _MAX_PATH value */
#  define DF_MAXPATH    _MAX_PATH
#  define DF_MAXDRIVE   1
#  define DF_MAXDIR     768
#  define DF_MAXFILE    255
#  define DF_MAXEXT     1
   typedef struct dirent DIR_ENT;
#endif /* ?MSDOS */

#ifndef __TURBOC__
int getdisk(void);
int setdisk(int drive);
#endif /* ?!__TURBOC__ */

int path_split(const char *, char *, char *, char *, char *);
void path_merge(char *, char *, char *, char *, char *);

typedef struct {
    char*  dd_name;             /* File name */
    time_t dd_time;             /* File time stamp */
    off_t  dd_size;             /* File length */
    mode_t dd_mode;             /* Attributes of file */

    /*  Below is private (machine specific) data, which should
     *  only be accessed by dosdir modules.
     */
#if defined (MSDOS)
#  ifdef __TURBOC__
    struct ffblk  dos_fb;
#  else /* ?MSC */
    struct find_t dos_fb;
#  endif /* ?TURBOC */
#else /* ?unix */
    DIR*          dd_dirp;                /* Directory ptr */
    DIR_ENT*      dd_dp;                  /* Directory entry */
    char          dd_attribs;             /* File search attributes */
    char          dd_filespec[DF_MAXFILE];   /* File search mask */
#endif /* ?MSDOS */
} dir_ffblk;

int dir_findfirst(const char *path, dir_ffblk *fb, int attrib);
int dir_findnext(dir_ffblk *fb);
int dir_getattrib(const dir_ffblk *fb);
const char* dir_getname(const dir_ffblk *fb);
int dir_match(const char *string, const char *pattern, int ignore_case);

#define FindFirst(A, B, C) dir_findfirst((A), &(C), (B))
#define FindNext(A)        dir_findnext(&(A))
#define AttribOf(ff)       dir_getattrib(&(ff))
#define NameOf(ff)         dir_getname(&(ff))

#define _A_NORMAL 0x00

typedef struct GFILE GFILE;
GFILE *file_open(const char *name, const char *mode);
int file_stat(const char *path, struct stat *buf);
int file_fclose(GFILE *f);
int file_fseek(GFILE *f, off_t offset, int whence);
off_t file_ftell(GFILE *f);
ssize_t file_fread(void* buf, size_t len, size_t cnt, GFILE *f);
ssize_t file_fwrite(const void* buf, size_t len, size_t cnt, GFILE *f);

#define _A_NORMAL 0x00

#define fnsplit path_split
#define fnmerge path_merge
#define fnstatat file_stat

void _dev_assert(BOOL cond);

BOOL system_keyhit(void);
int system_getkey(void); /* read a keystroke */
int system_getshift(void); /* read the keyboard shift status */
void system_resetmouse(void); /* reset the mouse */
int system_mousebuttons(void); /* return true if mouse buttons are pressed */
void system_get_mouseposition(int *x, int *y); /* return mouse coordinates */
int system_button_releases(void); /* return true if a mouse button has been released */

#ifndef _MSC_VER
# define stricmp strcasecmp
#endif

/* Save and restore current working directory. */

struct saved_cwd
{
    int desc;
    char *name;
};

int save_cwd (struct saved_cwd *cwd);
int restore_cwd (const struct saved_cwd *cwd);
void free_cwd (struct saved_cwd *cwd);

#endif // TEXTUI_SUPPORT_H
