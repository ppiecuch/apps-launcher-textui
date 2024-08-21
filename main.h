#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#ifdef __cplusplus
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif

/* Low-level frambuffer management */

enum {
/**
    * Do NOT put TTY in graphics mode.
    *
    * Passing this flag to tfb_acquire_fb() will
    * allow to use the framebuffer and to see stdout on TTY as well. That usually
    * is undesirable because the text written to TTY will overwrite the graphics.
    */
    FB_FL_NO_TTY_KD_GRAPHICS = (1 << 0),

    /**
    * Do NOT write directly onto the framebuffer.
    *
    * Passing this flag to tfb_acquire_fb() will make it allocate a regular memory
    * buffer where all the writes (while drawing) will be directed to. The changes
    * will appear on-screen only after manually called tfb_flush_rect() or
    * tfb_flush_rect(). This flag is useful for applications needing to clean and
    * redraw the whole screen (or part of it) very often (e.g. games) in order to
    * avoid the annoying flicker effect.
    */
    FB_FL_USE_DOUBLE_BUFFER = (1 << 1),
};

EXTERN_C int fb_acquire_fb(uint32_t flags, const char *fb_device, const char *tty_device);
EXTERN_C void fb_release_fb(void);

EXTERN_C uint32_t fb_screen_width(void);
EXTERN_C uint32_t fb_screen_height(void);

EXTERN_C uint32_t fb_con_width(void);
EXTERN_C uint32_t fb_con_height(void);

EXTERN_C void fb_con_put_char(int x, int y, uint32_t ch);
EXTERN_C void fb_con_draw_line(int x1, int y1, int x2, int y2, uint32_t ch);
EXTERN_C void fb_con_draw_box(int x, int y, int w, int h);

EXTERN_C void fb_flush_rect(int x, int y, int w, int h);
EXTERN_C void fb_flush_window(void);
EXTERN_C int fb_flush_fb(void);

/* Keyboard support */

/**
 * Non-blocking input mode
 * When passed to fb_set_kb_raw_mode(), this flag makes the TTY input to be non-blocking.
 */
#define FB_FL_KB_NONBLOCK (1 << 2)

typedef uint64_t fb_key_t; /* Library's type used to represent keystrokes */

EXTERN_C int fb_set_kb_raw_mode(uint32_t flags);
EXTERN_C int fb_restore_kb_mode(void);
EXTERN_C fb_key_t fb_read_keypress(void);
EXTERN_C int fb_get_fn_key_num(fb_key_t k);

#define FB_KEY_NONE    ((fb_key_t)0)
#define FB_KEY_ENTER   ((fb_key_t)10)
#define FB_KEY_UP      (*(fb_key_t*)("\e[A\0\0\0\0\0"))
#define FB_KEY_DOWN    (*(fb_key_t*)("\e[B\0\0\0\0\0"))
#define FB_KEY_RIGHT   (*(fb_key_t*)("\e[C\0\0\0\0\0"))
#define FB_KEY_LEFT    (*(fb_key_t*)("\e[D\0\0\0\0\0"))
#define FB_KEY_INS     (*(fb_key_t*)("\e[2~\0\0\0\0\0"))
#define FB_KEY_DEL     (*(fb_key_t*)("\e[3~\0\0\0\0\0"))
#define FB_KEY_HOME    (*(fb_key_t*)("\e[1~\0\0\0\0\0"))
#define FB_KEY_END     (*(fb_key_t*)("\e[4~\0\0\0\0\0"))

#define FB_KEY_F1      (fb_int_fn_key_sequences[0])
#define FB_KEY_F2      (fb_int_fn_key_sequences[1])
#define FB_KEY_F3      (fb_int_fn_key_sequences[2])
#define FB_KEY_F4      (fb_int_fn_key_sequences[3])
#define FB_KEY_F5      (fb_int_fn_key_sequences[4])
#define FB_KEY_F6      (fb_int_fn_key_sequences[5])
#define FB_KEY_F7      (fb_int_fn_key_sequences[6])
#define FB_KEY_F8      (fb_int_fn_key_sequences[7])
#define FB_KEY_F9      (fb_int_fn_key_sequences[8])
#define FB_KEY_F10     (fb_int_fn_key_sequences[9])
#define FB_KEY_F11     (fb_int_fn_key_sequences[10])
#define FB_KEY_F12     (fb_int_fn_key_sequences[11])

extern fb_key_t *fb_int_fn_key_sequences;

/* Intialization and termination */

EXTERN_C void alert(const char *msg);
EXTERN_C void fatal(const char *msg);

#endif // MAIN_H
