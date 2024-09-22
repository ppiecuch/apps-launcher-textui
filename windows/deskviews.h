#ifndef INSTRUMENTS_H
#define INSTRUMENTS_H

#include "text_ui/textUI.h"

#ifdef __cplusplus
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif

#define kInstrumentsWinW 35
#define kInstrumentsWinH 10

#define kLogWinH 17

enum {
    ID_LCDLABEL=0xe000,
    ID_PLOTGRAPH
};

extern DBOX graph;

EXTERN_C int InstrumentsProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2);

#endif // INSTRUMENTS_H

