#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

/* Low-level frambuffer management */

int fb_acquire_fb(uint32_t flags, const char *fb_device, const char *tty_device);
void fb_release_fb(void);

uint32_t fb_screen_width(void);
uint32_t fb_screen_height(void);

uint32_t fb_console_width(void);
uint32_t fb_console_height(void);

void fb_flush_rect(int x, int y, int w, int h);
void fb_flush_window(void);
int fb_flush_fb(void);

/* Intialization and termination */

void alert(const char *msg);
void fatal(const char *msg);

#endif // MAIN_H
