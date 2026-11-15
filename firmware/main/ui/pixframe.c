#include "pixframe.h"

#include "theme.h"

#define GLOW_RINGS 2

pixframe_t pixframe_default(void)
{
    return (pixframe_t){
        .border = COL_ORANGE,
        .fill = COL_CARD_BG,
        .fill_pressed = COL_CARD_BG,
        .glow = COL_GLOW,
        .border_opa = LV_OPA_COVER,
        .fill_opa = LV_OPA_COVER,
        .glow_opa = 0,
        .px = 3,
        .steps = 3,
    };
}

static void rect(lv_layer_t *layer, lv_draw_rect_dsc_t *d, int32_t x, int32_t y, int32_t w, int32_t h)
{
    if (w <= 0 || h <= 0) {
        return;
    }
    const lv_area_t a = {x, y, x + w - 1, y + h - 1};
    lv_draw_rect(layer, d, &a);
}

/** Outline (or filled interior) of a stepped rectangle covering area a. */
static void draw_shape(lv_layer_t *layer, lv_draw_rect_dsc_t *d, const lv_area_t *a, int p, int r, bool interior)
{
    const int32_t x1 = a->x1, y1 = a->y1;
    const int32_t W = lv_area_get_width(a), H = lv_area_get_height(a);
    const int32_t x2 = x1 + W, y2 = y1 + H; // exclusive
    if (r < 1) r = 1;                       // the corner pixel itself is always cut

    if (interior) {
        // centre block, then one band per corner step (rows 1..r-1 top and bottom)
        rect(layer, d, x1 + p, y1 + r * p, W - 2 * p, H - 2 * r * p);
        for (int k = 1; k < r; k++) {
            const int inset = (r - k + 1) * p;
            rect(layer, d, x1 + inset, y1 + k * p, W - 2 * inset, p);
            rect(layer, d, x1 + inset, y2 - (k + 1) * p, W - 2 * inset, p);
        }
        return;
    }
    // straight edges
    rect(layer, d, x1 + r * p, y1, W - 2 * r * p, p);
    rect(layer, d, x1 + r * p, y2 - p, W - 2 * r * p, p);
    rect(layer, d, x1, y1 + r * p, p, H - 2 * r * p);
    rect(layer, d, x2 - p, y1 + r * p, p, H - 2 * r * p);
    // diagonal staircase in each corner
    for (int k = 1; k < r; k++) {
        const int dx = (r - k) * p, dy = k * p;
        rect(layer, d, x1 + dx, y1 + dy, p, p);
        rect(layer, d, x2 - dx - p, y1 + dy, p, p);
        rect(layer, d, x1 + dx, y2 - dy - p, p, p);
        rect(layer, d, x2 - dx - p, y2 - dy - p, p, p);
    }
}

static void on_draw(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    pixframe_t *pf = lv_event_get_user_data(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t a;
    lv_obj_get_coords(obj, &a);

    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.radius = 0;

    if (pf->glow_opa > 0) {
        for (int i = GLOW_RINGS; i >= 1; i--) {
            lv_area_t g = a;
            lv_area_increase(&g, i * pf->px, i * pf->px);
            d.bg_color = pf->glow;
            d.bg_opa = (lv_opa_t)(pf->glow_opa / i);
            draw_shape(layer, &d, &g, pf->px, pf->steps + i, false);
        }
    }
    if (pf->fill_opa > 0) {
        d.bg_color = lv_obj_has_state(obj, LV_STATE_PRESSED) ? pf->fill_pressed : pf->fill;
        d.bg_opa = pf->fill_opa;
        draw_shape(layer, &d, &a, pf->px, pf->steps, true);
    }
    if (pf->border_opa > 0) {
        d.bg_color = pf->border;
        d.bg_opa = pf->border_opa;
        draw_shape(layer, &d, &a, pf->px, pf->steps, false);
    }
}

static void on_ext_size(lv_event_t *e)
{
    pixframe_t *pf = lv_event_get_user_data(e);
    if (pf->glow_opa > 0) {
        lv_event_set_ext_draw_size(e, GLOW_RINGS * pf->px);
    }
}

void pixframe_attach(lv_obj_t *obj, pixframe_t *pf)
{
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_outline_width(obj, 0, 0);
    lv_obj_add_event_cb(obj, on_draw, LV_EVENT_DRAW_MAIN, pf);
    lv_obj_add_event_cb(obj, on_ext_size, LV_EVENT_REFR_EXT_DRAW_SIZE, pf);
    lv_obj_refresh_ext_draw_size(obj);
}

void pixframe_refresh(lv_obj_t *obj)
{
    lv_obj_refresh_ext_draw_size(obj);
    lv_obj_invalidate(obj);
}
