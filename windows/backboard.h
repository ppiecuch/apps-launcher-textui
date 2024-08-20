
#include "text_ui/textUI.h"
#include "instruments.h"

# define f_ssprintf(...)                                \
    ({ int _ss_size = snprintf(0, 0, ##__VA_ARGS__);    \
    char *_ss_ret = (char*)alloca(_ss_size+1);          \
    snprintf(_ss_ret, _ss_size+1, ##__VA_ARGS__);       \
    _ss_ret; })

enum {
    ID_LCDLABEL=0xe000,
    ID_PLOTGRAPH
};

class BackboardWindow
{
    WINDOW wnd, info, log;
    DBOX graph;
    int fnt_width, fnt_height;
    int con_width, con_height;
    bool visible;

    enum LogMsgType {
        LOG_MSG_INFO,
        LOG_MSG_DEBUG,
        LOG_MSG_WARNING,
        LOG_MSG_ERROR
    };

public:
    void resize();
    void update();

    void openCalendar();
    void openSysMessageLog();
    void startCaptureEvents();
    void endCaptureEvents();
    
    bool logMessage(const char* msg);
    bool logMessage(LogMsgType msg_type, const char* msg);

    void dispatch_message();

    void toggleVisible() { visible = !visible; }
    bool isVisible() const { return visible; }

    BackboardWindow();
    ~BackboardWindow();
};

typedef void(*_output_line_t)(const char *prefix, const char *file, int lineNumber, const char *message);

extern _output_line_t output_line; // custom logger procedure
extern BackboardWindow backboard;
