#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>

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

struct fb_var_screeninfo __fbi;
int __tfb_ttyfd = -1;

static int fbfd = -1;

/* Essential variables */
extern void *__fb_buffer;
extern void *__fb_real_buffer;
extern int __fb_screen_w;
extern int __fb_screen_h;
extern size_t __fb_size;
extern size_t __fb_pitch;
extern size_t __fb_pitch_div4; /* see the comment in drawing.c */

/* Window-related variables */
extern int __fb_win_w;
extern int __fb_win_h;
extern int __fb_off_x;
extern int __fb_off_y;
extern int __fb_win_end_x;
extern int __fb_win_end_y;

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

inline static uint32_t fb_make_color(uint8_t r, uint8_t g, uint8_t b)
{
   return ((r << __fb_r_pos) & __fb_r_mask) |
          ((g << __fb_g_pos) & __fb_g_mask) |
          ((b << __fb_b_pos) & __fb_b_mask);
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

static const char *error_msgs[] =
{
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

const char *fb_strerror(int error_code)
{
   if (error_code < 0 || error_code >= ARRAY_SIZE(error_msgs))
      return "(unknown error)";

   return error_msgs[error_code];
}

static void fb_init_colors(void)
{
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

int fb_acquire_fb(uint32_t flags, const char *fb_device, const char *tty_device)
{
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

void fb_release_fb(void)
{
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

/// MAIN RUN

int main(int argc, char **argv)
{
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
                 h / 2 - rect_h / 2,  /* y coordinate */
                 rect_w,              /* width */
                 rect_h,              /* height */
                 fb_red              /* color */);

   getchar();
   fb_release_fb();
   return 0;
}
