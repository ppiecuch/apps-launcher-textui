// kate: replace-tabs on; tab-indents on; tab-width 4; indent-width 4; indent-mode cstyle;

/* ----------- textUI_Classes.h ------------ */
/*
 *         Class definition source file
 *         Make class changes to this source file
 *         Other source files will adapt
 *
 *         You should update LOGGER.C so that logs
 *         can have the appropriate class texts.
 *
 *         You must add entries to the color tables in
 *         CONFIG.C for new classes.
 *
 *        Class Name  Base Class   Processor       Attribute    
 *       ------------  --------- ---------------  -----------
 */
ClassDef(  NORMAL,      -1,      NormalProc,      0 )
ClassDef(  APPLICATION, NORMAL,  ApplicationProc, VISIBLE   |
                                                  SAVESELF  |
                                                  CONTROLBOX )
ClassDef(  TEXTBOX,     NORMAL,  TextBoxProc,     0          )
ClassDef(  TEXTVIEW,    TEXTBOX, TextViewProc,    0          )
ClassDef(  LISTBOX,     TEXTBOX, ListBoxProc,     0          )

ClassDef(  EDITBOX,     TEXTBOX, EditBoxProc,     0          )

ClassDef(  MENUBAR,     NORMAL,  MenuBarProc,     NOCLIP     )
ClassDef(  POPDOWNMENU, LISTBOX, PopDownProc,     SAVESELF  |
                                                  NOCLIP    |
                                                  HASBORDER  )
ClassDef(  PICTUREBOX,  TEXTBOX, PictureProc,     0          )
ClassDef(  DIALOG,      NORMAL,  DialogProc,      SHADOW    |
                                                  MOVEABLE  |
                                                  CONTROLBOX|
                                                  HASBORDER |
                                                  NOCLIP     )
ClassDef(  BOX,         NORMAL,  BoxProc,         HASBORDER  )
ClassDef(  BUTTON,      TEXTBOX, ButtonProc,      SHADOW     )
ClassDef(  COMBOBOX,    EDITBOX, ComboProc,       0          )
ClassDef(  TEXT,        TEXTBOX, TextProc,        0          )
ClassDef(  RADIOBUTTON, TEXTBOX, RadioButtonProc, 0          )
ClassDef(  CHECKBOX,    TEXTBOX, CheckBoxProc,    0          )
ClassDef(  SPINBUTTON,  LISTBOX, SpinButtonProc,  0          )
ClassDef(  ERRORBOX,    DIALOG,  NULL,            SHADOW    |
                                                  HASBORDER  )
ClassDef(  MESSAGEBOX,  DIALOG,  NULL,            SHADOW    |
                                                  HASBORDER  )
ClassDef(  HELPBOX,     DIALOG,  HelpBoxProc,     MOVEABLE  |
                                                  SAVESELF  |
                                                  HASBORDER |
                                                  NOCLIP    |
                                                  CONTROLBOX )
ClassDef(  STATUSBAR,   TEXTBOX, StatusBarProc,   NOCLIP     )
ClassDef(  EDITOR,      EDITBOX, EditorProc,      0          )

/*
 *  ========> Add new classes here <========
 */

ClassDef(  GRAPHBOX,    NORMAL,  GraphBoxProc,     0          )
ClassDef(  LCDBOX,      TEXTBOX, LcdBoxProc,       0          )

/* ---------- pseudo classes to create enums, etc. ---------- */
ClassDef(  TITLEBAR,    -1,      NULL,            0          )
ClassDef(  DUMMY,       -1,      NULL,            HASBORDER  )

