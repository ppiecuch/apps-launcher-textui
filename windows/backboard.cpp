// Text plotting:
// --------------
//  https://github.com/rigel314/ttysys
//  http://elmon.sourceforge.net
//  https://github.com/yaronn/blessed-contrib
//  https://github.com/madbence/node-drawille
//  https://github.com/asciimoo/drawille
//  https://github.com/Huulivoide/libdrawille
//  https://github.com/fosu/drawille-plusplus/blob/master/drawille.hpp

#include "deskviews.h"
#include "main.h"
#include "info.h"
#include "backboard.h"
#include "platform/device_info.h"
#include "platform/support.h"
#include "platform/output.h"
#include "platform/time_lapse.h"
#include "chaiscript/chaiscript.h"

#include <stdio.h>
#include <stdint.h>
#include <libgen.h>
#include <assert.h>
#include <time.h>

#include <vector>
#include <string>
#include <thread>
#include <chrono>

#define _S(v) #v
#define STR(x) _S(x)

// https://stackoverflow.com/questions/11796355/how-to-convert-or-cast-a-float-into-its-bit-sequence-such-as-a-long
union _fpconv { float f; PARAM p; };
#define PVALUE(fv) _fpconv{float(fv)}.p

/// String and filename management

#if defined(_WIN32) || defined(WIN32)
# define PATH_SEP "\\"
#else
# define PATH_SEP "/"
#endif

#define TS() ({ time_t now(time(NULL)); char buffer[80]; strftime(buffer, 80, "%m/%d %H:%M", localtime(&now)); buffer; })

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

static std::string human_readable_size(uintmax_t size) {
    constexpr uintmax_t KB = 1024;
    constexpr uintmax_t MB = KB * 1024;
    constexpr uintmax_t GB = MB * 1024;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    if (size >= GB) {
        oss << static_cast<double>(size) / GB << " GB";
    } else if (size >= MB) {
        oss << static_cast<double>(size) / MB << " MB";
    } else if (size >= KB) {
        oss << static_cast<double>(size) / KB << " KB";
    } else {
        oss << size << " bytes";
    }
    return oss.str();
}

/// Logging and errors handling

_output_line_t output_line = nullptr;

void log_output_line(const char *prefix, const char *file, int lineNumber, const char *message) {
    backboard.logMessage(f_ssprintf("%s %s %s:%d %s", TS(), prefix, Filename(file).c_str(), lineNumber, message));
}

#define ERROR_LOG(msg) log_output_line("[ERR] ", __FILE__, __LINE__, msg)
#define INFO_LOG(msg)  log_output_line("[INF] ", __FILE__, __LINE__, msg)
#define TRACE_LOG(msg) log_output_line("[TRC] ", __FILE__, __LINE__, msg)
#define DEBUG_LOG(msg) log_output_line("[DBG] ", __FILE__, __LINE__, msg)

/// Backboard implementation

BackboardWindow backboard;

static int MainAppProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2);

// http://www.alanwood.net/unicode/braille_patterns.html
// dots:
//  ,___,
//  |1 4|
//  |2 5|
//  |3 6|
//  |7 8|
//  `````
//

namespace drawille {
    const uint8_t pixmap[4][2] = {
        {0x01, 0x08},
        {0x02, 0x10},
        {0x04, 0x20},
        {0x40, 0x80}
    };

    // braille unicode characters starts at 0x2800
    const uint32_t braille_char_offset = 0x2800;

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


int const CAPTURED_FRAMES_NUM = 30;  // 30 captures
float const AVG_TIME          = 0.5; // 500 millisecondes
int const TARGET_FRAMERATE    = 30;

class FPSCompute {
    float history[CAPTURED_FRAMES_NUM];
    int indx;
    const float step;
    float average;
    float last;

    using clock_t = typename std::chrono::steady_clock; // high_resolution_clock
    using timep_t = typename clock_t::time_point;
    timep_t start_point, last_frame_time;

public:
    FPSCompute(void)
    : indx(0)
    , step(AVG_TIME / CAPTURED_FRAMES_NUM)
    , average(TARGET_FRAMERATE)
    , last(0) {
        for (int i = 0; i < CAPTURED_FRAMES_NUM; ++i) {
            history[i] = TARGET_FRAMERATE / CAPTURED_FRAMES_NUM;
        }
        start_point = clock_t::now();
    }

    float compute_fps(float delta_time) {
        float const total_time = since(start_point).count();
        float fps_frame = 1 / delta_time;
        if (total_time - last > step) {
            last = total_time;
            ++indx %= CAPTURED_FRAMES_NUM;
            average -= history[indx];
            history[indx] = fps_frame / CAPTURED_FRAMES_NUM;
            average += history[indx];
        }
        return average;
    }

