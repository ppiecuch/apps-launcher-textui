// Reference:
// ----------
//  - http://www.seasip.info/Unix/PSF/Amstrad/Setfont/index.html
//  - https://stackoverflow.com/questions/59922972/how-to-stop-echo-in-terminal-using-c

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <numeric>

#include "graphics/psf.h"
#include "graphics/cp437.h"

#include "windows/backboard.h"

#include "main.h"

#define DEFAULT_FB_DEVICE "/dev/fb0"
#define DEFAULT_TTY_DEVICE "/dev/tty"

#define ARRAY_SIZE(a) ((int)(sizeof(a)/sizeof(a[0])))
#define INT_ABS(x) ((x) > 0 ? (x) : (-(x)))

#define MIN(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x <= _y ? _x : _y; })

#define MAX(x, y) \
   ({ __typeof__ (x) _x = (x); \
      __typeof__ (y) _y = (y); \
      _x > _y ? _x : _y; })


// Set 'n' 32-bit elems pointed by 's' to 'val'.
static inline void *memset32(void *s, uint32_t val, size_t n) {
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
   __asm__ volatile ("rep stosl"
                     : "=D" (s), "=a" (val), "=c" (n)
                     :  "D" (s), "a" (val), "c" (n)
                     : "cc", "memory");
#else
   for (size_t i = 0; i < n; i++)
      ((volatile uint32_t *)s)[i] = val;
#endif
   return s;
}

struct fb_var_screeninfo __fbi;
int __fb_ttyfd = -1;

static int fbfd = -1;

/* Essential variables */
void *__fb_buffer;
void *__fb_real_buffer;
int __fb_screen_w;
int __fb_screen_h;
size_t __fb_size;
size_t __fb_pitch;
size_t __fb_pitch_div4; /* see the comment in drawing.c */

/* Window-related variables */
int __fb_win_w;
int __fb_win_h;
int __fb_off_x;
int __fb_off_y;
int __fb_win_end_x;
int __fb_win_end_y;

/* Color-related variables */
uint32_t __fb_r_mask;
uint32_t __fb_g_mask;
uint32_t __fb_b_mask;
uint8_t __fb_r_mask_size;
uint8_t __fb_g_mask_size;
uint8_t __fb_b_mask_size;
uint8_t __fb_r_pos;
uint8_t __fb_g_pos;
uint8_t __fb_b_pos;

/* Predefined colors */
uint32_t fb_lightred;
uint32_t fb_red;
uint32_t fb_darkred;
uint32_t fb_pink;
uint32_t fb_deeppink;
uint32_t fb_orange;
uint32_t fb_darkorange;
uint32_t fb_gold;
uint32_t fb_yellow;
uint32_t fb_violet;
uint32_t fb_lightmagenta;
uint32_t fb_magenta;
uint32_t fb_darkviolet;
uint32_t fb_indigo;
uint32_t fb_lightgreen;
uint32_t fb_green;
uint32_t fb_darkgreen;
uint32_t fb_olive;
uint32_t fb_lightcyan;
uint32_t fb_cyan;
uint32_t fb_lightblue;
uint32_t fb_blue;
uint32_t fb_darkblue;
uint32_t fb_brown;
uint32_t fb_maroon;
uint32_t fb_white;
uint32_t fb_lightgray;
uint32_t fb_gray;
uint32_t fb_darkgray;
uint32_t fb_silver;
uint32_t fb_black;
uint32_t fb_purple;

inline static uint32_t fb_make_color(uint8_t r, uint8_t g, uint8_t b) {
    return ((r << __fb_r_pos) & __fb_r_mask) | ((g << __fb_g_pos) & __fb_g_mask) | ((b << __fb_b_pos) & __fb_b_mask);
}

#define FB_HUE_DEGREE 256 /* Value for 1 degree (of 360) of hue, when passed to tfb_make_color_hsv() */
#define DEG_60 (60 * FB_HUE_DEGREE)

uint32_t tfb_make_color_hsv(uint32_t h, uint8_t s, uint8_t v) {
    int sv = -s * v;
    uint32_t r = 0, g = 0, b = 0;
    uint32_t region = h / DEG_60;
    uint32_t r1 = region;
    uint32_t p = (256 * v - s * v) / 256;

    if (!(region & 1)) {
        r1++;
        sv = -sv;
    }
    const uint32_t x = (256 * DEG_60 * v + h * sv - DEG_60 * sv * r1) / (256 * DEG_60);
    switch(region) {
        case 0: r = v; g = x; b = p; break;
        case 1: r = x; g = v; b = p; break;
        case 2: r = p; g = v; b = x; break;
        case 3: r = p; g = x; b = v; break;
        case 4: r = x; g = p; b = v; break;
        case 5: r = v; g = p; b = x; break;
    }
    return fb_make_color(r, g, b);
}

/* Error codes */

