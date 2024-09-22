#include "text_ui/textUI.h"

#include "backboard.h"
#include "deskviews.h"


DIALOGBOX(sysInfo)
    DB_TITLE("SysInfo", 0,0,-1,-1)
    CONTROL(LISTBOX, NULL,           0, 0,-1,-1, ID_LIST2COLS)
ENDDB