    float get_average_fps(void) { return average; }

    void begin_frame() { last_frame_time = clock_t::now(); }

    void end_frame() {
        timep_t current_time = clock_t::now();
        float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_frame_time).count();

        if (frame_time < 1.0 / TARGET_FRAMERATE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>((1.0 / TARGET_FRAMERATE - frame_time) * 1000)));
        }
    }
}; // fps compute

static int getLogWindowHeight(int totalHeight) {
    return std::max(5., totalHeight * 0.2);
}

bool BackboardWindow::quit() { return quitting; }

void BackboardWindow::resize() {
    if (wnd == 0) {
        con_width = fb_con_width();
        con_height = fb_con_height();

        if (!init_console(con_width, con_height)) {
            fatal("BackboardWindow: Unable to start the console system.");
            return;
        }
        if (!init_messages()) {
            fatal("BackboardWindow: Unable to start the messaging system.");
            return;
        }

        device = buildDeviceInfo();

        int logh = getLogWindowHeight(con_height);

        wnd = CreateWindow(APPLICATION, "Backboard v" STR(APP_VERSION), 0, 0, -1, -1, &MainBackboardMenu, NULL, MainAppProc, HASSTATUSBAR | HASMENUBAR);
        info = CreateWindow(TEXTVIEW, "Events", 0, 0, -1, -1, NULL, wnd, MainAppProc, SHADOW | HASBORDER | VSCROLLBAR);
        log = CreateWindow(TEXTBOX, "Log", 0, 0, -1, -1, NULL, wnd, MainAppProc, HASBORDER | VSCROLLBAR | VISIBLE);

        updateStatusBar("Welcome!");

        recordEvent(f_ssprintf("Running Backboard v%s", STR(APP_FULL_IDENT)));
        recordEvent(f_ssprintf("Detected %s", device->getPlatformDescription()));

        INFO_LOG(f_ssprintf("Build info: %s", BUILDTIME));
        INFO_LOG(f_ssprintf("Running on framebuffer %dx%d pixels.", fb_screen_width(), fb_screen_height()));
        INFO_LOG(f_ssprintf("Resizing console to %dx%d chars.", con_width, con_height));

        instruments = DialogWindow(wnd, &graph, InstrumentsProc); // instruments dialog
        // file manager dialog
        // system info dialog
        // settings dialog

        SendMessage(ControlWindow(&graph, ID_PLOTGRAPH), SETLABEL, 0, PARAM("FPS"));
        SendMessage(ControlWindow(&graph, ID_PLOTGRAPH), SETLABEL, 1, PARAM("CPU"));
        SendMessage(ControlWindow(&graph, ID_PLOTGRAPH), SETLABEL, 2, PARAM("MEM"));

        SendMessage(log, SETFOCUS, TRUE, 0);

        updateLayout();
    }
}

void BackboardWindow::updateStatusBar(const char *msg) {
    const char *battery_level[] = { "\x2b", "\x2c", "\x2d", "\x2e", "\x2f" };
    const char *battery_plug = "\x3f";
    const char *help_status1 = "\x01\x5c\x01\x29\x01\x2a\x01\x5b \x01\x06\x01L\x01O\x01G \x01\x03\xf0";
    const char *hw[] = { "\x3c", "\x3d", "\x3e" }; // cpu, sdcard, mem
    int v;
    std::string cpu; if (device->getSystemLoadPercentage(v)) {
        char dbuf[10], nbuf[10] = { 0, 0, 0, 0, 0, 0, 0 };
        switch (snprintf(dbuf, 10, "%d", v)) {
            case 3: nbuf[4] = '\x01'; nbuf[5] = dbuf[2];
            case 2: nbuf[2] = '\x01'; nbuf[3] = dbuf[1];
            case 1: nbuf[0] = '\x01'; nbuf[1] = dbuf[0];
        }
        cpu = f_ssprintf(" \x01%s%s\x01\x3b", hw[0], nbuf);
    }
    std::string mem; if (device->getMemoryUsagePercentage(v)) {
        char dbuf[10], nbuf[10] = { 0, 0, 0, 0, 0, 0, 0 };
        switch (snprintf(dbuf, 10, "%d", v)) {
            case 3: nbuf[4] = '\x01'; nbuf[5] = dbuf[2];
            case 2: nbuf[2] = '\x01'; nbuf[3] = dbuf[1];
            case 1: nbuf[0] = '\x01'; nbuf[1] = dbuf[0];
        }
        mem = f_ssprintf(" \x01%s%s\x01\x3b", hw[2], nbuf);
    }
    SendMessage(wnd, ADDSTATUS, PARAM(f_ssprintf("%s%s%s\t%s", help_status1, cpu.c_str(), mem.c_str(), msg)), 0);
}