enum {
    FB_SUCCESS = 0, // The call completed successfully without any errors.
    FB_ERR_OPEN_FB = 1, // open() failed on the framebuffer device
    FB_ERR_IOCTL_FB = 2, // ioctl() failed on the framebuffer device file descriptor
    FB_ERR_OPEN_TTY = 3, // open() failed on the TTY device
    FB_ERR_TTY_GRAPHIC_MODE = 4, // ioctl() on the TTY failed while trying to set TTY in graphic mode
    FB_ERR_MMAP_FB = 5, // mmap() on the framebuffer file description failed
    FB_ERR_INVALID_WINDOW = 6, // Invalid window position/size
    FB_ERR_UNSUPPORTED_VIDEO_MODE = 7, // Unsupported video mode (currently the library supports only 32-bit color modes.)
    FB_ERR_INVALID_FONT_ID = 8, // The supplied font_id is invalid
    FB_ERR_READ_FONT_FILE_FAILED = 9, // Unable to open/read/load the supplied font file
    FB_ERR_OUT_OF_MEMORY = 10, // Out of memory (malloc() returned 0)
    FB_ERR_NOT_A_DYN_LOADED_FONT = 11, // The supplied font_id is does not belog to a dynamically loaded font
    FB_ERR_KB_WRONG_MODE = 12, // The keyboard input is not in the expected mode (e.g. already in raw mode)
    FB_ERR_KB_MODE_GET_FAILED = 13, // Unable to get a keyboard input paramater with ioctl()
    FB_ERR_KB_MODE_SET_FAILED = 14, // Unable to set a keyboard input paramater with ioctl()
    FB_ERR_FONT_NOT_FOUND = 15, // Unable to find a font matching the criteria
    FB_ERR_FB_FLUSH_IOCTL_FAILED = 16, // Unable to flush the framebuffer with ioctl()
};

static const char *error_msgs[] = {
    /*  0 */  "Success",
    /*  1 */  "Unable to open the framebuffer device",
    /*  2 */  "Unable to get screen info for the framebuffer device",
    /*  3 */  "Unable to open the TTY device",
    /*  4 */  "Unable to set TTY in graphic mode",
    /*  5 */  "Unable to mmap the framebuffer",
    /*  6 */  "Invalid window position/size",
    /*  7 */  "Unsupported video mode",
    /*  8 */  "Invalid font_id",
    /*  9 */  "Unable to open/read/load the font file",
    /* 10 */  "Out of memory",
    /* 11 */  "The font_id is does not belog to a dynamically loaded font",
    /* 12 */  "The keyboard input is not in the expected mode",
    /* 13 */  "Unable to get a keyboard input paramater with ioctl()",
    /* 14 */  "Unable to set a keyboard input paramater with ioctl()",
    /* 15 */  "Unable to find a font matching the criteria",
    /* 16 */  "Unable to flush the framebuffer with ioctl()",
};

const char *fb_strerror(int error_code) {
    if (error_code < 0 || error_code >= ARRAY_SIZE(error_msgs))
        return "(unknown error)";

    return error_msgs[error_code];
}

static void fb_init_colors(void) {
    fb_lightred = fb_make_color(255, 204, 203);
    fb_red = fb_make_color(255, 0, 0);
    fb_darkred = fb_make_color(139, 0, 0);
    fb_pink = fb_make_color(255, 192, 203);
    fb_deeppink = fb_make_color(255, 20, 147);
    fb_orange = fb_make_color(255, 165, 0);
    fb_darkorange = fb_make_color(255, 140, 0);
    fb_gold = fb_make_color(255, 215, 0);
    fb_yellow = fb_make_color(255, 255, 0);
    fb_violet = fb_make_color(238, 130, 238);
    fb_lightmagenta = fb_make_color(255, 66, 249);
    fb_magenta = fb_make_color(255, 0, 255);
    fb_darkviolet = fb_make_color(148, 0, 211);
    fb_indigo = fb_make_color(75, 0, 130);
    fb_lightgreen = fb_make_color(144, 238, 144);
    fb_green = fb_make_color(0, 255, 0);
    fb_darkgreen = fb_make_color(0, 100, 0);
    fb_olive = fb_make_color(128, 128, 0);
    fb_lightcyan = fb_make_color(224, 255, 255);
    fb_cyan = fb_make_color(0, 255, 255);
    fb_lightblue = fb_make_color(173, 216, 230);
    fb_blue = fb_make_color(0, 0, 255);
    fb_darkblue = fb_make_color(0, 0, 139);
    fb_brown = fb_make_color(165, 42, 42);
    fb_maroon = fb_make_color(128, 0, 0);
    fb_white = fb_make_color(255, 255, 255);
    fb_lightgray = fb_make_color(211, 211, 211);
    fb_gray = fb_make_color(128, 128, 128);
    fb_darkgray = fb_make_color(169, 169, 169);
    fb_silver = fb_make_color(192, 192, 192);
    fb_black = fb_make_color(0, 0, 0);
    fb_purple = fb_make_color(128, 0, 128);
}

/* Drawing and window handling */

int fb_set_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    if (x + w > (uint32_t)__fb_screen_w)
        return FB_ERR_INVALID_WINDOW;

    if (y + h > (uint32_t)__fb_screen_h)
        return FB_ERR_INVALID_WINDOW;

    __fb_off_x = __fbi.xoffset + x;
    __fb_off_y = __fbi.yoffset + y;
    __fb_win_w = w;
    __fb_win_h = h;
    __fb_win_end_x = __fb_off_x + __fb_win_w;
    __fb_win_end_y = __fb_off_y + __fb_win_h;

    return FB_SUCCESS;
}

uint32_t fb_screen_width(void) { return __fb_screen_w; }
uint32_t fb_screen_height(void) { return __fb_screen_h; }
uint32_t fb_win_width(void) { return __fb_win_w; }
uint32_t fb_win_height(void) { return __fb_win_h; }

