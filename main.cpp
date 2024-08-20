// Reference:
// ----------
//  - http://www.seasip.info/Unix/PSF/Amstrad/Setfont/index.html

#include <cstdint>
#include <string.h>
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
#include <termios.h>
#include <errno.h>

#include "graphics/psf.h"

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
uint32_t fb_red;
uint32_t fb_darkred;
uint32_t fb_pink;
uint32_t fb_deeppink;
uint32_t fb_orange;
uint32_t fb_darkorange;
uint32_t fb_gold;
uint32_t fb_yellow;
uint32_t fb_violet;
uint32_t fb_magenta;
uint32_t fb_darkviolet;
uint32_t fb_indigo;
uint32_t fb_lightgreen;
uint32_t fb_green;
uint32_t fb_darkgreen;
uint32_t fb_olive;
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
    fb_red = fb_make_color(255, 0, 0);
    fb_darkred = fb_make_color(139, 0, 0);
    fb_pink = fb_make_color(255, 192, 203);
    fb_deeppink = fb_make_color(255, 20, 147);
    fb_orange = fb_make_color(255, 165, 0);
    fb_darkorange = fb_make_color(255, 140, 0);
    fb_gold = fb_make_color(255, 215, 0);
    fb_yellow = fb_make_color(255, 255, 0);
    fb_violet = fb_make_color(238, 130, 238);
    fb_magenta = fb_make_color(255, 0, 255);
    fb_darkviolet = fb_make_color(148, 0, 211);
    fb_indigo = fb_make_color(75, 0, 130);
    fb_lightgreen = fb_make_color(144, 238, 144);
    fb_green = fb_make_color(0, 255, 0);
    fb_darkgreen = fb_make_color(0, 100, 0);
    fb_olive = fb_make_color(128, 128, 0);
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

inline uint32_t fb_screen_width(void) { return __fb_screen_w; }
inline uint32_t fb_screen_height(void) { return __fb_screen_h; }
inline uint32_t fb_win_width(void) { return __fb_win_w; }
inline uint32_t fb_win_height(void) { return __fb_win_h; }

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

void fb_draw_char(int x, int y, uint32_t fg_color, uint32_t bg_color, uint8_t c) {
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

void fb_draw_char_scaled(int x, int y, uint32_t fg, uint32_t bg, int xscale, int yscale, uint8_t c) {
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

int fb_acquire_fb(uint32_t flags, const char *fb_device, const char *tty_device) {
    static struct fb_fix_screeninfo fb_fixinfo;

    int ret = FB_SUCCESS;

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
    if (*fb_font_file_list)
        fb_set_default_font((void *)*fb_font_file_list);

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

/* Console management and high level ui */

inline uint32_t fb_console_width(void) { __fb_screen_w / curr_font_w; }
inline uint32_t fb_console_height(void) {  __fb_screen_h / curr_font_h; }


/// MAIN RUN

int main(int argc, char **argv) {
    int rc;

    if ((rc = fb_acquire_fb(0, NULL, NULL)) != FB_SUCCESS) {
        fprintf(stderr, "fb_acquire_fb() failed: %s\n", fb_strerror(rc));
        return 1;
    }

    uint32_t w = fb_screen_width();
    uint32_t h = fb_screen_height();
    uint32_t rect_w = w / 2;
    uint32_t rect_h = h / 2;

    /* Paint the whole screen in black */
    fb_clear_screen(fb_black);

    /* Draw some text on-screen */
    fb_draw_string(10, 10, fb_white, fb_black, "Press ENTER to quit");

    /* Draw a red rectangle at the center of the screen */
    fb_draw_rect(w / 2 - rect_w / 2,  /* x coordinate */
                 h / 2 - rect_h / 2, /* y coordinate */
                 rect_w,             /* width */
                 rect_h,             /* height */
                 fb_red              /* color */);

    getchar();
    fb_release_fb();
    return 0;
}