bool BackboardWindow::toggleLog() {
    return (GetAttribute(log) &= ~VISIBLE);
}

void BackboardWindow::updateLayout() {
    int cw = ClientWidth(GetParent(log)), ch = ClientHeight(GetParent(log));
    int logh = getLogWindowHeight(ch);
    printf("%d %d %d \n",cw,ch,logh);
    SendMessage(log, MOVE, 0, ch - logh + TopBorderAdj(GetParent(log)));
    SendMessage(log, SIZE, cw - 1, ch - 1);
    for (HWND &w : std::vector<HWND>{info, instruments}) {
        cw = ClientWidth(GetParent(w)), ch = ClientHeight(GetParent(w));
        SendMessage(w, MOVE, 0, TopBorderAdj(GetParent(w)));
        SendMessage(w, SIZE, cw - BorderAdj(w) * 2, isHidden(log) ? ch : ch - logh);
        SendMessage(w, PAINT, 0, 0);
    }
}

bool BackboardWindow::dispatchMessage() {
    return Cooperate();
}

void BackboardWindow::update() {
    static auto start = std::chrono::steady_clock::now();
    static FPSCompute fps;

    float delta = since(start).count() / 1000.;
    uint32_t ticks = get_tics_from_secs(delta);

    KickProgress(ticks); // 18.2 ticks/sec

    fps.compute_fps(delta);

    fps.begin_frame();

    SendMessage(ControlWindow(&graph, ID_PLOTGRAPH), ADDSAMPLE, 0, PVALUE(int(1/delta)));
    SendMessage(ControlWindow(&graph, ID_PLOTGRAPH), ADDSAMPLEAVG, 0, PVALUE(int(fps.get_average_fps())));

    if (visible) {
        if (dispatchMessage()) { // window has changed
            char_info_t *video = (char_info_t*)get_videomode(); // flush video memory
            for (int row = 0; row < con_height; ++row) {
                for (int col = 0; col < con_width; ++col) {
                    const char_info_t &p = *video;
                    const uint8_t fg = p.fg;
                    const uint8_t bg = p.bg;
                    const uint16_t chr = p.character | (p.font << 8);
                    fb_con_put_char_attrib(col, row, chr, fg, bg);
                    video++;
                }
            }
            fb_flush_window(); // to framebuffer
        }
    }

    start = std::chrono::steady_clock::now();

    fps.end_frame();
}

void BackboardWindow::openCalendar() {
    Calendar(wnd);
}

void BackboardWindow::openSysMessageLog() {
    MessageLog(wnd);
}

void BackboardWindow::logMessage(const char* msg) {
    static std::vector<std::string> _queue;
    if (log) {
        while (!_queue.empty())
            SendMessage(log, ADDTEXT, PARAM(take(_queue).c_str()), 0);
        SendMessage(log, ADDTEXT, PARAM(msg), 0);
        SendMessage(log, PAINT, 0, 0);
    } else
        _queue.insert(_queue.begin(), msg);
}

void BackboardWindow::recordEvent(const char *msg) {
    static std::vector<std::string> _queue;
    if (info) {
        while (!_queue.empty())
            SendMessage(log, ADDTEXT, PARAM(take(_queue).c_str()), 0);
        const char *line = f_ssprintf("\x09%s", msg);
        SendMessage(info, ADDTEXT, PARAM(line), 0);
        SendMessage(info, PAINT, 0, 0);
    } else
        _queue.insert(_queue.begin(), msg);
}

/* ------- window processing module for application window ----- */
static int MainAppProc(WINDOW wnd,MESSAGE msg,PARAM p1,PARAM p2)
{
    switch (msg) {
        case CREATE_WINDOW:
            break;
        case KEYBOARD:
            break;
    }
    
    return DefaultWndProc(wnd, msg, p1, p2);
}

void PrepBackboardFileMenu(void *, struct Menu *) {
}

void PrepBackboardEditMenu(void *, struct Menu *) {
}

BackboardWindow::BackboardWindow() {
    wnd = nullptr;
    info = log = nullptr;
    visible = true, quitting = false;

    SelectColorScheme (color);        /* Color Scheme */
    output_line = log_output_line;    /* Register custom logger */
}

BackboardWindow::~BackboardWindow() {
}