inline void fb_draw_pixel(int x, int y, uint32_t color) {
    x += __fb_off_x;
    y += __fb_off_y;

    if ((uint32_t)x < (uint32_t)__fb_win_end_x && (uint32_t)y < (uint32_t)__fb_win_end_y)
        ((volatile uint32_t *)__fb_buffer)[x + y * __fb_pitch_div4] = color;
}

void fb_draw_hline(int x, int y, int len, uint32_t color) {
    if (x < 0) {
        len += x;
        x = 0;
    }

    x += __fb_off_x;
    y += __fb_off_y;

    if (len < 0 || y < __fb_off_y || y >= __fb_win_end_y)
        return;

    len = MIN(len, MAX(0, (int)__fb_win_end_x - x));
    memset32(__fb_buffer + y * __fb_pitch + (x << 2), color, len);
}

void fb_draw_vline(int x, int y, int len, uint32_t color) {
    if (y < 0) {
        len += y;
        y = 0;
    }

    x += __fb_off_x;
    y += __fb_off_y;

    if (len < 0 || x < __fb_off_x || x >= __fb_win_end_x)
        return;

    int yend = MIN(y + len, __fb_win_end_y);

    volatile uint32_t *buf = ((volatile uint32_t *) __fb_buffer) + y * __fb_pitch_div4 + x;

    for (; y < yend; y++, buf += __fb_pitch_div4)
        *buf = color;
}

void fb_fill_rect(int x, int y, int w, int h, uint32_t color) {
    if (w < 0) {
        x += w;
        w = -w;
    }

    if (h < 0) {
        y += h;
        h = -h;
    }

    x += __fb_off_x;
    y += __fb_off_y;

    if (x < 0) {
        w += x;
        x = 0;
    }

    if (y < 0) {
        h += y;
        y = 0;
    }

    if (w < 0 || h < 0)
        return;

    w = MIN(w, MAX(0, (int)__fb_win_end_x - x));
    uint32_t yend = MIN(y + h, __fb_win_end_y);

    void *dest = __fb_buffer + y * __fb_pitch + (x << 2);

    for (uint32_t cy = y; cy < yend; cy++, dest += __fb_pitch)
        memset32(dest, color, w);
}

void fb_draw_rect(int x, int y, int w, int h, uint32_t color) {
    fb_draw_hline(x, y, w, color);
    fb_draw_vline(x, y, h, color);
    fb_draw_vline(x + w - 1, y, h, color);
    fb_draw_hline(x, y + h - 1, w, color);
}

void fb_clear_screen(uint32_t color) {
    if (__fb_pitch == (uint32_t) 4 * __fb_screen_w) {
        memset32(__fb_buffer, color, __fb_size >> 2);
        return;
    }

    for (int y = 0; y < __fb_screen_h; y++)
        fb_draw_hline(0, y, __fb_screen_w, color);
}

void fb_clear_win(uint32_t color)
{
    fb_fill_rect(0, 0, __fb_win_w, __fb_win_h, color);
}

/* Fonts and text drawing */

static void *curr_font;
static uint32_t curr_font_w;
static uint32_t curr_font_h;
static uint32_t curr_font_w_bytes;
static uint32_t curr_font_bytes_per_glyph;
static uint8_t *curr_font_data;

int fb_set_current_font(fb_font_t font_id) {
    const struct font_file *ff = (font_file *)font_id;
    struct psf1_header *h1 = (psf1_header *)ff->data;
    struct psf2_header *h2 = (psf2_header *)ff->data;

    if (h2->magic == PSF2_MAGIC) {
        curr_font = h2;
        curr_font_w = h2->width;
        curr_font_h = h2->height;
        curr_font_w_bytes = h2->bytes_per_glyph / h2->height;
        curr_font_data = (uint8_t *)curr_font + h2->header_size;
        curr_font_bytes_per_glyph = h2->bytes_per_glyph;
    } else if (h1->magic == PSF1_MAGIC) {
        curr_font = h1;
        curr_font_w = 8;
        curr_font_h = h1->bytes_per_glyph;
        curr_font_w_bytes = 1;
        curr_font_data = (uint8_t *)curr_font + sizeof(struct psf1_header);
        curr_font_bytes_per_glyph = h1->bytes_per_glyph;
    } else {
        return FB_ERR_INVALID_FONT_ID;
    }

    return FB_SUCCESS;
}

void fb_set_default_font(fb_font_t font_id)
{
    if (!curr_font)
        fb_set_current_font(font_id);
}

#define draw_char_partial(b)                                                \
    do {                                                                     \
        fb_draw_pixel(x + (b << 3) + 7, row, arr[!(data[b] & (1 << 0))]);    \
        fb_draw_pixel(x + (b << 3) + 6, row, arr[!(data[b] & (1 << 1))]);    \
        fb_draw_pixel(x + (b << 3) + 5, row, arr[!(data[b] & (1 << 2))]);    \
        fb_draw_pixel(x + (b << 3) + 4, row, arr[!(data[b] & (1 << 3))]);    \
        fb_draw_pixel(x + (b << 3) + 3, row, arr[!(data[b] & (1 << 4))]);    \
        fb_draw_pixel(x + (b << 3) + 2, row, arr[!(data[b] & (1 << 5))]);    \
        fb_draw_pixel(x + (b << 3) + 1, row, arr[!(data[b] & (1 << 6))]);    \
        fb_draw_pixel(x + (b << 3) + 0, row, arr[!(data[b] & (1 << 7))]);    \
    } while (0)

