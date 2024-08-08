#ifndef __CG_H__
#define __CG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xft.h"

#define real_t float

#define _0 (real_t)0
#define _1 (real_t)1

#define REAL(v) (real_t)v

struct cg_point_t {
    real_t x;
    real_t y;
};

struct cg_rect_t {
    real_t x;
    real_t y;
    real_t w;
    real_t h;
};

struct cg_matrix_t {
    real_t a; real_t b;
    real_t c; real_t d;
    real_t tx; real_t ty;
};

struct cg_color_t {
    real_t r;
    real_t g;
    real_t b;
    real_t a;
};

struct cg_gradient_stop_t {
    real_t offset;
    struct cg_color_t color;
};

enum cg_path_element_t {
    CG_PATH_ELEMENT_MOVE_TO = 0,
    CG_PATH_ELEMENT_LINE_TO = 1,
    CG_PATH_ELEMENT_CURVE_TO = 2,
    CG_PATH_ELEMENT_CLOSE = 3,
};

enum cg_spread_method_t {
    CG_SPREAD_METHOD_PAD = 0,
    CG_SPREAD_METHOD_REFLECT = 1,
    CG_SPREAD_METHOD_REPEAT = 2,
};

enum cg_gradient_type_t {
    CG_GRADIENT_TYPE_LINEAR = 0,
    CG_GRADIENT_TYPE_RADIAL = 1,
};

enum cg_texture_type_t {
    CG_TEXTURE_TYPE_PLAIN = 0,
    CG_TEXTURE_TYPE_TILED = 1,
};

enum cg_line_cap_t {
    CG_LINE_CAP_BUTT = 0,
    CG_LINE_CAP_ROUND = 1,
    CG_LINE_CAP_SQUARE = 2,
};

enum cg_line_join_t {
    CG_LINE_JOIN_MITER = 0,
    CG_LINE_JOIN_ROUND = 1,
    CG_LINE_JOIN_BEVEL = 2,
};

enum cg_fill_rule_t {
    CG_FILL_RULE_NON_ZERO = 0,
    CG_FILL_RULE_EVEN_ODD = 1,
};

enum cg_paint_type_t {
    CG_PAINT_TYPE_COLOR = 0,
    CG_PAINT_TYPE_GRADIENT = 1,
    CG_PAINT_TYPE_TEXTURE = 2,
};

enum cg_operator_t {
    CG_OPERATOR_SRC = 0, /* r = s * ca + d * cia */
    CG_OPERATOR_SRC_OVER = 1, /* r = (s + d * sia) * ca + d * cia */
    CG_OPERATOR_DST_IN = 2, /* r = d * sa * ca + d * cia */
    CG_OPERATOR_DST_OUT = 3, /* r = d * sia * ca + d * cia */
};

struct cg_surface_t {
    int ref;
    int width;
    int height;
    int stride;
    int owndata;
    void *pixels;
};

struct cg_path_t {
    int contours;
    struct cg_point_t start;
    struct {
        enum cg_path_element_t * data;
        int size;
        int capacity;
    } elements;
    struct {
        struct cg_point_t * data;
        int size;
        int capacity;
    } points;
};

struct cg_gradient_t {
    enum cg_gradient_type_t type;
    enum cg_spread_method_t spread;
    struct cg_matrix_t matrix;
    real_t values[6];
    real_t opacity;
    struct {
        struct cg_gradient_stop_t * data;
        int size;
        int capacity;
    } stops;
};

struct cg_texture_t {
    enum cg_texture_type_t type;
    struct cg_surface_t * surface;
    struct cg_matrix_t matrix;
    real_t opacity;
};

struct cg_paint_t {
    enum cg_paint_type_t type;
    struct cg_color_t color;
    struct cg_gradient_t gradient;
    struct cg_texture_t texture;
};

struct cg_span_t {
    int x;
    int len;
    int y;
    unsigned char coverage;
};

struct cg_rle_t {
    struct {
        struct cg_span_t * data;
        int size;
        int capacity;
    } spans;
    int x;
    int y;
    int w;
    int h;
};

struct cg_dash_t {
    real_t offset;
    real_t * data;
    int size;
};

struct cg_stroke_data_t {
    real_t width;
    real_t miterlimit;
    enum cg_line_cap_t cap;
    enum cg_line_join_t join;
    struct cg_dash_t * dash;
};

struct cg_state_t {
    struct cg_rle_t * clippath;
    struct cg_paint_t paint;
    struct cg_matrix_t matrix;
    enum cg_fill_rule_t winding;
    struct cg_stroke_data_t stroke;
    enum cg_operator_t op;
    real_t opacity;
    struct cg_state_t * next;
};

struct cg_ctx_t {
    struct cg_surface_t * surface;
    struct cg_state_t * state;
    struct cg_path_t * path;
    struct cg_rle_t * rle;
    struct cg_rle_t * clippath;
    struct cg_rect_t clip;
    void * outline_data;
    size_t outline_size;
};

