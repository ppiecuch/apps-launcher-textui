// Text plotting:
// --------------
//  https://github.com/rigel314/ttysys
//  http://elmon.sourceforge.net
//  https://github.com/yaronn/blessed-contrib
//  https://github.com/madbence/node-drawille
//  https://github.com/asciimoo/drawille
//  https://github.com/Huulivoide/libdrawille
//  https://github.com/fosu/drawille-plusplus/blob/master/drawille.hpp

#include "main.h"
#include "backboard.h"
#include "platform/support.h"

#include <stdio.h>
#include <stdint.h>
#include <libgen.h>
#include <assert.h>

#include <vector>
#include <string>

#define _S(v) #v

_output_line_t output_line = nullptr;

void log_output_line(const char *prefix, const char *file, int lineNumber, const char *message) {
    backboard.logMessage(f_ssprintf("%s %s:%d %s", prefix, Filename(file).c_str(), lineNumber, message));
}

/// String and filename management

#if defined(_WIN32) || defined(WIN32)
# define PATH_SEP "\\"
#else
# define PATH_SEP "/"
#endif

// Get the basename of a filename (ie the filename with the extension).
inline std::string Filename (const std::string path, const char *sep = PATH_SEP) {
    size_t slash = path.find_last_of (sep);
    if (slash == std::string::npos)
        return path;
    return path.substr (slash+1);
}

// Get the filename without extension.
inline std::string Basename (const std::string path, const char *sep = PATH_SEP) {
    size_t point = path.find_last_of (".");
    size_t slash = path.find_last_of (sep);

    if (point == std::string::npos && slash == std::string::npos)
        return path;

    if (point == std::string::npos)
        return path.substr (slash+1);

    if (slash == std::string::npos)
        return path.substr (0, point);

    if (slash < point)
        return path.substr (slash+1, point - (slash+1));
    else
        return path.substr (slash+1);
}

/// Blackboard implementation

BackboardWindow backboard;

#define kLogWinH 17

static int MainAppProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2);
static int InstrumentsProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2);

/* ------------ Instruments dialog box -------------- */

namespace drawille {
    const uint8_t pixmap[4][2] = {
        {0x01, 0x08},
        {0x02, 0x10},
        {0x04, 0x20},
        {0x40, 0x80}
    };

    template<typename T> class array2d
    {
        size_t w, h;
        T* d;

        size_t checkedIndex(size_t i) const { assert(i<size()); return i; }
        size_t checkedIndex(size_t i, size_t j) const { size_t s = w*i+j; assert(s<size()); return s; }

    public:
        typedef T value_type;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;

        inline size_t width() const { return w; }
        inline size_t height() const { return h; }
        inline size_t size() const { return w * h; }
        inline size_t bytes() const { return size() * sizeof(value_type); }

        void fill(value_type val) { std::uninitialized_fill_n(d, size(), val); }

        void swap(array2d<T>& a)
        {
            std::swap(w, a.w);
            std::swap(h, a.h);
            std::swap(d, a.d);
        }

        void resize(size_t width, size_t height)
        {
            size_t nelements = width*height;
            assert(nelements>0);

            if(d!=0)
            {
                if (width==w && height==h) return;
                delete[] d;
                d = 0;
            }

            d = new value_type[nelements];
            w = width;
            h = height;
        }

        inline const_iterator begin() const { return d; }
        inline const_iterator end() const { return d + size(); }
        inline iterator begin() { return d; }
        inline iterator end() { return d + size(); }
        inline const T& operator () (size_t i) const { return d[checkedIndex(i)]; }
        inline const T& operator () (size_t i, size_t j) const { return d[checkedIndex(i,j)]; }
        inline T& operator () (size_t i) { return d[checkedIndex(i)]; }
        inline T& operator () (size_t i, size_t j) { return d[checkedIndex(i,j)]; }
        inline const T* c_data() const { return d; }
        inline T* c_data() { return d; }

        array2d(size_t width=1, size_t height=1) : w(0), h(0), d(0) { resize(width, height); }
        array2d(const array2d<T>& a) { resize(a.width(), a.height()); std::copy(a.begin(), a.end(), d); }
        array2d& operator = (const array2d<T>& a) { resize(a.width(), a.height()); std::copy(a.begin(), a.end(), d); return *this; }
        ~array2d() { delete[] d; }
    };

    class Canvas {
        array2d<uint8_t> canvas;

    public:
        void set(size_t x, size_t y) {
            if (x > (canvas.width() * 2) or x < 1) return;
            if (y > (canvas.height() * 4) or y < 1)    return;
            canvas(x / 2, y / 4) |= pixmap[y % 4][x % 2];
        }

        void unset(size_t x, size_t y) {
            if (x > (canvas.width() * 2) or x < 1) return;
            if (y > (canvas.height() * 4) or y < 1)    return;
            canvas(x / 2, y / 4) &= ~pixmap[y % 4][x % 2];
        }

        Canvas(size_t width, size_t height) : canvas(width / 2, height / 4) { }
    };
} // drawille

void BackboardWindow::resize() {
    if (wnd == 0) {
        con_width = fb_con_width();
        con_height = fb_con_height();
        logMessage(f_ssprintf("Resizing console to %dx%d chars.", con_width, con_height));

        if (!init_console(con_width, con_height)) {
            fatal("BackboardWindow: Unable to start the console system.");
            return;
        }
        if (!init_messages()) {
            fatal("BackboardWindow: Unable to start the messaging system.");
            return;
        }

        wnd = CreateWindow(APPLICATION, "Backboard", 0, 0, -1, -1, NULL, NULL, MainAppProc, HASSTATUSBAR);
        info = CreateWindow(TEXTVIEW, "GL Info", 2, 2, 15, 40, NULL, wnd, MainAppProc, SHADOW | MOVEABLE | HASBORDER | MINMAXBOX | VSCROLLBAR | VISIBLE);
        log = CreateWindow(TEXTBOX, "Log", 0, con_height-kLogWinH-1, kLogWinH, con_width-1, NULL, wnd, MainAppProc, SHADOW | MOVEABLE | HASBORDER | MINMAXBOX | VSCROLLBAR | VISIBLE);

        DialogBox(wnd, &graph, NO, InstrumentsProc);
        //openCalendar();

        // add and refresh info:
        const char *msg = f_ssprintf("Running Blackbord %s", _S(APP_VERSION));
        SendMessage(info, ADDTEXT, PARAM(msg), 0);
        SendMessage(info, PAINT, 0, 0);

        // WINDOW cwnd = ControlWindow(&graph, ID_LCDLABEL);
        // SendMessage(cwnd, PAINT, 0, 0);
        // WINDOW pwnd = ControlWindow(&graph, ID_PLOTGRAPH);
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
        char_info_t *video = (char_info_t*)get_videomode(); // flush video memory
        fb_flush_window();                             // to framebuffer
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
    output_line = log_output_line;    /* Register custom logger */
}

BackboardWindow::~BackboardWindow() {
}
