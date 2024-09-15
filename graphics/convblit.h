/*
 * Conversion blits - public header file
 */

#include <stdint.h>

/* types and definitions */

typedef unsigned char *		ADDR8;
typedef unsigned short *	ADDR16;
typedef uint32_t *			ADDR32;

/* Drawing modes (raster ops)*/
#define	MWROP_COPY			0	/* src*/
#define	MWROP_XOR			1	/* src ^ dst*/
#define	MWROP_OR			2	/* src | dst (PAINT)*/
#define	MWROP_AND			3	/* src & dst (MASK)*/
#define	MWROP_CLEAR			4	/* 0*/
#define	MWROP_SET			5	/* ~0*/
#define	MWROP_EQUIV			6	/* ~(src ^ dst)*/
#define	MWROP_NOR			7	/* ~(src | dst)*/
#define	MWROP_NAND			8	/* ~(src & dst)*/
#define	MWROP_INVERT		9	/* ~dst*/
#define	MWROP_COPYINVERTED	10	/* ~src*/
#define	MWROP_ORINVERTED	11	/* ~src | dst*/
#define	MWROP_ANDINVERTED	12	/* ~src & dst (SUBTRACT)*/
#define MWROP_ORREVERSE		13	/* src | ~dst*/
#define	MWROP_ANDREVERSE	14	/* src & ~dst*/
#define	MWROP_NOOP			15	/* dst*/
#define	MWROP_XOR_FGBG		16	/* src ^ background ^ dst (Java XOR mode)*/
#define MWROP_SIMPLE_MAX 	16	/* last non-compositing rop*/

/* Porter-Duff compositing operations.  Only SRC, CLEAR and SRC_OVER are commonly used*/
#define	MWROP_SRC			MWROP_COPY
#define	MWROP_DST			MWROP_NOOP
//#define MWROP_CLEAR		MWROP_CLEAR
#define	MWROP_SRC_OVER		17	/* dst = alphablend src,dst*/
#define	MWROP_DST_OVER		18
#define	MWROP_SRC_IN		19
#define	MWROP_DST_IN		20
#define	MWROP_SRC_OUT		21
#define	MWROP_DST_OUT		22
#define	MWROP_SRC_ATOP		23
#define	MWROP_DST_ATOP		24
#define	MWROP_PORTERDUFF_XOR 25
#define MWROP_SRCTRANSCOPY	26	/* copy src -> dst except for transparent color in src*/
#define	MWROP_MAX			26	/* last non-blit rop*/

/* blit ROP modes in addtion to MWROP_xxx */
#define MWROP_BLENDCONSTANT		32	/* alpha blend src -> dst with constant alpha*/
#define MWROP_BLENDFGBG			33	/* alpha blend fg/bg color -> dst with src alpha channel*/
//#define MWROP_BLENDCHANNEL	35	/* alpha blend src -> dst with separate per pixel alpha chan*/
//#define MWROP_STRETCH			36	/* stretch src -> dst*/
#define MWROP_USE_GC_MODE		255 /* use current GC mode for ROP.  Nano-X CopyArea only*/

/* Note that the following APPLYOP macro implements the
 * Porter-Duff rules assuming that source and destination
 * both have an alpha of 1.0.  Drivers that support alpha
 * should provide a better implementation of these rules.
 *
 * The following are not handled yet:
 *		MWROP_SRC_IN
 *		MWROP_SRC_ATOP
 *		MWROP_DST_OVER
 *		MWROP_DST_IN
 *		MWROP_DST_ATOP
 *		MWROP_SRC_OUT
 *		MWROP_DST_OUT
 *		MWROP_PORTERDUFF_XOR
 *
 * Arguments:
 *	op		- MWROP code
 *	width	- repeat count
 *	STYPE	- src 'type' e.g. (ADDR32) or (MWPIXELVAL)
 *	s		- src pointer or value
 *	DTYPE	- dst 'type' e.g. *(ADDR32)
 *	d		- dst pointer
 *	ssz		- src pointer increment
 *	dsz		- dst pointer increment
 */