#ifndef CG_MIN
#define CG_MIN(a, b) ({typeof(a) _amin = (a); typeof(b) _bmin = (b); (void)(&_amin == &_bmin); _amin < _bmin ? _amin : _bmin;})
#endif
#ifndef CG_MAX
#define CG_MAX(a, b) ({typeof(a) _amax = (a); typeof(b) _bmax = (b); (void)(&_amax == &_bmax); _amax > _bmax ? _amax : _bmax;})
#endif
#ifndef CG_CLAMP
#define CG_CLAMP(v, a, b) CG_MIN(CG_MAX(a, v), b)
#endif
#ifndef CG_ALPHA
#define CG_ALPHA(c) ((c) >> 24)
#endif
#ifndef CG_DIV255
#define CG_DIV255(x) (((x) + ((x) >> 8) + 0x80) >> 8)
#endif
#ifndef CG_BYTE_MUL
#define CG_BYTE_MUL(x, a) ((((((x) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) + (((((x) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff))
#endif

void cg_memfill32(uint32_t * dst, uint32_t val, int len);
void cg_comp_solid_source(uint32_t * dst, int len, uint32_t color, uint32_t alpha);
void cg_comp_solid_source_over(uint32_t * dst, int len, uint32_t color, uint32_t alpha);
void cg_comp_solid_destination_in(uint32_t * dst, int len, uint32_t color, uint32_t alpha);
void cg_comp_solid_destination_out(uint32_t * dst, int len, uint32_t color, uint32_t alpha);
void cg_comp_source(uint32_t * dst, int len, uint32_t * src, uint32_t alpha);
void cg_comp_source_over(uint32_t * dst, int len, uint32_t * src, uint32_t alpha);
void cg_comp_destination_in(uint32_t * dst, int len, uint32_t * src, uint32_t alpha);
void cg_comp_destination_out(uint32_t * dst, int len, uint32_t * src, uint32_t alpha);

void cg_matrix_init(struct cg_matrix_t * m, real_t a, real_t b, real_t c, real_t d, real_t tx, real_t ty);
void cg_matrix_init_identity(struct cg_matrix_t * m);
void cg_matrix_init_translate(struct cg_matrix_t * m, real_t tx, real_t ty);
void cg_matrix_init_scale(struct cg_matrix_t * m, real_t sx, real_t sy);
void cg_matrix_init_rotate(struct cg_matrix_t * m, real_t r);
void cg_matrix_translate(struct cg_matrix_t * m, real_t tx, real_t ty);
void cg_matrix_scale(struct cg_matrix_t * m, real_t sx, real_t sy);
void cg_matrix_rotate(struct cg_matrix_t * m, real_t r);
void cg_matrix_multiply(struct cg_matrix_t * m, struct cg_matrix_t * m1, struct cg_matrix_t * m2);
void cg_matrix_invert(struct cg_matrix_t * m);
void cg_matrix_map_point(struct cg_matrix_t * m, struct cg_point_t * p1, struct cg_point_t * p2);

struct cg_surface_t * cg_surface_create(int width, int height);
struct cg_surface_t * cg_surface_create_for_data(int width, int height, void * pixels);
void cg_surface_destroy(struct cg_surface_t * surface);
struct cg_surface_t * cg_surface_reference(struct cg_surface_t * surface);

void cg_gradient_set_values_linear(struct cg_gradient_t * gradient, real_t x1, real_t y1, real_t x2, real_t y2);
void cg_gradient_set_values_radial(struct cg_gradient_t * gradient, real_t cx, real_t cy, real_t cr, real_t fx, real_t fy, real_t fr);
void cg_gradient_set_spread(struct cg_gradient_t * gradient, enum cg_spread_method_t spread);
void cg_gradient_set_matrix(struct cg_gradient_t * gradient, struct cg_matrix_t * m);
void cg_gradient_set_opacity(struct cg_gradient_t * gradient, real_t opacity);
void cg_gradient_add_stop_rgb(struct cg_gradient_t * gradient, real_t offset, real_t r, real_t g, real_t b);
void cg_gradient_add_stop_rgba(struct cg_gradient_t * gradient, real_t offset, real_t r, real_t g, real_t b, real_t a);
void cg_gradient_add_stop_color(struct cg_gradient_t * gradient, real_t offset, struct cg_color_t * color);
void cg_gradient_add_stop(struct cg_gradient_t * gradient, struct cg_gradient_stop_t * stop);
void cg_gradient_clear_stops(struct cg_gradient_t * gradient);

void cg_texture_set_type(struct cg_texture_t * texture, enum cg_texture_type_t type);
void cg_texture_set_matrix(struct cg_texture_t * texture, struct cg_matrix_t * m);
void cg_texture_set_surface(struct cg_texture_t * texture, struct cg_surface_t * surface);
void cg_texture_set_opacity(struct cg_texture_t * texture, real_t opacity);

