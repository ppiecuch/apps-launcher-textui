#ifndef INSTRUMENTS_H
#define INSTRUMENTS_H

#include "text_ui/textUI.h"

#ifdef __cplusplus
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif

/// Views and dialogs

enum {
    ID_LCDLABEL=0xe000,
    ID_PLOTGRAPH,
    ID_LIST2COLS,
};

extern DBOX graph;
EXTERN_C int InstrumentsProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2);


extern DBOX fileMgr;
EXTERN_C int FileMgrProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2);

extern DBOX sysInfo;
EXTERN_C int SysInfoProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2);

/// Menus

extern MBAR MainBackboardMenu;

enum {
    ID_NEW,
};

EXTERN_C void PrepBackboardFileMenu(void *, struct Menu *);
EXTERN_C void PrepBackboardEditMenu(void *, struct Menu *);
EXTERN_C void PrepBackboardSearchMenu(void *, struct Menu *);
EXTERN_C void PrepBackboardWindowMenu(void *, struct Menu *);

#endif // INSTRUMENTS_H