void fb_draw_char(int x, int y, uint32_t fg_color, uint32_t bg_color, uint16_t c) {
    if (!curr_font) {
        fprintf(stderr, "[fblib] ERROR: no font currently selected\n");
        return;
    }

    uint8_t *data = curr_font_data + curr_font_bytes_per_glyph * c;
    const uint32_t arr[] = { fg_color, bg_color };

    /*
     * NOTE: the following algorithm is certainly not the fastest way to draw
     * a character on-screen, but its performance is pretty good, in particular
     * for text that does not have to change continuosly (like a console).
     * Actually, this algorithm is used by Tilck[1]'s framebuffer console in a
     * fail-safe case for drawing characters on-screen: on low resolutions,
     * its performance is pretty acceptable on modern machines, even when used
     * by a console to full-redraw a screen with text. Therefore, for the
     * purposes of this library (mostly to show static text on-screen), the
     * following implementation is absolutely good enough.
     *
     * -------------------------------------------------
     * [1] Tilck [A Tiny Linux-Compatible Kernel]
     *     https://github.com/vvaltchev/tilck
     */

    if (curr_font_w_bytes == 1) {
        for (uint32_t row = y; row < (y + curr_font_h); row++) {
            draw_char_partial(0);
            data += curr_font_w_bytes;
        }
    } else if (curr_font_w_bytes == 2) {
        for (uint32_t row = y; row < (y + curr_font_h); row++) {
            draw_char_partial(0);
            draw_char_partial(1);
            data += curr_font_w_bytes;
        }
    } else {
        for (uint32_t row = y; row < (y + curr_font_h); row++) {
            for (uint32_t b = 0; b < curr_font_w_bytes; b++) {
            draw_char_partial(b);
            }
            data += curr_font_w_bytes;
        }
    }
}

void fb_draw_char_scaled(int x, int y, uint32_t fg, uint32_t bg, int xscale, int yscale, uint16_t c) {
    if (!curr_font) {
        fprintf(stderr, "[fblib] ERROR: no font currently selected\n");
        return;
    }

    if (xscale < 0)
        x += -xscale * curr_font_w;

    if (yscale < 0)
        y += -yscale * curr_font_h;

    uint8_t *d = curr_font_data + curr_font_bytes_per_glyph * c;

    /*
     * NOTE: this algorithm is clearly much slower than the simpler variant
     * used in tfb_draw_char(), but it is still pretty good for static text.
     * In case better performance is needed, the proper solution would be to use
     * a scaled font instead of the *_scaled draw text functions.
     */

    for (uint32_t row = 0; row < curr_font_h; row++, d += curr_font_w_bytes)
        for (uint32_t b = 0; b < curr_font_w_bytes; b++)
            for (uint32_t bit = 0; bit < 8; bit++) {
            const int xoff = xscale * ((b << 3) + 8 - bit - 1);
            const int yoff = yscale * row;
            const uint32_t color = (d[b] & (1 << bit)) ? fg : bg;
            fb_fill_rect(x + xoff, y + yoff, xscale, yscale, color);
        }
}

void fb_draw_string(int x, int y, uint32_t fg_color, uint32_t bg_color, const char *s) {
    if (!curr_font) {
        fprintf(stderr, "[fblib] ERROR: no font currently selected\n");
        return;
    }

    for (; *s; s++, x += curr_font_w) {
        fb_draw_char(x, y, fg_color, bg_color, *s);
    }
}

void fb_draw_string_scaled(int x, int y, uint32_t fg, uint32_t bg, int xscale, int yscale, const char *s) {
    if (!curr_font) {
        fprintf(stderr, "[fblib] ERROR: no font currently selected\n");
        return;
    }

    const int xs = xscale > 0 ? xscale : -xscale;

    for (; *s; s++, x += xs * curr_font_w) {
        fb_draw_char_scaled(x, y, fg, bg, xscale, yscale, *s);
    }
}

/* Framebuffer initialisation and management */

void fb_release_fb(void) {
    if (__fb_real_buffer)
        munmap(__fb_real_buffer, __fb_size);

    if (__fb_buffer != __fb_real_buffer)
        free(__fb_buffer);

    if (__fb_ttyfd != -1) {
        ioctl(__fb_ttyfd, KDSETMODE, KD_TEXT);
        close(__fb_ttyfd);
    }

    if (fbfd != -1)
        close(fbfd);
}

