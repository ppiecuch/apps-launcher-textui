// Text plotting:
// --------------
//  https://github.com/rigel314/ttysys
//  http://elmon.sourceforge.net
//  https://github.com/yaronn/blessed-contrib
//  https://github.com/madbence/node-drawille
//  https://github.com/asciimoo/drawille
//  https://github.com/Huulivoide/libdrawille
//  https://github.com/fosu/drawille-plusplus/blob/master/drawille.hpp

#include "backboard.h"

#include <stdio.h>
#include <stdint.h>
#include <vector>

BackboardWindow backboard;

#define kLogWinH 17

static int MainAppProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2);
static int InstrumentsProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2);

static void log_output_line(const char *prefix, const char *file, int lineNumber, const char *message) {
    const int _max_ln = 1024; char s[_max_ln] = { 0 };

    snprintf(s, _max_ln, "%s %s:%d %s", prefix, Filename(file).c_str(), lineNumber, message);

    backboard.logMessage(s);
}

enum {
    ID_LCDLABEL=0xe000,
    ID_PLOTGRAPH
};


/* ------------ Instruments dialog box -------------- */

namespace drawille {
  using std::vector;

  const uint8_t pixmap[4][2] = {
    {0x01, 0x08},
    {0x02, 0x10},
    {0x04, 0x20},
    {0x40, 0x80}
  };

  class Canvas {
  public:
    Canvas(size_t width, size_t height) {
      this->canvas.resize(height);
      for(auto& v: this->canvas)
        v.resize(width);
    }

    void set(size_t x, size_t y) {
      if(x > (this->canvas[0].size() * 2) or x < 1) x = 0;
      if(y > (this->canvas.size() * 4) or y < 1)    y = 0;
      this->canvas[y / 4][x / 2] |= pixmap[y % 4][x % 2];
    }

    void unset(size_t x, size_t y) {
      if(x > (this->canvas[0].size() * 2) or x < 1) x = 0;
      if(y > (this->canvas.size() * 4) or y < 1)    y = 0;
      this->canvas[y / 4][x / 2] &= ~pixmap[y % 4][x % 2];
    }

  protected:
    std::vector<std::vector<uint8_t> > canvas;
  };
} // drawille

void BackboardWindow::resize(int w, int h) {
    if (wnd == 0) {

        assert(w>0);
        assert(h>0);

        uint32_t fnt = uiLoadFont(DOS_7x9); // return size of the font console

        fnt_width = _LODWORD(fnt);
        fnt_height = _HIDWORD(fnt);
        DebugLog("Console font size %dx%d px.", fnt_width, fnt_height);

        con_width = w/fnt_width;
        con_height = h/fnt_height;
        DebugLog("Resizing console to %dx%d chars.", con_width, con_height);

        if (!init_console(con_width, con_height)) {
            ErrorLog ("BackboardWindow: Unable to start the console system.");
            return;
        }
        if (!init_messages()) {
            ErrorLog ("BackboardWindow: Unable to start the messaging system.");
            return;
        }
        wnd = CreateWindow(APPLICATION, "Backboard", 0, 0, -1, -1, NULL, NULL, MainAppProc, HASSTATUSBAR);
        info = CreateWindow(TEXTVIEW, "GL Info", 2, 2, 15, 40, NULL, wnd, MainAppProc,
                            SHADOW | MOVEABLE | HASBORDER | MINMAXBOX | VSCROLLBAR | VISIBLE);
        log = CreateWindow(TEXTBOX, "Log", 0, con_height-kLogWinH-1, kLogWinH, con_width-1, NULL, wnd, MainAppProc,
                           SHADOW | MOVEABLE | HASBORDER | MINMAXBOX | VSCROLLBAR | VISIBLE);

        DialogBox(wnd, &dbGraph, NO, InstrumentsProc);
        //openCalendar();

        // add and refresh info:
        char *msg = f_ssprintf("%s; %s", (char *)glGetString(GL_VERSION), (char *)glGetString(GL_EXTENSIONS));
        SendMessage(info, ADDTEXT, PARAM(msg), 0);
        SendMessage(info, PAINT, 0, 0);

        // WINDOW cwnd = ControlWindow(&dbGraph, ID_LCDLABEL);
        // SendMessage(cwnd, PAINT, 0, 0);
        // WINDOW pwnd = ControlWindow(&dbGraph, ID_PLOTGRAPH);
        // SendMessage(pwnd, PAINT, 0, 0);

        SendMessage(log, SETFOCUS, TRUE, 0);
    }
}

void BackboardWindow::dispatch_message() {
    if (visible)
        Cooperate();
}

void BackboardWindow::update() {
    dispatch_message();
    if (visible) {
        uint16_t *video = ((uint16_t*)get_videomode(); // flush video memory
        fb_flush_window()                              // to framebuffer
    }
}

void BackboardWindow::openCalendar() {
    Calendar(wnd);
}

void BackboardWindow::openSysMessageLog() {
    MessageLog(wnd);
}

bool BackboardWindow::logMessage(const char* msg) {
    if (log) {
        SendMessage(log, ADDTEXT, PARAM(msg), 0);
        SendMessage(log, PAINT, 0, 0);
        return true;
    } else
        return false;
}

/* ------- window processing module for application window ----- */
static int MainAppProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
    if ( msg == CREATE_WINDOW )
    {
    }
    
    return DefaultWndProc(wnd, msg, p1, p2);
}

static int InstrumentsProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
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
    }

    return DefaultWndProc(wnd, msg, p1, p2);
}

BackboardWindow::BackboardWindow() : wnd(0), info(0), visible(true) {
    SelectColorScheme (color);        /* Color Scheme */
    CustomizeDisplayPropBox ();       /* Start other services */
    output_line = log_output_line;    /* Register custom logger */
}

BackboardWindow::~BackboardWindow() {
}
