#ifndef MAIN_H
#define MAIN_H

/* Low-level frambuffer management */

int fb_acquire_fb(uint32_t flags, const char *fb_device, const char *tty_device);
void fb_release_fb(void);

void fb_flush_rect(int x, int y, int w, int h);
void fb_flush_window(void);
int fb_flush_fb(void);

#endif // MAIN_H
