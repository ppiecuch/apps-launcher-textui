#include "text_ui/textUI.h"

#include "backboard.h"
#include "deskviews.h"

DIALOGBOX(fileMgr)
    DB_TITLE("Files", 0,0,-1,-1)
    CONTROL(TEXT,    "File ~Name:",   3, 1, 1,10, ID_FILENAME)
    CONTROL(EDITBOX, NULL,           14, 1, 1,40, ID_FILENAME)
    CONTROL(TEXT,    NULL,            3, 3, 1,50, ID_PATH)
    CONTROL(TEXT,    "~Files:",       3, 5, 1, 6, ID_FILES)
    CONTROL(LISTBOX, NULL,            3, 6,10,14, ID_FILES)
    CONTROL(TEXT,    "~Directories:",19, 5, 1,12, ID_DIRECTORY)
    CONTROL(LISTBOX, NULL,           19, 6,10,13, ID_DIRECTORY)
    CONTROL(TEXT,    "Dri~ves:",     34, 5, 1, 7, ID_DRIVE)
    CONTROL(LISTBOX, NULL,           34, 6,10,10, ID_DRIVE)
ENDDB