struct cg_ctx_t * cg_create(struct cg_surface_t * surface);
void cg_destroy(struct cg_ctx_t * ctx);
void cg_save(struct cg_ctx_t * ctx);
void cg_restore(struct cg_ctx_t * ctx);
struct cg_color_t * cg_set_source_rgb(struct cg_ctx_t * ctx, real_t r, real_t g, real_t b);
struct cg_color_t * cg_set_source_rgba(struct cg_ctx_t * ctx, real_t r, real_t g, real_t b, real_t a);
struct cg_color_t * cg_set_source_color(struct cg_ctx_t * ctx, struct cg_color_t * color);
struct cg_gradient_t * cg_set_source_linear_gradient(struct cg_ctx_t * ctx, real_t x1, real_t y1, real_t x2, real_t y2);
struct cg_gradient_t * cg_set_source_radial_gradient(struct cg_ctx_t * ctx, real_t cx, real_t cy, real_t cr, real_t fx, real_t fy, real_t fr);
struct cg_texture_t * cg_set_source_surface(struct cg_ctx_t * ctx, struct cg_surface_t * surface, real_t x, real_t y);
void cg_set_operator(struct cg_ctx_t * ctx, enum cg_operator_t op);
void cg_set_opacity(struct cg_ctx_t * ctx, real_t opacity);
void cg_set_fill_rule(struct cg_ctx_t * ctx, enum cg_fill_rule_t winding);
void cg_set_line_width(struct cg_ctx_t * ctx, real_t width);
void cg_set_line_cap(struct cg_ctx_t * ctx, enum cg_line_cap_t cap);
void cg_set_line_join(struct cg_ctx_t * ctx, enum cg_line_join_t join);
void cg_set_miter_limit(struct cg_ctx_t * ctx, real_t limit);
void cg_set_dash(struct cg_ctx_t * ctx, real_t * dashes, int ndash, real_t offset);
void cg_translate(struct cg_ctx_t * ctx, real_t tx, real_t ty);
void cg_scale(struct cg_ctx_t * ctx, real_t sx, real_t sy);
void cg_rotate(struct cg_ctx_t * ctx, real_t r);
void cg_transform(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
void cg_set_matrix(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
void cg_identity_matrix(struct cg_ctx_t * ctx);
void cg_move_to(struct cg_ctx_t * ctx, real_t x, real_t y);
void cg_line_to(struct cg_ctx_t * ctx, real_t x, real_t y);
void cg_curve_to(struct cg_ctx_t * ctx, real_t x1, real_t y1, real_t x2, real_t y2, real_t x3, real_t y3);
void cg_quad_to(struct cg_ctx_t * ctx, real_t x1, real_t y1, real_t x2, real_t y2);
void cg_rel_move_to(struct cg_ctx_t * ctx, real_t dx, real_t dy);
void cg_rel_line_to(struct cg_ctx_t * ctx, real_t dx, real_t dy);
void cg_rel_curve_to(struct cg_ctx_t * ctx, real_t dx1, real_t dy1, real_t dx2, real_t dy2, real_t dx3, real_t dy3);
void cg_rel_quad_to(struct cg_ctx_t * ctx, real_t dx1, real_t dy1, real_t dx2, real_t dy2);
void cg_rectangle(struct cg_ctx_t * ctx, real_t x, real_t y, real_t w, real_t h);
void cg_round_rectangle(struct cg_ctx_t * ctx, real_t x, real_t y, real_t w, real_t h, real_t rx, real_t ry);
void cg_ellipse(struct cg_ctx_t * ctx, real_t cx, real_t cy, real_t rx, real_t ry);
void cg_circle(struct cg_ctx_t * ctx, real_t cx, real_t cy, real_t r);
void cg_arc(struct cg_ctx_t * ctx, real_t cx, real_t cy, real_t r, real_t a0, real_t a1);
void cg_arc_negative(struct cg_ctx_t * ctx, real_t cx, real_t cy, real_t r, real_t a0, real_t a1);
void cg_new_path(struct cg_ctx_t * ctx);
void cg_close_path(struct cg_ctx_t * ctx);
void cg_reset_clip(struct cg_ctx_t * ctx);
void cg_clip(struct cg_ctx_t * ctx);
void cg_clip_preserve(struct cg_ctx_t * ctx);
void cg_fill(struct cg_ctx_t * ctx);
void cg_fill_preserve(struct cg_ctx_t * ctx);
void cg_stroke(struct cg_ctx_t * ctx);
void cg_stroke_preserve(struct cg_ctx_t * ctx);
void cg_paint(struct cg_ctx_t * ctx);

#ifdef __cplusplus
}
#endif

#endif /* __CG_H__ */
