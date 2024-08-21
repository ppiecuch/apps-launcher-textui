#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

/* Low-level frambuffer management */

#ifdef __cplusplus
# define EXTERN_C extern "C"
#else
# define EXTERN_C
#endif

EXTERN_C int fb_acquire_fb(uint32_t flags, const char *fb_device, const char *tty_device);
EXTERN_C void fb_release_fb(void);

EXTERN_C uint32_t fb_screen_width(void);
EXTERN_C uint32_t fb_screen_height(void);

EXTERN_C uint32_t fb_console_width(void);
EXTERN_C uint32_t fb_console_height(void);

EXTERN_C void fb_flush_rect(int x, int y, int w, int h);
EXTERN_C void fb_flush_window(void);
EXTERN_C int fb_flush_fb(void);

/* Intialization and termination */

EXTERN_C void alert(const char *msg);
EXTERN_C void fatal(const char *msg);

#endif // MAIN_H
