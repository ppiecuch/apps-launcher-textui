#ifndef PSF_FONT_H
#define PSF_FONT_H

#include <stdint.h>

struct font_file {
   const char *filename;
   unsigned int data_size;
   unsigned char data[];
};

extern const struct font_file **fb_font_file_list;

#define PSF1_MAGIC               0x0436

#define PSF1_MODE512             0x01
#define PSF1_MODEHASTAB          0x02
#define PSF1_MODEHASSEQ          0x04
#define PSF1_MAXMODE             0x05

#define PSF1_SEPARATOR           0xFFFF
#define PSF1_STARTSEQ            0xFFFE

struct psf1_header {
   uint16_t magic;
   uint8_t mode;
   uint8_t bytes_per_glyph;
};

#define PSF2_MAGIC               0x864ab572

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE   0x01

/* max version recognized so far */
#define PSF2_MAXVERSION          0

/* UTF8 separators */
#define PSF2_SEPARATOR           0xFF
#define PSF2_STARTSEQ            0xFE

struct psf2_header {
    uint32_t magic;
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t glyphs_count;
    uint32_t bytes_per_glyph;
    uint32_t height;          /* height in pixels */
    uint32_t width;           /* width in pixels */
};

typedef void *fb_font_t; /* Opaque font type */

#endif // PSF_FONT_H
