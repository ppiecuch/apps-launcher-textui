#ifndef BACKBOARD_H
#define BACKBOARD_H

#include "text_ui/textUI.h"
#include "deskviews.h"

#ifdef __cplusplus

class DeviceInfo;

class BackboardWindow
{
    WINDOW wnd, info, log, instruments, settings; // windows

    int fnt_width, fnt_height;
    int con_width, con_height;
    bool visible, quitting;

    DeviceInfo *device;

public:
    void resize();
    void update();

    bool quit();

    void openCalendar();

    void openSysMessageLog();
    void startCaptureEvents();
    void endCaptureEvents();
    
    void logMessage(const char* msg);
    void recordEvent(const char *msg);

    void updateStatusBar(const char *msg);
    void updateLayout();
    bool toggleLog();

    bool dispatchMessage();

    void toggleVisible() { visible = !visible; }
    bool isVisible() const { return visible; }

    BackboardWindow();
    ~BackboardWindow();
};

extern BackboardWindow backboard;

#endif // __cplusplus

typedef void(*_output_line_t)(const char *prefix, const char *file, int lineNumber, const char *message);
extern _output_line_t output_line; // custom logger procedure
void log_output_line(const char *prefix, const char *file, int lineNumber, const char *message); //default logger

#endif // BACKBOARD_H