int fb_acquire_fb(uint32_t flags, const char *fb_device, const char *tty_device, const char *pref_font) {
    static struct fb_fix_screeninfo fb_fixinfo;

    int ret = FB_SUCCESS;

    auto find_font = [](const char *pref) -> const font_file* {
        if (pref) {
            const font_file **fonts = fb_font_file_list; while (*fonts) {
                const font_file *fnt = *fonts;
                if (strstr(fnt->filename, pref))
                    return fnt;
                fonts++;
            }
        }
        return nullptr;
    };

    if (!fb_device)
        fb_device = DEFAULT_FB_DEVICE;

    if (!tty_device)
        tty_device = DEFAULT_TTY_DEVICE;

    fbfd = open(fb_device, O_RDWR);

    if (fbfd < 0) {
        ret = FB_ERR_OPEN_FB;
        goto out;
    }

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fb_fixinfo) != 0) {
        ret = FB_ERR_IOCTL_FB;
        goto out;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &__fbi) != 0) {
        ret = FB_ERR_IOCTL_FB;
        goto out;
    }

    __fb_pitch = fb_fixinfo.line_length;
    __fb_size = __fb_pitch * __fbi.yres;
    __fb_pitch_div4 = __fb_pitch >> 2;

    if (__fbi.bits_per_pixel != 32) {
        ret = FB_ERR_UNSUPPORTED_VIDEO_MODE;
        goto out;
    }

    if (__fbi.red.msb_right || __fbi.green.msb_right || __fbi.blue.msb_right) {
        ret = FB_ERR_UNSUPPORTED_VIDEO_MODE;
        goto out;
    }

    __fb_ttyfd = open(tty_device, O_RDWR);

    if (__fb_ttyfd < 0) {
        ret = FB_ERR_OPEN_TTY;
        goto out;
    }

    if (!(flags & FB_FL_NO_TTY_KD_GRAPHICS)) {
        if (ioctl(__fb_ttyfd, KDSETMODE, KD_GRAPHICS) != 0) {
            ret = FB_ERR_TTY_GRAPHIC_MODE;
            goto out;
        }
    }

    __fb_real_buffer = mmap(NULL, __fb_size,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED, fbfd, 0);

    if (__fb_real_buffer == MAP_FAILED) {
        ret = FB_ERR_MMAP_FB;
        goto out;
    }

    if (flags & FB_FL_USE_DOUBLE_BUFFER) {
        __fb_buffer = malloc(__fb_size);

        if (!__fb_buffer) {
            ret = FB_ERR_OUT_OF_MEMORY;
            goto out;
        }
    } else {
        __fb_buffer = __fb_real_buffer;
    }

    __fb_screen_w = __fbi.xres;
    __fb_screen_h = __fbi.yres;

    __fb_r_pos = __fbi.red.offset;
    __fb_r_mask_size = __fbi.red.length;
    __fb_r_mask = ((1 << __fb_r_mask_size) - 1) << __fb_r_pos;

    __fb_g_pos = __fbi.green.offset;
    __fb_g_mask_size = __fbi.green.length;
    __fb_g_mask = ((1 << __fb_g_mask_size) - 1) << __fb_g_pos;

    __fb_b_pos = __fbi.blue.offset;
    __fb_b_mask_size = __fbi.blue.length;
    __fb_b_mask = ((1 << __fb_b_mask_size) - 1) << __fb_b_pos;

    fb_set_window(0, 0, __fb_screen_w, __fb_screen_h);
    fb_init_colors();

    /* Just use as default font the first one (if any) */
    if (*fb_font_file_list) {
        if (const font_file *fnt = find_font(pref_font)) {
            fb_set_default_font((void *)fnt);
        } if (const font_file *fnt = find_font("8x8")) {
            fb_set_default_font((void *)fnt);
        } else
            fb_set_default_font((void *)*fb_font_file_list);
    }

out:
    if (ret != FB_SUCCESS)
        fb_release_fb();

    return ret;
}

void fb_flush_rect(int x, int y, int w, int h) {
    if (__fb_buffer == __fb_real_buffer)
        return;

    x += __fb_off_x;
    y += __fb_off_y;

    if (x < 0) {
        w += x;
        x = 0;
    }

    if (y < 0) {
        h += y;
        y = 0;
    }

    if (w < 0 || h < 0)
        return;

    w = MIN(w, MAX(0, __fb_win_end_x - x));
    int yend = MIN(y + h, __fb_win_end_y);

    size_t offset = y * __fb_pitch + (x << 2);
    void *dest = __fb_real_buffer + offset;
    void *src = __fb_buffer + offset;
    uint32_t rect_pitch = w << 2;

    for (int cy = y; cy < yend; cy++, src += __fb_pitch, dest += __fb_pitch)
        memcpy(dest, src, rect_pitch);
}

void fb_flush_window(void) { fb_flush_rect(0, 0, __fb_win_w, __fb_win_h); }

int fb_flush_fb(void) {
    __fbi.activate |= FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &__fbi) < 0) {
        return FB_ERR_FB_FLUSH_IOCTL_FAILED;
    }

    return FB_SUCCESS;
}

/* Keyboard management */

/* Set the TTY keyboard input to raw mode */
int fb_set_kb_raw_mode(uint32_t flags) {
    return 0;
}

/* Restore the TTY keyboard input to its previous state */
int fb_restore_kb_mode(void) {
    return 0;
}

/* Read a keystroke */
fb_key_t fb_read_keypress(void) {
    return FB_KEY_NONE;
}

/* Get the number of the F key corresponding to 'k' */
int fb_get_fn_key_num(fb_key_t k) {
    return 0;
}

/* Console management and high level ui */

static uint32_t *con_palette[16] = {
    /* BLACK        */ &fb_black,
    /* BLUE         */ &fb_blue,
    /* GREEN        */ &fb_green,
    /* CYAN         */ &fb_cyan,
    /* RED          */ &fb_red,
    /* MAGENTA      */ &fb_magenta,
    /* BROWN        */ &fb_brown,
    /* LIGHTGRAY    */ &fb_lightgray,
    /* DARKGRAY     */ &fb_darkgray,
    /* LIGHTBLUE    */ &fb_lightblue,
    /* LIGHTGREEN   */ &fb_lightgreen,
    /* LIGHTCYAN    */ &fb_lightcyan,
    /* LIGHTRED     */ &fb_lightred,
    /* LIGHTMAGENTA */ &fb_lightmagenta,
    /* YELLOW       */ &fb_yellow,
    /* WHITE        */ &fb_white,
};

static int __fg_color = 0;
static int __bg_color = 15;

