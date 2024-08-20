
#include "text_ui/textUI.h"

#include "backboard.h"
#include "instruments.h"

/* ------------ Instruments dialog box -------------- */

DIALOGBOX(dbGraph)
DB_TITLE("Instruments", 44, 2, kInstrumentsWinH, kInstrumentsWinW)
CONTROL(LCDBOX,   NULL,   1,   1,  3, kInstrumentsWinW-3, ID_LCDLABEL)
CONTROL(GRAPHBOX, NULL,   0,   4,  5, kInstrumentsWinW-2, ID_PLOTGRAPH)
ENDDB