/* applyOp with count, src ptr, ssz/dsz increment*/
#define	APPLYOP(op, width, STYPE, s, DTYPE, d, ssz, dsz)	\
	{											\
		int  count = width;						\
		switch (op) {							\
		case MWROP_COPY:						\
		case MWROP_SRC_OVER:					\
		case MWROP_SRC_IN:						\
		case MWROP_SRC_ATOP:					\
			while(--count >= 0) {				\
				DTYPE d = STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_XOR_FGBG:					\
		case MWROP_PORTERDUFF_XOR:				\
			while(--count >= 0) {				\
				DTYPE d ^= (STYPE s) ^ gr_background; \
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_SRCTRANSCOPY:				\
			while(--count >= 0) {				\
				DTYPE d = (DTYPE d)? DTYPE d: STYPE s; \
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_XOR:							\
			while(--count >= 0) {				\
				DTYPE d ^= STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_AND:							\
			while(--count >= 0) {				\
				DTYPE d &= STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_OR:							\
			while(--count >= 0) {				\
				DTYPE d |= STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_SRC_OUT:						\
		case MWROP_DST_OUT:						\
		case MWROP_CLEAR:						\
			while(--count >= 0) {				\
				DTYPE d = 0;					\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_SET:							\
			while(--count >= 0) {				\
				DTYPE d = ~0;					\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_EQUIV:						\
			while(--count >= 0) {				\
				DTYPE d = ~(DTYPE d ^ STYPE s); \
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_NOR:							\
			while(--count >= 0) {				\
				DTYPE d = ~(DTYPE d | STYPE s); \
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_NAND:						\
			while(--count >= 0) {				\
				DTYPE d = ~(DTYPE d & STYPE s); \
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_INVERT:						\
			while(--count >= 0) {				\
				DTYPE d = ~DTYPE d;				\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_COPYINVERTED:				\
			while(--count >= 0) {				\
				DTYPE d = ~STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_ORINVERTED:					\
			while(--count >= 0) {				\
				DTYPE d |= ~STYPE s;			\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_ANDINVERTED:					\
			while(--count >= 0) {				\
				DTYPE d &= ~STYPE s;			\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_ORREVERSE:					\
			while(--count >= 0) {				\
				DTYPE d = ~DTYPE d | STYPE s; 	\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_ANDREVERSE:					\
			while(--count >= 0) {				\
				DTYPE d = ~DTYPE d & STYPE s; 	\
				d += dsz; s += ssz; }			\
			break;								\
		case MWROP_NOOP:						\
		case MWROP_DST_OVER:					\
		case MWROP_DST_IN:						\
		case MWROP_DST_ATOP:					\
			break;								\
		}										\
	}

/* APPLYOP w/return value - used only in fblin4.c*/
#define DEFINE_applyOpR				\
static inline int applyOpR(int op, unsigned char src, unsigned char dst)	\
{						\
	switch (op) {				\
	case MWROP_XOR:			\
		return (src) ^ (dst);		\
	case MWROP_AND:			\
		return (src) & (dst);		\
	case MWROP_OR:				\
		return (src) | (dst);		\
	case MWROP_SRC_OUT:		\
	case MWROP_DST_OUT:		\
	case MWROP_PORTERDUFF_XOR:		\
	case MWROP_CLEAR:			\
		return 0;			\
	case MWROP_SET:			\
		return ~0;			\
	case MWROP_EQUIV:			\
		return ~((src) ^ (dst));	\
	case MWROP_NOR:			\
		return ~((src) | (dst));	\
	case MWROP_NAND:			\
		return ~((src) & (dst));	\
	case MWROP_INVERT:			\
		return ~(dst);			\
	case MWROP_COPYINVERTED:		\
		return ~(src);			\
	case MWROP_ORINVERTED:			\
		return ~(src) | (dst);		\
	case MWROP_ANDINVERTED:		\
		return ~(src) & (dst);		\
	case MWROP_ORREVERSE:			\
		return (src) | ~(dst);		\
	case MWROP_ANDREVERSE:			\
		return (src) & ~(dst);		\
	case MWROP_SRC_OVER:		\
	case MWROP_SRC_IN:			\
	case MWROP_SRC_ATOP:		\
	case MWROP_COPY:			\
		return (src);			\
	case MWROP_XOR_FGBG:		\
		return (src) ^ (dst) ^ gr_background;	\
	case MWROP_DST_OVER:		\
	case MWROP_DST_IN:			\
	case MWROP_DST_ATOP:		\
	case MWROP_NOOP:			\
	default:				\
		return (dst);			\
	}					\
}

/* portrait modes*/
#define MWPORTRAIT_NONE		0x00	/* hw framebuffer, no rotation*/
#define MWPORTRAIT_LEFT		0x01	/* rotate left*/
#define	MWPORTRAIT_RIGHT	0x02	/* rotate right*/
#define MWPORTRAIT_DOWN		0x04	/* upside down*/

/* force byte-packed structures and inlining*/
#if defined(__GNUC__)
#define MWPACKED			__attribute__ ((aligned(1), packed))	/* for structs*/
#define PACKEDDATA			__attribute__ ((__packed__))			/* for data members*/
#define ALWAYS_INLINE		__attribute__ ((always_inline))			/* force inline functions*/
#else
#define MWPACKED
#define PACKEDDATA			/* FIXME for MSVC #pragma pack(1) equiv*/
#define ALWAYS_INLINE
#endif

typedef struct _mwscreendevice *PSD;

typedef int				MWCOORD;	/* device coordinates*/
typedef int				MWBOOL;		/* boolean value*/
typedef unsigned char	MWUCHAR;	/* unsigned char*/
typedef uint32_t		MWPIXELVAL;	/* pixel value parameter type, not for packing*/
typedef uint32_t		MWCOLORVAL;	/* device-independent color value (0xAABBGGRR)*/

extern MWPIXELVAL gr_foreground;		/* current foreground color */
extern MWPIXELVAL gr_background;		/* current background color */

/* Pixel formats */
#define MWPF_TRUECOLORRGB  4	/* pixel is packed 24 bits R/G/B RGB truecolor*/
#define MWPF_TRUECOLOR565  5	/* pixel is packed 16 bits 5/6/5 RGB truecolor*/
#define MWPF_TRUECOLOR555  6	/* pixel is packed 16 bits 5/5/5 RGB truecolor*/
#define MWPF_TRUECOLOR332  7	/* pixel is packed  8 bits 3/3/2 RGB truecolor*/
#define MWPF_TRUECOLORARGB 8	/* pixel is packed 32 bits A/R/G/B ARGB truecolor with alpha */
#define MWPF_TRUECOLOR233  9	/* pixel is packed  8 bits 2/3/3 BGR truecolor*/
#define MWPF_TRUECOLORABGR 11	/* pixel is packed 32 bits A/B/G/R ABGR truecolor with alpha */
#define MWPF_TRUECOLOR1555 12   /* pixel is packed 16 bits 1/5/5/5 NDS truecolor */

/* MWPIXELVALHW definition: changes based on target system. Set using -DMWPIXEL_FORMAT=MWPF_XXX */
#ifndef MWPIXEL_FORMAT
#define MWPIXEL_FORMAT	MWPF_TRUECOLORARGB
#endif

/*
 * Alpha blending evolution
 *
 * Blending r,g,b pixels w/src alpha
 * unoptimized two mult one div		 	bg = (a*fg+(255-a)*bg)/255
 * optimized one mult one div			bg = (a*(fg-bg))/255 + bg
 * optimized /255 replaced with +1/>>8	bg = (((a+1)*(fg-bg))>>8) + bg
 * optimized +=							bg +=(((a+1)*(fg-bg))>>8)
 * macro +=								bg +=muldiv255(a,fg-bg)
 * macro =								bg  =muldiv255(a,fg-bg) + bg
 * -or-
 * macro = (src/dst reversed)			bg  =muldiv255(255-a,bg-fg) + fg
 *
 * Updating dst alpha from src alpha
 * original routine						d =   (255-d)*a/255 + d
 * rearrange							d =   a*(255-d)/255 + d
 * replace multiply by fast +1>>8		d = (((a+1)*(255-d)) >> 8) + d
 * macro =								d =  muldiv255(a, 255 - d) + d
 * macro +=								d += muldiv255(a, 255 - d)
 * -or- src/dst reversed (method used in 0.91, not quite correct)
 * mathematical correct  routine		d =  (d * (255 - a)/255 + a
 * rearrange							d = ((d * (255 - a + 1)) >> 8) + a
 * alternate (used in v0.91)			d = ((d * (256 - a)) >> 8) + a
 * macro = (to duplicate 0.91 code)		d = muldiv255(255 - a, d) + a
 * correct macro =						d = muldiv255(d, 255 - a) + a
 */
/* single byte color macros for 24/32bpp*/

//#define muldiv255(a,b)	(((a)*(b))/255)		/* slow divide, exact*/
//#define muldiv255(a,b)	((((a)+((a)>>7))*(b))>>8)	/* fast, 35% accurate*/
#define muldiv255(a,b)		((((a)+1)*(b))>>8)		/* very fast, 92% accurate*/

/* pixel to pixel blend for 16bpp*/
#define mulscale(a,b,n)		((((a)+1)*(b))>>(n))	/* very fast, always shift for 16bpp*/

/* single byte color macros for 15/16bpp macros - FIXME endian specific*/
/* d = muldiv255(255-a, d-s) + s*/
#define muldiv255_rgb565(d, sr, sg, sb, as) \
						  (((((((d) & 0xF800) - (sr)) * as) >> 8) + (sr)) & 0xF800)\
						| (((((((d) & 0x07E0) - (sg)) * as) >> 8) + (sg)) & 0x07E0)\
						| (((((((d) & 0x001F) - (sb)) * as) >> 8) + (sb)) & 0x001F)
#define muldiv255_rgb555(d, sr, sg, sb, as) \
						  (((((((d) & 0x7C00) - (sr)) * as) >> 8) + (sr)) & 0x7C00)\
						| (((((((d) & 0x03E0) - (sg)) * as) >> 8) + (sg)) & 0x03E0)\
						| (((((((d) & 0x001F) - (sb)) * as) >> 8) + (sb)) & 0x001F)
#define muldiv255_rgb1555(d, sr, sg, sb, as) \
						  (((((((d) & 0x001F) - (sr)) * as) >> 8) + (sr)) & 0x001F)\
						| (((((((d) & 0x03E0) - (sg)) * as) >> 8) + (sg)) & 0x03E0)\
						| (((((((d) & 0x7C00) - (sb)) * as) >> 8) + (sb)) & 0x7C00)


#define RGBDEF(r,g,b)	{r, g, b} /* palette color definition*/

/* return palette entry as MWCOLORVAL (0xAABBGGRR)*/
#define GETPALENTRY(pal,index) ((MWCOLORVAL)(pal[index].r | (pal[index].g << 8) | (pal[index].b << 16) | (255 << 24)))

/* extract MWCOLORVAL (0xAABBGGRR) values*/
#define REDVALUE(rgb)	((rgb) & 0xff)
#define GREENVALUE(rgb) (((rgb) >> 8) & 0xff)
#define BLUEVALUE(rgb)	(((rgb) >> 16) & 0xff)
#define ALPHAVALUE(rgb)	(((rgb) >> 24) & 0xff)

/* Mask values for MWIF_ image data formats*/
/* MWIF_BGR233*/
#define RMASK233	0x07
#define GMASK233	0x38
#define BMASK233	0xc0
#define AMASK233	0x00

/* MWIF_RGB332*/
#define RMASK332	0xe0
#define GMASK332	0x1c
#define BMASK332	0x03
#define AMASK332	0x00

/* MWIF_RGB555*/
#define RMASK555	0x7c00
#define GMASK555	0x03e0
#define BMASK555	0x001f
#define AMASK555	0x8000

/* MWIF_RGB565*/
#define RMASK565	0xf800
#define GMASK565	0x07e0
#define BMASK565	0x001f
#define AMASK565	0x0000

/* MWIF_BGR888*/
#define RMASKBGR	0xff0000
#define GMASKBGR	0x00ff00
#define BMASKBGR	0x0000ff
#define AMASKBGR	0x000000

/* MWIF_BGRA8888*/
#define RMASKBGRA	0x00ff0000
#define GMASKBGRA	0x0000ff00
#define BMASKBGRA	0x000000ff
#define AMASKBGRA	0xff000000

/* MWIF_RGBA8888*/
#define RMASKRGBA	0x000000ff
#define GMASKRGBA	0x0000ff00
#define BMASKRGBA	0x00ff0000
#define AMASKRGBA	0xff000000

/* Truecolor color conversion and extraction macros*/

/* Conversion from 8-bit RGB components to MWPIXELVAL */

#define RGB2PIXEL8888(r,g,b) (0xFF000000UL | ((r) << 16) | ((g) << 8) | (b)) /* create 32 bit 8/8/8/8 format pixel (0xAARRGGBB) from RGB triplet */
#define RGB2PIXELABGR(r,g,b) (0xFF000000UL | ((b) << 16) | ((g) << 8) | (r)) /* create 32 bit 8/8/8/8 format pixel (0xAABBGGRR) from RGB triplet */
#define ARGB2PIXEL8888(a,r,g,b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)) /* create 32 bit 8/8/8/8 format pixel (0xAARRGGBB) from ARGB quad */
#define ARGB2PIXELABGR(a,r,g,b) (((a) << 24) | ((b) << 16) | ((g) << 8) | (r)) /* create 32 bit 8/8/8/8 format pixel (0xAABBGGRR) from ARGB quad */
#define RGB2PIXEL888(r,g,b) (((r) << 16) | ((g) << 8) | (b)) /* create 24 bit 8/8/8 format pixel (0x00RRGGBB) from RGB triplet */
#define RGB2PIXEL565(r,g,b) ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3)) /* create 16 bit 5/6/5 format pixel from RGB triplet */
#define RGB2PIXEL555(r,g,b) ((((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3)) /* create 16 bit 5/5/5 format pixel from RGB triplet */
#define RGB2PIXEL1555(r,g,b) ((((b) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((r) & 0xf8) >> 3) | 0x8000) /* create 16 bit b/g/r 5/5/5 format pixel from RGB triplet */
#define RGB2PIXEL332(r,g,b) (((r) & 0xe0) | (((g) & 0xe0) >> 3) | (((b) & 0xc0) >> 6)) /* create 8 bit 3/3/2 format pixel from RGB triplet */
#define RGB2PIXEL233(r,g,b) ((((r) & 0xe0) >> 5) | (((g) & 0xe0) >> 2) | (((b) & 0xc0) >> 0)) /* create 8 bit 2/3/3 format pixel from RGB triplet */

/* create single component of 5/6/5format pixel from color byte */
#define RED2PIXEL565(byte)		(((byte) & 0xf8) << 8)
#define GREEN2PIXEL565(byte)	(((byte) & 0xfc) << 3)
#define BLUE2PIXEL565(byte)		(((byte) & 0xf8) >> 3)

/* create single component of 5/5/5format pixel from color byte */
#define RED2PIXEL555(byte)		(((byte) & 0xf8) << 7)
#define GREEN2PIXEL555(byte)	(((byte) & 0xf8) << 2)
#define BLUE2PIXEL555(byte)		(((byte) & 0xf8) >> 3)

/* create single component of 1/5/5/5format pixel from color byte */
#define RED2PIXEL1555(byte)		(((byte) & 0xf8) >> 3)
#define GREEN2PIXEL1555(byte)	(((byte) & 0xf8) << 2)
#define BLUE2PIXEL1555(byte)	(((byte) & 0xf8) << 7)

/* these defines used in convblits, must be available regardless of MWPIXEL_FORMAT, default 565 */
#if MWPIXEL_FORMAT == MWPF_TRUECOLOR555
# define muldiv255_16bpp		muldiv255_rgb555
# define RED2PIXEL(byte)		RED2PIXEL555(byte)
# define GREEN2PIXEL(byte)	GREEN2PIXEL555(byte)
# define BLUE2PIXEL(byte)	BLUE2PIXEL555(byte)
# define REDMASK(pixel)		((pixel) & 0x7c00)
# define GREENMASK(pixel)	((pixel) & 0x03e0)
# define BLUEMASK(pixel)		((pixel) & 0x001f)
#elif MWPIXEL_FORMAT == MWPF_TRUECOLOR1555
# define muldiv255_16bpp		muldiv255_rgb1555
# define RED2PIXEL(byte)		RED2PIXEL1555(byte)
# define GREEN2PIXEL(byte)	GREEN2PIXEL1555(byte)
# define BLUE2PIXEL(byte)	BLUE2PIXEL1555(byte)
# define REDMASK(pixel)		((pixel) & 0x001f)
# define GREENMASK(pixel)	((pixel) & 0x03e0)
# define BLUEMASK(pixel)		((pixel) & 0x7c00)
#else
# define muldiv255_16bpp		muldiv255_rgb565
# define RED2PIXEL(byte)		RED2PIXEL565(byte)
# define GREEN2PIXEL(byte)	GREEN2PIXEL565(byte)
# define BLUE2PIXEL(byte)	BLUE2PIXEL565(byte)
# define REDMASK(pixel)		((pixel) & 0xf800)
# define GREENMASK(pixel)	((pixel) & 0x07e0)
# define BLUEMASK(pixel)		((pixel) & 0x001f)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLORARGB
#define RGB2PIXEL(r,g,b)	RGB2PIXEL8888(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL8888(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL8888TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL8888RED(p)
#define PIXEL2GREEN(p)		PIXEL8888GREEN(p)
#define PIXEL2BLUE(p)		PIXEL8888BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLORABGR
#define RGB2PIXEL(r,g,b)	RGB2PIXELABGR(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXELABGR(c)
#define PIXELVALTOCOLORVAL(p)	PIXELABGRTOCOLORVAL(p)
#define PIXEL2RED(p)		PIXELABGRRED(p)
#define PIXEL2GREEN(p)		PIXELABGRGREEN(p)
#define PIXEL2BLUE(p)		PIXELABGRBLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLORRGB
#define RGB2PIXEL(r,g,b)	RGB2PIXEL888(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL888(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL888TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL888RED(p)
#define PIXEL2GREEN(p)		PIXEL888GREEN(p)
#define PIXEL2BLUE(p)		PIXEL888BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR565
#define RGB2PIXEL(r,g,b)	RGB2PIXEL565(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL565(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL565TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL565RED(p)
#define PIXEL2GREEN(p)		PIXEL565GREEN(p)
#define PIXEL2BLUE(p)		PIXEL565BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR555
#define RGB2PIXEL(r,g,b)	RGB2PIXEL555(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL555(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL555TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL555RED(p)
#define PIXEL2GREEN(p)		PIXEL555GREEN(p)
#define PIXEL2BLUE(p)		PIXEL555BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR1555
#define RGB2PIXEL(r,g,b)	RGB2PIXEL1555(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL1555(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL1555TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL1555RED(p)
#define PIXEL2GREEN(p)		PIXEL1555GREEN(p)
#define PIXEL2BLUE(p)		PIXEL1555BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR332
#define RGB2PIXEL(r,g,b)	RGB2PIXEL332(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL332(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL332TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL332RED(p)
#define PIXEL2GREEN(p)		PIXEL332GREEN(p)
#define PIXEL2BLUE(p)		PIXEL332BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_TRUECOLOR233
#define RGB2PIXEL(r,g,b)	RGB2PIXEL233(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXEL233(c)
#define PIXELVALTOCOLORVAL(p)	PIXEL233TOCOLORVAL(p)
#define PIXEL2RED(p)		PIXEL233RED(p)
#define PIXEL2GREEN(p)		PIXEL233GREEN(p)
#define PIXEL2BLUE(p)		PIXEL233BLUE(p)
#endif

#if MWPIXEL_FORMAT == MWPF_PALETTE
//only required for compiling in palette pixel size, not supported in convblits
#define RGB2PIXEL(r,g,b)	0
#endif

/* In-core color palette structure */
typedef struct {
	MWUCHAR	r;
	MWUCHAR	g;
	MWUCHAR	b;
	MWUCHAR _padding;
} MWPALENTRY;

/* Interface to Screen Device Driver */
typedef struct _mwscreendevice {
	MWCOORD	xvirtres;	/* X drawing res (will be flipped in portrait mode) */
	MWCOORD	yvirtres;	/* Y drawing res (will be flipped in portrait mode) */
	int		bpp;		/* # bpp*/
	int		portrait;	 /* screen portrait mode*/
	void	(*Update)(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height);
} SCREENDEVICE;

/* GdConversionBlit parameter structure */
typedef struct {
	int			op;				/* MWROP operation requested*/
	int			data_format;	/* MWIF_ image data format*/
	MWCOORD		width, height;	/* width and height for src and dest*/
	MWCOORD		dstx, dsty;		/* dest x, y*/
	MWCOORD		srcx, srcy;		/* source x, y*/
	unsigned int src_pitch;		/* source row length in bytes*/
	MWCOLORVAL	fg_colorval;	/* fg color, MWCOLORVAL 0xAARRGGBB format*/
	MWCOLORVAL	bg_colorval;
	uint32_t	fg_pixelval;	/* fg color, hw pixel format*/
	uint32_t	bg_pixelval;
	MWBOOL		usebg;			/* set =1 to draw background*/
	void *		data;			/* input image data GdConversionBlit*/

	/* these items filled in by GdConversionBlit*/
	void *		data_out;		/* output image from conversion blits subroutines*/
	unsigned int dst_pitch;		/* dest row length in bytes*/

	/* used by GdBlit and GdStretchBlit for GdCheckCursor and fallback blit*/
	PSD			srcpsd;			/* source psd for psd->psd blits*/

	/* used by frameblits only*/
	MWCOORD		src_xvirtres;	/* srcpsd->x/y virtres, used in frameblit for src coord rotation*/
	MWCOORD		src_yvirtres;

	/* used in stretch blits only*/
	int			src_x_step;		/* normal steps in source image*/
	int			src_y_step;
	int			src_x_step_one;	/* 1-unit steps in source image*/
	int			src_y_step_one;
	int			err_x_step;		/* 1-unit error steps in source image*/
	int			err_y_step;
	int			err_y;			/* source coordinate error tracking*/
	int			err_x;
	int			x_denominator;	/* denominator fraction*/
	int			y_denominator;

	/* used in palette conversions only*/
	MWPALENTRY *palette;		/* palette for image*/
	uint32_t	transcolor;		/* transparent color in image*/

//	PSD			alphachan;		/* alpha chan for MWROP_BLENDCHANNEL*/
} MWBLITPARMS, *PMWBLITPARMS;

typedef void (*MWBLITFUNC)(PSD, PMWBLITPARMS);		/* proto for blitter functions*/


/* convblit_8888.c*/

/* ----- 32bpp output -----*/
void convblit_srcover_rgba8888_rgba8888(PSD psd, PMWBLITPARMS gc);	// png/tiff 32bpp RGBA srcover
void convblit_copy_rgba8888_rgba8888(PSD psd, PMWBLITPARMS gc);		// 32bpp RGBA to 32bpp RGBA copy
void convblit_copy_rgb888_rgba8888(PSD psd, PMWBLITPARMS gc);		// png/jpg 24bpp RGB copy

void convblit_srcover_rgba8888_bgra8888(PSD psd, PMWBLITPARMS gc);	// png/tiff 32bpp RGBA srcover
void convblit_copy_rgba8888_bgra8888(PSD psd, PMWBLITPARMS gc);		// 32bpp RGBA to 32bpp BGRA copy
void convblit_copy_rgb888_bgra8888(PSD psd, PMWBLITPARMS gc);		// png/jpg 24bpp RGB copy

void convblit_copy_8888_8888(PSD psd, PMWBLITPARMS gc);				// 32bpp to 32bpp copy

/* ----- 24bpp output -----*/
void convblit_srcover_rgba8888_bgr888(PSD psd, PMWBLITPARMS gc);
void convblit_copy_rgba8888_bgr888(PSD psd, PMWBLITPARMS gc);		// 32bpp RGBA to 24bpp BGR copy
void convblit_copy_rgb888_bgr888(PSD psd, PMWBLITPARMS gc);

void convblit_copy_888_888(PSD psd, PMWBLITPARMS gc);				// 24bpp to 24bpp copy

void convblit_copy_bgra8888_bgr888(PSD psd, PMWBLITPARMS gc);		// 32bpp BGRA to 24bpp BGR copy

/* ----- 16bpp output -----*/
void convblit_srcover_rgba8888_16bpp(PSD psd, PMWBLITPARMS gc);
void convblit_copy_rgba8888_16bpp(PSD psd, PMWBLITPARMS gc);		// 32bpp RGBA to 16bpp copy
void convblit_copy_rgb888_16bpp(PSD psd, PMWBLITPARMS gc);

void convblit_copy_16bpp_16bpp(PSD psd, PMWBLITPARMS gc);			// 16bpp to 16bpp copy

/* convblit_mask.c*/
/* 1bpp and 8bpp (alphablend) mask conversion blits - for font display*/

/* ----- 32bpp output -----*/
void convblit_copy_mask_mono_byte_msb_rgba(PSD psd, PMWBLITPARMS gc);
void convblit_copy_mask_mono_byte_lsb_rgba(PSD psd, PMWBLITPARMS gc);
void convblit_copy_mask_mono_word_msb_rgba(PSD psd, PMWBLITPARMS gc);
void convblit_blend_mask_alpha_byte_rgba(PSD psd, PMWBLITPARMS gc);

void convblit_copy_mask_mono_byte_msb_bgra(PSD psd, PMWBLITPARMS gc);		/* ft2 non-alias*/
void convblit_copy_mask_mono_byte_lsb_bgra(PSD psd, PMWBLITPARMS gc);		/* t1lib non-alias*/
void convblit_copy_mask_mono_word_msb_bgra(PSD psd, PMWBLITPARMS gc);		/* pcf non-alias*/
void convblit_blend_mask_alpha_byte_bgra(PSD psd, PMWBLITPARMS gc);			/* ft2/t1lib alias*/

/* ----- 24bpp output -----*/
void convblit_copy_mask_mono_byte_msb_bgr(PSD psd, PMWBLITPARMS gc);
void convblit_copy_mask_mono_byte_lsb_bgr(PSD psd, PMWBLITPARMS gc);
void convblit_copy_mask_mono_word_msb_bgr(PSD psd, PMWBLITPARMS gc);
void convblit_blend_mask_alpha_byte_bgr(PSD psd, PMWBLITPARMS gc);

/* ----- 16bpp output -----*/
void convblit_copy_mask_mono_byte_msb_16bpp(PSD psd, PMWBLITPARMS gc);
void convblit_copy_mask_mono_byte_lsb_16bpp(PSD psd, PMWBLITPARMS gc);
void convblit_copy_mask_mono_word_msb_16bpp(PSD psd, PMWBLITPARMS gc);
void convblit_blend_mask_alpha_byte_16bpp(PSD psd, PMWBLITPARMS gc);

#if LATER
void convblit_copy_mask_mono_byte_msb_bgra_large(PSD psd, PMWBLITPARMS gc);	/* ft2 non-alias*/
void convblit_copy_mask_mono_byte_lsb_bgra_large(PSD psd, PMWBLITPARMS gc);	/* t1lib non-alias*/
#endif


/* convblit_frameb.c*/
/* framebuffer pixel format blits - must handle backwards copy, different rotation code*/
void frameblit_xxxa8888(PSD psd, PMWBLITPARMS gc);		/* 32bpp*/
void frameblit_24bpp(PSD psd, PMWBLITPARMS gc);			/* 24bpp*/
void frameblit_16bpp(PSD psd, PMWBLITPARMS gc);			/* 16bpp*/
void frameblit_8bpp(PSD psd, PMWBLITPARMS gc);			/* 8bpp*/

/* framebuffer pixel format stretch blits - different rotation code, no backwards copy*/
void frameblit_stretch_xxxa8888(PSD dstpsd, PMWBLITPARMS gc);	/* 32bpp, alpha in byte 4*/
void frameblit_stretch_24bpp(PSD psd, PMWBLITPARMS gc);			/* 24 bpp*/
void frameblit_stretch_16bpp(PSD psd, PMWBLITPARMS gc);			/* 16 bpp*/
void frameblit_stretch_8bpp(PSD psd, PMWBLITPARMS gc);			/* 8 bpp*/

/* these work for src_over and copy only*/
void frameblit_stretch_rgba8888_bgra8888(PSD psd, PMWBLITPARMS gc);	/* RGBA -> BGRA*/
void frameblit_stretch_rgba8888_bgr888(PSD psd, PMWBLITPARMS gc);	/* RGBA -> BGR*/
void frameblit_stretch_rgba8888_16bpp(PSD psd, PMWBLITPARMS gc);	/* RGBA -> 16bpp*/

/* devimage.c*/
void convblit_pal8_rgba8888(PMWBLITPARMS gc);
void convblit_pal4_msb_rgba8888(PMWBLITPARMS gc);
void convblit_pal1_byte_msb_rgba8888(PMWBLITPARMS gc);

/* image_bmp.c*/

/* Conversion blit 24bpp BGR to 24bpp RGB*/
void convblit_bgr888_rgb888(unsigned char *data, int width, int height, int pitch);
/* Conversion blit 32bpp BGRX to 32bpp RGBA 255 alpha*/
void convblit_bgrx8888_rgba8888(unsigned char *data, int width, int height, int pitch);

/* image_tiff.c*/

/* Conversion blit flip y direction 32bpp (upside-down)*/
void convblit_flipy_8888(PMWBLITPARMS gc);