uint32_t fb_con_width(void) { return (__fb_screen_w / curr_font_w); }
uint32_t fb_con_height(void) { return (__fb_screen_h / curr_font_h); }
void fb_con_set_fg_color(int fg) { __fg_color = fg; }
int fb_con_get_fg_color(void) { return __fg_color; }
void fb_con_set_bg_color(int bg) { __bg_color = bg; }
int fb_con_get_bg_color(void) { return __bg_color; }

struct region_t {
    int x, y;
    int width, height;
};

struct line_t {
    int x1, y1;
    int x2, y2;
    uint32_t ch;
    void (*draw) (struct region_t*, struct line_t*);
};

void _put_char(struct region_t *w, int x, int y, uint32_t ch) {
    fb_draw_char((w->x + x) * curr_font_w , (w->y + y) *curr_font_h, *con_palette[__fg_color], *con_palette[__bg_color], ch);
}

/* Helper function for clip_line(). */
static uint8_t _clip_bits(struct region_t *r, int x, int y) {
    uint8_t b = 0;

    if (x < 0)
        b |= (1<<0);
    else if (x >= r->width)
        b |= (1<<1);

    if (y < 0)
        b |= (1<<2);
    else if (y >= r->height)
        b |= (1<<3);

    return b;
}

/* Generic Cohen-Sutherland line clipping function. */
static void _clip_line(struct region_t *r, struct line_t *s) {
    uint8_t bits1 = _clip_bits(r, s->x1, s->y1);
    uint8_t bits2 = _clip_bits(r, s->x2, s->y2);

    if(bits1 & bits2)
        return;

    if(bits1 == 0) {
        if(bits2 == 0)
            s->draw(r, s);
        else {
            int tmp;
            tmp = s->x1; s->x1 = s->x2; s->x2 = tmp;
            tmp = s->y1; s->y1 = s->y2; s->y2 = tmp;
            _clip_line(r, s);
        }

        return;
    }

    if (bits1 & (1<<0)) {
        s->y1 = s->y2 - (s->x2 - 0) * (s->y2 - s->y1) / (s->x2 - s->x1);
        s->x1 = 0;
    } else if (bits1 & (1<<1)) {
        int xmax = r->width - 1;
        s->y1 = s->y2 - (s->x2 - xmax) * (s->y2 - s->y1) / (s->x2 - s->x1);
        s->x1 = xmax;
    } else if (bits1 & (1<<2)) {
        s->x1 = s->x2 - (s->y2 - 0) * (s->x2 - s->x1) / (s->y2 - s->y1);
        s->y1 = 0;
    } else if (bits1 & (1<<3)) {
        int ymax = r->height - 1;
        s->x1 = s->x2 - (s->y2 - ymax) * (s->x2 - s->x1) / (s->y2 - s->y1);
        s->y1 = ymax;
    }

    _clip_line(r, s);
}


/* Solid line drawing function, using Bresenham's mid-point line scan-conversion algorithm. */
static void _draw_solid_line(struct region_t *r, struct line_t* s) {
    int x1 = s->x1, y1 = s->y1, x2 = s->x2, y2 = s->y2;

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int xinc = (x1 > x2) ? -1 : 1;
    int yinc = (y1 > y2) ? -1 : 1;

    if (dx >= dy) {
        int dpr = dy << 1;
        int dpru = dpr - (dx << 1);
        int delta = dpr - dx;

        for(; dx>=0; dx--) {
            _put_char(r, x1, y1, s->ch);
            if (delta > 0)
            {
                x1 += xinc;
                y1 += yinc;
                delta += dpru;
            } else {
                x1 += xinc;
                delta += dpr;
            }
        }
    } else {
        int dpr = dx << 1;
        int dpru = dpr - (dy << 1);
        int delta = dpr - dy;

        for(; dy >= 0; dy--) {
            _put_char(r, x1, y1, s->ch);
            if (delta > 0) {
                x1 += xinc;
                y1 += yinc;
                delta += dpru;
            } else {
                y1 += yinc;
                delta += dpr;
            }
        }
    }
}

/* Thin line drawing function, using Bresenham's mid-point line scan-conversion algorithm and ASCII art graphics. */
static void _draw_thin_line(struct region_t *r, struct line_t* s) {
    uint32_t charmapx[2], charmapy[2];
    int x1, y1, x2, y2;
    int yinc;

    if (s->x2 >= s->x1) {
        charmapx[0] = (s->y1 > s->y2) ? ',' : '`';
        charmapx[1] = (s->y1 > s->y2) ? '\'' : '.';
        x1 = s->x1; y1 = s->y1; x2 = s->x2; y2 = s->y2;
    } else {
        charmapx[0] = (s->y1 > s->y2) ? '`' : '.';
        charmapx[1] = (s->y1 > s->y2) ? ',' : '\'';
        x2 = s->x1; y2 = s->y1; x1 = s->x2; y1 = s->y2;
    }

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    if (y1 > y2) {
        charmapy[0] = ',';
        charmapy[1] = '\'';
        yinc = -1;
    } else {
        yinc = 1;
        charmapy[0] = '`';
        charmapy[1] = '.';
    }

    if (dx >= dy) {
        int dpr = dy << 1;
        int dpru = dpr - (dx << 1);
        int delta = dpr - dx;
        int prev = 0;

        for(; dx>=0; dx--) {
            if (delta > 0) {
                _put_char(r, x1, y1, charmapy[1]);
                x1++;
                y1 += yinc;
                delta += dpru;
                prev = 1;
            } else {
                if (prev)
                    _put_char(r, x1, y1, charmapy[0]);
                else
                    _put_char(r, x1, y1, '-');
                x1++;
                delta += dpr;
                prev = 0;
            }
        }
    } else {
        int dpr = dx << 1;
        int dpru = dpr - (dy << 1);
        int delta = dpr - dy;

        for(; dy >= 0; dy--) {
            if (delta > 0) {
                _put_char(r, x1, y1, charmapx[0]);
                _put_char(r, x1 + 1, y1, charmapx[1]);
                x1++;
                y1 += yinc;
                delta += dpru;
            } else {
                _put_char(r, x1, y1, '|');
                y1 += yinc;
                delta += dpr;
            }
        }
    }
}

