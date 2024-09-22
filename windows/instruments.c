
#include "text_ui/textUI.h"

#include "backboard.h"
#include "deskviews.h"

/* ------------ Instruments dialog box -------------- */

DIALOGBOX(graph)
DB_TITLE("Instruments",   0, 0, -1, -1)
CONTROL(LCDBOX,   NULL,   0, 0,  3, -1, ID_LCDLABEL)
CONTROL(GRAPHBOX, NULL,   0, 4,  5, -1, ID_PLOTGRAPH)
ENDDB

int InstrumentsProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
    WINDOW ct;
    switch (msg) {
        case INITIATE_DIALOG:
            DBOX *db = (DBOX*)wnd->extension;
            ct = ControlWindow(db, ID_LCDLABEL);
            if (ct) {
                SendMessage(ct, SETTEXT, (PARAM)"00 00 00 00", 0);
                SendMessage(ct, SHOW_WINDOW, 0, 0);
            }
            ct = ControlWindow(db, ID_PLOTGRAPH);
            if (ct) {
                SendMessage(ct, SHOW_WINDOW, 0, 0);
            }
            break;
        case SIZE:
            break;
    }

    return DefaultWndProc(wnd, msg, p1, p2);
}
