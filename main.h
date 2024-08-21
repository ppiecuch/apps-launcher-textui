#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#ifdef __cplusplus
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif

/* Low-level frambuffer management */

EXTERN_C int fb_acquire_fb(uint32_t flags, const char *fb_device, const char *tty_device);
EXTERN_C void fb_release_fb(void);

EXTERN_C uint32_t fb_screen_width(void);
EXTERN_C uint32_t fb_screen_height(void);

EXTERN_C uint32_t fb_con_width(void);
EXTERN_C uint32_t fb_con_height(void);

EXTERN_C void fb_flush_rect(int x, int y, int w, int h);
EXTERN_C void fb_flush_window(void);
EXTERN_C int fb_flush_fb(void);

/* Keyboard support */

/**
 * Non-blocking input mode
 * When passed to tfb_set_kb_raw_mode(), this flag makes the TTY input to be non-blocking.
 */
#define TFB_FL_KB_NONBLOCK (1 << 2)

typedef uint64_t tfb_key_t; /* Library's type used to represent keystrokes */

EXTERN_C int tfb_set_kb_raw_mode(uint32_t flags);
EXTERN_C int tfb_restore_kb_mode(void);
EXTERN_C tfb_key_t tfb_read_keypress(void);
EXTERN_C int tfb_get_fn_key_num(tfb_key_t k);

#define TFB_KEY_ENTER   ((tfb_key_t)10)
#define TFB_KEY_UP      (*(tfb_key_t*)("\e[A\0\0\0\0\0"))
#define TFB_KEY_DOWN    (*(tfb_key_t*)("\e[B\0\0\0\0\0"))
#define TFB_KEY_RIGHT   (*(tfb_key_t*)("\e[C\0\0\0\0\0"))
#define TFB_KEY_LEFT    (*(tfb_key_t*)("\e[D\0\0\0\0\0"))
#define TFB_KEY_INS     (*(tfb_key_t*)("\e[2~\0\0\0\0\0"))
#define TFB_KEY_DEL     (*(tfb_key_t*)("\e[3~\0\0\0\0\0"))
#define TFB_KEY_HOME    (*(tfb_key_t*)("\e[1~\0\0\0\0\0"))
#define TFB_KEY_END     (*(tfb_key_t*)("\e[4~\0\0\0\0\0"))

#define TFB_KEY_F1      (tfb_int_fn_key_sequences[0])
#define TFB_KEY_F2      (tfb_int_fn_key_sequences[1])
#define TFB_KEY_F3      (tfb_int_fn_key_sequences[2])
#define TFB_KEY_F4      (tfb_int_fn_key_sequences[3])
#define TFB_KEY_F5      (tfb_int_fn_key_sequences[4])
#define TFB_KEY_F6      (tfb_int_fn_key_sequences[5])
#define TFB_KEY_F7      (tfb_int_fn_key_sequences[6])
#define TFB_KEY_F8      (tfb_int_fn_key_sequences[7])
#define TFB_KEY_F9      (tfb_int_fn_key_sequences[8])
#define TFB_KEY_F10     (tfb_int_fn_key_sequences[9])
#define TFB_KEY_F11     (tfb_int_fn_key_sequences[10])
#define TFB_KEY_F12     (tfb_int_fn_key_sequences[11])

/* Intialization and termination */

EXTERN_C void alert(const char *msg);
EXTERN_C void fatal(const char *msg);

#endif // MAIN_H