/* Draw a line on the canvas using the given character. */
static void _draw_line(struct region_t *r, int x1,  int y1,  int x2, int y2, uint32_t ch) {
    struct line_t s;
    s.x1 = x1;
    s.y1 = y1;
    s.x2 = x2;
    s.y2 = y2;
    s.ch = ch;
    s.draw = _draw_solid_line;
    _clip_line(r, &s);
}

/* Fill a box on the canvas using the given character. */
static int _fill_box(struct region_t *r, int x, int y, int w, int h, uint32_t ch) {
    int x2 = x + w - 1;
    int y2 = y + h - 1;

    if (x > x2) {
        int tmp = x;
        x = x2; x2 = tmp;
    }

    if (y > y2) {
        int tmp = y;
        y = y2; y2 = tmp;
    }

    const int xmax = fb_con_width() - 1;
    const int ymax = fb_con_height() - 1;

    if (x2 < 0 || y2 < 0 || x > xmax || y > ymax)
        return 0;

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 > xmax) x2 = xmax;
    if (y2 > ymax) y2 = ymax;

    for (int j = y; j <= y2; j++)
        for (int i = x; i <= x2; i++)
            _put_char(r, i, j, ch);

    return 0;
}

static int _draw_box(struct region_t *r, int x, int y, int w, int h, uint32_t const *chars) {
    int x2 = x + w - 1;
    int y2 = y + h - 1;

    if (x > x2) {
        int tmp = x;
        x = x2; x2 = tmp;
    }

    if (y > y2) {
        int tmp = y;
        y = y2; y2 = tmp;
    }

    const int xmax = r->width - 1;
    const int ymax = r->height - 1;

    if (x2 < 0 || y2 < 0 || x > xmax || y > ymax)
        return 0;

    /* Draw edges */
    if (y >= 0)
        for (int i = x < 0 ? 1 : x + 1; i < x2 && i < xmax; i++)
            _put_char(r, i, y, chars[0]);

    if (y2 <= ymax)
        for (int i = x < 0 ? 1 : x + 1; i < x2 && i < xmax; i++)
            _put_char(r, i, y2, chars[0]);

    if (x >= 0)
        for (int j = y < 0 ? 1 : y + 1; j < y2 && j < ymax; j++)
            _put_char(r, x, j, chars[1]);

    if (x2 <= xmax)
        for (int j = y < 0 ? 1 : y + 1; j < y2 && j < ymax; j++)
            _put_char(r, x2, j, chars[1]);

    /* Draw corners */
    _put_char(r, x, y, chars[2]);
    _put_char(r, x, y2, chars[3]);
    _put_char(r, x2, y, chars[4]);
    _put_char(r, x2, y2, chars[5]);

    return 0;
}

/* Draw a box on the canvas using the given character. */
static void _draw_box(struct region_t *r, int x, int y, int w, int h, uint32_t ch) {
    const int x2 = x + w - 1;
    const int y2 = y + h - 1;

    _draw_line(r, x,  y,  x, y2, ch);
    _draw_line(r, x, y2, x2, y2, ch);
    _draw_line(r, x2, y2, x2,  y, ch);
    _draw_line(r, x2,  y,  x,  y, ch);
}

/* Draw a thin box on the canvas. */
static int _draw_thin_box(struct region_t *r, int x, int y, int w, int h) {
    static uint32_t const ascii_chars[] =
    {
        '-', '|', ',', '`', '.', '\'',
    };

    return _draw_box(r, x, y, w, h, ascii_chars);
}

/* Draw a box on the canvas using CP437 characters. */
static int _draw_cp437_box(struct region_t *r, int x, int y, int w, int h) {
    static uint32_t const cp437_chars[] =
    {
        /* ─ │ ┌ └ ┐ ┘ */
        BOX_SLR, BOX_SUD, BOX_SDR, BOX_SUR, BOX_SDL, BOX_SUL,
    };

    return _draw_box(r, x, y, w, h, cp437_chars);
}

/* Draw a box on the canvas using Unicode characters. */
static int _draw_uc_box(struct region_t *r, int x, int y, int w, int h) {
    static uint32_t const cp437_chars[] =
    {
        /* ─ │ ┌ └ ┐ ┘ */
        0x2500, 0x2502, 0x250c, 0x2514, 0x2510, 0x2518,
    };

    return _draw_box(r, x, y, w, h, cp437_chars);
}

void fb_con_put_char_attrib(int x, int y, uint32_t ch, int fg, int bg) {
    fb_draw_char(x * curr_font_w , y *curr_font_h, *con_palette[fg], *con_palette[bg], ch);
}

