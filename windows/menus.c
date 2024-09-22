#include "text_ui/textUI.h"

#include "deskviews.h"

DEFMENU(MainBackboardMenu)
/* --------------- the File popdown menu ----------------*/
POPDOWN("~File", PrepBackboardFileMenu, "Commands for manipulating files")
SELECTION("~New", ID_NEW, CTRL_N, 0) /* 0.7a */
ENDPOPDOWN

/* --------------- the Edit popdown menu ----------------*/
POPDOWN("~Edit", PrepBackboardEditMenu, "Commands for editing files")
SELECTION("Cu~t", ID_CUT, CTRL_X, INACTIVE)
SELECTION("~Copy", ID_COPY, CTRL_C, INACTIVE)
ENDPOPDOWN

/* --------------- the Log popdown menu ----------------*/
POPDOWN("~Log", PrepBackboardEditMenu, "Log access")
SELECTION("Cu~t", ID_CUT, CTRL_X, INACTIVE)
SELECTION("~Copy", ID_COPY, CTRL_C, INACTIVE)
ENDPOPDOWN

/* --------------- the System popdown menu ----------------*/
POPDOWN("~System", PrepBackboardEditMenu, "System information")
SELECTION("Cu~t", ID_CUT, CTRL_X, INACTIVE)
SELECTION("~Copy", ID_COPY, CTRL_C, INACTIVE)
ENDPOPDOWN
ENDMENU