void fb_con_put_char(int x, int y, uint32_t fg_color, uint32_t bg_color, uint32_t ch) {
    fb_con_put_char_attrib(x , y, fg_color, bg_color, ch);
}

void fb_con_draw_string(int x, int y, uint32_t fg_color, uint32_t bg_color, const char *s) {
    if (!curr_font) {
        fprintf(stderr, "[fblib] ERROR: no font currently selected\n");
        return;
    }

    for (; *s; s++, x += curr_font_w) {
        fb_draw_char(x, y, fg_color, bg_color, *s);
    }
}

void fb_con_draw_line(int x1, int y1, int x2, int y2, uint32_t ch) {
    struct region_t r{0, 0, (int)fb_con_width(), (int)fb_con_height()};
}

void fb_con_draw_box(int x, int y, int w, int h) {
    struct region_t r{0, 0, (int)fb_con_width(), (int)fb_con_height()};
    _draw_cp437_box(&r, x, y, w, h);
}

/* GUI elements */

static std::string format_string(const char* fmt, va_list ap) {
    std::string message;
    va_list ap_copy;
    va_copy(ap_copy, ap);
    size_t len = vsnprintf(0, 0, fmt, ap_copy);
    message.resize(len + 1); // need space for NUL
    vsnprintf(&message[0], len + 1,fmt, ap);
    message.resize(len); // remove the NUL
    return message;
}

static std::string format_string(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string tmp = format_string(fmt, args);
    va_end(args);
    return tmp;
}

static std::vector<std::string> split_string_by_newline(const std::string &str) {
    auto result = std::vector<std::string>{};
    auto ss = std::stringstream{str};
    for (std::string line; std::getline(ss, line, '\n');)
        result.push_back(line);
    return result;
}

std::vector<std::string> split_string_at_length(std::string str, const std::string delimiters, size_t limit) {
    std::vector<std::string> lines;
    std::string current_line;
    size_t pos = 0, current_length = 0;
    while ((pos = str.find_first_of(delimiters)) != std::string::npos) {
        std::string token = str.substr(0, pos + 1); // Include the delimiter in the token
        if (current_length + token.length() > limit) {
            if (!current_line.empty()) {
                lines.push_back(current_line);
                current_line.clear();
                current_length = 0;
            }
        }
        current_line += token, current_length += token.length();
        str.erase(0, pos + 1);
    }
    if (!str.empty()) { // Add the remaining part of the string
        if (current_length + str.length() > limit && !current_line.empty()) {
            lines.push_back(current_line);
            current_line.clear();
        }
        current_line += str;
    }
    if (!current_line.empty())
        lines.push_back(current_line);
    return lines;
}

std::vector<std::string> split_string(std::string str, const std::string delimeters) {
    std::vector<std::string> split = {};
    size_t pos = 0;
    while ((pos = str.find_first_of(delimeters)) != std::string::npos) {
        std::string token = str.substr(0, pos);
        if (token.length() > 0)
            split.push_back(token);
        str.erase(0, pos + delimeters.length());
    }
    if (!str.empty())
        split.push_back(str);
    return split;
}

std::string wrap_string_at_length(std::string str, const std::string delimiters, size_t limit) {
    const auto lines = split_string_at_length(str, delimiters, limit);
    return std::accumulate(lines.begin(), lines.end(), std::string("\n"));
}

void alert(const char *msg) {
    fb_con_draw_box(4, 4, 10, 10);
}

void fatal(const char *msg) {
    fb_con_draw_box(4, 4, 10, 10);
}

void fatal_exit(const char *fmt, va_list ap) {
    std::string msg = format_string(fmt, ap);
    fatal(msg.c_str());
    exit(1);
}

void fatal_exit(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fatal_exit(fmt, args);
  va_end(args);
}

/// MAIN RUN

static bool running = true;
static struct termios start_tty;

void cleanup(void) {
    console_clear();
    console_alt_exit();
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &start_tty);
}

void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
        case SIGTERM:
            running = false;
            break;
        case SIGSEGV:
            cleanup();
            exit(EXIT_FAILURE);
            break;
    }

    return;
}


int main(int argc, char **argv) {
    int rc;

    if ((rc = fb_acquire_fb(FB_FL_NO_TTY_KD_GRAPHICS, NULL, NULL, "8x12")) != FB_SUCCESS) {
        fprintf(stderr, "fb_acquire_fb() failed: %s\n", fb_strerror(rc));
        return 1;
    }

    // console setup

    console_alt_enter();
    cursor_hide();

    tcgetattr(STDIN_FILENO, &start_tty);
    struct termios run_tty = start_tty;
    run_tty.c_iflag &= (~ICRNL) & (~IXON);
    run_tty.c_lflag &= (~ICANON) & (~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &run_tty);

    FILE *echo = freopen( "echo.log", "w", stdout ); // send output to log file
    if (!echo)
        FATAL("Cannot reopen output stream");

    // setup signals and exit cleanup

    struct sigaction sig;
    sig.sa_handler = signal_handler;
    sig.sa_flags = 0;
    sigaction(SIGTERM, &sig, NULL);
    sigaction(SIGINT, &sig, NULL);
    sigaction(SIGSEGV, &sig, NULL);

    atexit(cleanup);

    backboard.resize(); // ready

    while(not backboard.quit() && running) {
        backboard.update();
    }

    if (!freopen("CON", "w", stdout))
        FATAL("Cannot restore output stream");

    fb_release_fb();
    console_alt_exit();

    fclose(echo);

    return 0;
}
