/*
 * ============================================================================
 *  EyeExpression.cpp ? Dynamic Expression Display Implementation
 *
 *  Self-contained module. Uses the existing OLED_Ruan driver's
 *  OLED_SetCursor() and OLED_WriteData() to flush a 128x64 framebuffer.
 *
 *  Ported from the eye_all_in_one_esp32 project (GitHub).
 *  Adapted from U8g2 drawing primitives to native framebuffer operations.
 * ============================================================================
 */

#include "EyeExpression.h"
#include "OLED_Ruan.h"
#include <Arduino.h>
#include <stdlib.h>

/* ========================================================================
 *  FRAMEBUFFER
 *  SSD1306 128x64: 8 pages * 128 bytes = 1024 bytes
 *  Page N = rows (N*8) .. (N*8+7).  Bit-0 = top pixel, Bit-7 = bottom.
 * ======================================================================== */

#define FB_SIZE  1024
static uint8_t fb[FB_SIZE];

static inline void fb_clear() {
    memset(fb, 0, FB_SIZE);
}

static inline void fb_setPixel(int16_t x, int16_t y, uint8_t color) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    uint16_t idx = ((uint16_t)(y >> 3) << 7) + (uint16_t)x;
    uint8_t  bit = 1 << (y & 0x07);
    if (color)
        fb[idx] |= bit;
    else
        fb[idx] &= ~bit;
}

/* ---- Flush entire framebuffer to OLED via the existing driver ---- */
static void fb_flush() {
    // OLED_ShowExpression is the public API; it does SetCursor + WriteData
    // page-by-page internally. Use it instead of calling OLED_WriteData directly.
    OLED_ShowExpression(fb, 128, 64, 0, 0);
}

/* ========================================================================
 *  DRAWING PRIMITIVES
 *
 *  These replicate what the original U8g2-using EyeDrawer expects:
 *    drawHLine(x, y, len)   ? horizontal line, 'len' pixels to the right
 *    drawBox(x, y, w, h)    ? filled rectangle (w pixels wide, h pixels tall)
 *    drawTriangle            ? filled triangle via scanlines
 *
 *  Coordinate convention: x increases right, y increases down.
 * ======================================================================== */

/* Draw horizontal line from (x,y) extending len pixels to the right. */
static void fb_drawHLine(int16_t x, int16_t y, int16_t len) {
    for (int16_t i = 0; i < len; i++)
        fb_setPixel(x + i, y, 1);
}

/* Filled rectangle. l=left, t=top, r=right, b=bottom (inclusive edges). */
static void fb_drawRect(int16_t l, int16_t t, int16_t r, int16_t b) {
    if (l > r) { int16_t tmp = l; l = r; r = tmp; }
    if (t > b) { int16_t tmp = t; t = b; b = tmp; }
    for (int16_t y = t; y <= b; y++)
        for (int16_t x = l; x <= r; x++)
            fb_setPixel(x, y, 1);
}

/* Filled box: (x0,y0) is top-left corner, w=width, h=height. */
static void fb_drawBox(int16_t x0, int16_t y0, int16_t w, int16_t h) {
    if (w <= 0 || h <= 0) return;
    fb_drawRect(x0, y0, x0 + w - 1, y0 + h - 1);
}

/* Scanline-filled general triangle. */
static void fb_drawTriangle(int16_t x0, int16_t y0,
                            int16_t x1, int16_t y1,
                            int16_t x2, int16_t y2) {
    /* Sort vertices by y: v0(top) -> v1(mid) -> v2(bottom) */
    if (y0 > y1) { int16_t t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }
    if (y1 > y2) { int16_t t = x1; x1 = x2; x2 = t; t = y1; y1 = y2; y2 = t; }
    if (y0 > y1) { int16_t t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }

    /* Degenerate */
    if (y2 <= y0) return;

    int16_t y;
    for (y = y0; y < y2; y++) {
        int16_t x_left, x_right;
        if (y < y1) {
            /* Upper half: between v0-v1 and v0-v2 */
            float t0 = (y1 == y0) ? 0.5f : (float)(y - y0) / (y1 - y0);
            float t1 = (y2 == y0) ? 0.5f : (float)(y - y0) / (y2 - y0);
            x_left  = (int16_t)(x0 + (x1 - x0) * t0);
            x_right = (int16_t)(x0 + (x2 - x0) * t1);
        } else {
            /* Lower half: between v1-v2 and v0-v2 */
            float t0 = (y2 == y1) ? 0.5f : (float)(y - y1) / (y2 - y1);
            float t1 = (y2 == y0) ? 0.5f : (float)(y - y0) / (y2 - y0);
            x_left  = (int16_t)(x1 + (x2 - x1) * t0);
            x_right = (int16_t)(x0 + (x2 - x0) * t1);
        }
        if (x_left > x_right) { int16_t t = x_left; x_left = x_right; x_right = t; }
        fb_drawHLine(x_left, y, x_right - x_left + 1);
    }
}

/* Right triangle: (x0,y0), (x1,y1) = legs end points, right angle at (x1,y0). */
static void fb_drawRightTriangle(int16_t x0, int16_t y0,
                                 int16_t x1, int16_t y1) {
    fb_drawTriangle(x0, y0, x1, y1, x1, y0);
}

/* ========================================================================
 *  FILL ELLIPSE CORNER  ?  Midpoint ellipse algorithm, ported from the
 *  original EyeDrawer::FillEllipseCorner. Draws a filled quarter-ellipse
 *  at one of four corners.
 *
 *  T_R: top-right  (x0 is left edge, quarter fills right/up)
 *  T_L: top-left   (x0 is right edge, quarter fills left/up)
 *  B_L: bottom-left  (x0 is right edge, fills left/down)
 *  B_R: bottom-right (x0 is left edge, fills right/down)
 * ======================================================================== */

enum EllCorner { EC_TR, EC_TL, EC_BR, EC_BL };

static void fb_fillEllipseCorner(EllCorner corner,
                                 int16_t x0, int16_t y0,
                                 int32_t rx, int32_t ry) {
    if (rx < 2 || ry < 2) return;
    int32_t rx2 = rx * rx;
    int32_t ry2 = ry * ry;
    int32_t fx2 = 4 * rx2;
    int32_t fy2 = 4 * ry2;
    int32_t x, y, s;

    switch (corner) {
    case EC_TR:
        /* First quadrant: x increases right, y decreases up */
        for (x = 0, y = ry, s = 2 * ry2 + rx2 * (1 - 2 * ry); ry2 * x <= rx2 * y; x++) {
            fb_drawHLine(x0, y0 - y, x);
            if (s >= 0) { s += fx2 * (1 - y); y--; }
            s += ry2 * ((4 * x) + 6);
        }
        for (x = rx, y = 0, s = 2 * rx2 + ry2 * (1 - 2 * rx); rx2 * y <= ry2 * x; y++) {
            fb_drawHLine(x0, y0 - y, x);
            if (s >= 0) { s += fy2 * (1 - x); x--; }
            s += rx2 * ((4 * y) + 6);
        }
        break;

    case EC_BR:
        /* Fourth quadrant: x increases right, y increases down */
        for (x = 0, y = ry, s = 2 * ry2 + rx2 * (1 - 2 * ry); ry2 * x <= rx2 * y; x++) {
            fb_drawHLine(x0, y0 + y - 1, x);
            if (s >= 0) { s += fx2 * (1 - y); y--; }
            s += ry2 * ((4 * x) + 6);
        }
        for (x = rx, y = 0, s = 2 * rx2 + ry2 * (1 - 2 * rx); rx2 * y <= ry2 * x; y++) {
            fb_drawHLine(x0, y0 + y - 1, x);
            if (s >= 0) { s += fy2 * (1 - x); x--; }
            s += rx2 * ((4 * y) + 6);
        }
        break;

    case EC_TL:
        /* Second quadrant: x decreases left, y decreases up */
        for (x = 0, y = ry, s = 2 * ry2 + rx2 * (1 - 2 * ry); ry2 * x <= rx2 * y; x++) {
            fb_drawHLine(x0 - x, y0 - y, x);
            if (s >= 0) { s += fx2 * (1 - y); y--; }
            s += ry2 * ((4 * x) + 6);
        }
        for (x = rx, y = 0, s = 2 * rx2 + ry2 * (1 - 2 * rx); rx2 * y <= ry2 * x; y++) {
            fb_drawHLine(x0 - x, y0 - y, x);
            if (s >= 0) { s += fy2 * (1 - x); x--; }
            s += rx2 * ((4 * y) + 6);
        }
        break;

    case EC_BL:
        /* Third quadrant: x decreases left, y increases down */
        for (x = 0, y = ry, s = 2 * ry2 + rx2 * (1 - 2 * ry); ry2 * x <= rx2 * y; x++) {
            fb_drawHLine(x0 - x, y0 + y - 1, x);
            if (s >= 0) { s += fx2 * (1 - y); y--; }
            s += ry2 * ((4 * x) + 6);
        }
        for (x = rx, y = 0, s = 2 * rx2 + ry2 * (1 - 2 * rx); rx2 * y <= ry2 * x; y++) {
            fb_drawHLine(x0 - x, y0 + y, x);
            if (s >= 0) { s += fy2 * (1 - x); x--; }
            s += rx2 * ((4 * y) + 6);
        }
        break;
    }
}

/* ========================================================================
 *  EYE DRAWER
 *
 *  Static helper that draws one eye given a center point and an EyeConfig.
 *  This is a direct port of the original EyeDrawer::Draw().
 *  IMPORTANT: the original code uses U8g2's drawHLine(x, y, len) where
 *  len is the number of pixels to the RIGHT from x. Our fb_drawHLine
 *  follows the same convention.
 * ======================================================================== */

static void eye_draw(int16_t centerX, int16_t centerY, const EyeConfig *cfg) {
    /* ---- local mutable copy (the original modifies config in-place) ---- */
    EyeConfig c = *cfg;

    /* Compute slope offsets */
    int32_t delta_y_top    = (int32_t)(c.Height * c.Slope_Top / 2.0f);
    int32_t delta_y_bottom = (int32_t)(c.Height * c.Slope_Bottom / 2.0f);
    int32_t totalHeight    = c.Height + delta_y_top - delta_y_bottom;

    /* Radius auto-correction if radii exceed height */
    if (c.Radius_Bottom > 0 && c.Radius_Top > 0 &&
        totalHeight - 1 < c.Radius_Bottom + c.Radius_Top) {
        int32_t crt = (int32_t)((float)c.Radius_Top * (totalHeight - 1)
                                / (c.Radius_Bottom + c.Radius_Top));
        int32_t crb = (int32_t)((float)c.Radius_Bottom * (totalHeight - 1)
                                / (c.Radius_Bottom + c.Radius_Top));
        c.Radius_Top    = crt >= 0 ? crt : 0;
        c.Radius_Bottom = crb >= 0 ? crb : 0;
    }

    /* Inside corner coordinates (before slope/radius) */
    int32_t TLc_y = centerY + c.OffsetY - c.Height/2 + c.Radius_Top - delta_y_top;
    int32_t TLc_x = centerX + c.OffsetX - c.Width/2  + c.Radius_Top;
    int32_t TRc_y = centerY + c.OffsetY - c.Height/2 + c.Radius_Top + delta_y_top;
    int32_t TRc_x = centerX + c.OffsetX + c.Width/2  - c.Radius_Top;
    int32_t BLc_y = centerY + c.OffsetY + c.Height/2 - c.Radius_Bottom - delta_y_bottom;
    int32_t BLc_x = centerX + c.OffsetX - c.Width/2  + c.Radius_Bottom;
    int32_t BRc_y = centerY + c.OffsetY + c.Height/2 - c.Radius_Bottom + delta_y_bottom;
    int32_t BRc_x = centerX + c.OffsetX + c.Width/2  - c.Radius_Bottom;

    /* Inner extents */
    int32_t min_c_x = TLc_x < BLc_x ? TLc_x : BLc_x;
    int32_t max_c_x = TRc_x > BRc_x ? TRc_x : BRc_x;
    int32_t min_c_y = TLc_y < TRc_y ? TLc_y : TRc_y;
    int32_t max_c_y = BLc_y > BRc_y ? BLc_y : BRc_y;

    /* ---- Fill eye center ---- */
    fb_drawRect(min_c_x, min_c_y, max_c_x, max_c_y);

    /* ---- Fill outwards to meet rounded corner edges ---- */
    fb_drawRect(TRc_x, TRc_y, BRc_x + c.Radius_Bottom, BRc_y);  /* Right  */
    fb_drawRect(TLc_x - c.Radius_Top, TLc_y, BLc_x, BLc_y);      /* Left   */
    fb_drawRect(TLc_x, TLc_y - c.Radius_Top, TRc_x, TRc_y);      /* Top    */
    fb_drawRect(BLc_x, BLc_y, BRc_x, BRc_y + c.Radius_Bottom);   /* Bottom */

    /* ---- Slanted top edges ---- */
    if (c.Slope_Top > 0) {
        fb_drawRightTriangle(TLc_x, TLc_y - c.Radius_Top,
                             TRc_x, TRc_y - c.Radius_Top + 1);
    } else if (c.Slope_Top < 0) {
        fb_drawRightTriangle(TRc_x, TRc_y - c.Radius_Top,
                             TLc_x, TLc_y - c.Radius_Top + 1);
    }

    /* ---- Slanted bottom edges ---- */
    if (c.Slope_Bottom > 0) {
        fb_drawRightTriangle(
            BRc_x + c.Radius_Bottom, BRc_y + c.Radius_Bottom,
            BLc_x - c.Radius_Bottom + 1, BLc_y + c.Radius_Bottom);
    } else if (c.Slope_Bottom < 0) {
        fb_drawRightTriangle(
            BLc_x - c.Radius_Bottom + 1, BLc_y + c.Radius_Bottom,
            BRc_x + c.Radius_Bottom, BRc_y + c.Radius_Bottom);
    }

    /* ---- Rounded corners ---- */
    if (c.Radius_Top > 0) {
        fb_fillEllipseCorner(EC_TL, TLc_x, TLc_y, c.Radius_Top, c.Radius_Top);
        fb_fillEllipseCorner(EC_TR, TRc_x, TRc_y, c.Radius_Top, c.Radius_Top);
    }
    if (c.Radius_Bottom > 0) {
        fb_fillEllipseCorner(EC_BL, BLc_x, BLc_y, c.Radius_Bottom, c.Radius_Bottom);
        fb_fillEllipseCorner(EC_BR, BRc_x, BRc_y, c.Radius_Bottom, c.Radius_Bottom);
    }
}

/* ========================================================================
 *  UTILITY: Linear interpolation between two EyeConfig values
 * ======================================================================== */

static EyeConfig lerp_config(const EyeConfig *a, const EyeConfig *b, float t) {
    EyeConfig r;
    r.OffsetX   = (int16_t)(a->OffsetX   + (b->OffsetX   - a->OffsetX)   * t);
    r.OffsetY   = (int16_t)(a->OffsetY   + (b->OffsetY   - a->OffsetY)   * t);
    r.Height    = (int16_t)(a->Height    + (b->Height    - a->Height)    * t);
    r.Width     = (int16_t)(a->Width     + (b->Width     - a->Width)     * t);
    r.Slope_Top    = a->Slope_Top    + (b->Slope_Top    - a->Slope_Top)    * t;
    r.Slope_Bottom = a->Slope_Bottom + (b->Slope_Bottom - a->Slope_Bottom) * t;
    r.Radius_Top    = (int16_t)(a->Radius_Top    + (b->Radius_Top    - a->Radius_Top)    * t);
    r.Radius_Bottom = (int16_t)(a->Radius_Bottom + (b->Radius_Bottom - a->Radius_Bottom) * t);
    r.Inverse_Radius_Top    = (int16_t)(a->Inverse_Radius_Top    + (b->Inverse_Radius_Top    - a->Inverse_Radius_Top)    * t);
    r.Inverse_Radius_Bottom = (int16_t)(a->Inverse_Radius_Bottom + (b->Inverse_Radius_Bottom - a->Inverse_Radius_Bottom) * t);
    r.Inverse_Offset_Top    = (int16_t)(a->Inverse_Offset_Top    + (b->Inverse_Offset_Top    - a->Inverse_Offset_Top)    * t);
    r.Inverse_Offset_Bottom = (int16_t)(a->Inverse_Offset_Bottom + (b->Inverse_Offset_Bottom - a->Inverse_Offset_Bottom) * t);
    return r;
}

/* ========================================================================
 *  EYE STRUCTURE
 *
 *  Each eye maintains:
 *    - Current interpolated config (config, target_config, transition_t)
 *    - Blink state
 *    - Look-at offsets (move_x, move_y, target_x, target_y)
 *    - Center position
 *    - is_left flag (for mirroring)
 * ======================================================================== */

#define TRANSITION_DURATION_MS  600

struct EyeState {
    EyeConfig config;
    EyeConfig target_config;
    unsigned long transition_start;
    bool is_left;

    /* Blink */
    float blink_t;           /* 0=open, 1=fully shut; driven by blink_phase */
    unsigned long blink_start;
    unsigned long blink_duration;
    bool blink_pending;

    /* Look-at */
    float move_x, move_y;
    float target_move_x, target_move_y;
    unsigned long look_start;

    /* Center on screen */
    int16_t cx, cy;
};

/* ========================================================================
 *  FACE STRUCTURE (manages two eyes + global state)
 * ======================================================================== */

struct FaceState {
    EyeState left;
    EyeState right;

    EyeEmotion current_emotion;

    bool random_blink;
    bool random_behavior;
    bool random_look;

    unsigned long next_blink_time;
    unsigned long next_emotion_time;
    unsigned long next_look_time;
};

static FaceState g_face;

/* ---- Lookup: emotion -> left/right preset pairs ---- */

struct PresetPair {
    const EyeConfig *left;   /* displayed by the RIGHT eye on screen (mirrored) */
    const EyeConfig *right;  /* displayed by the LEFT eye on screen (mirrored) */
};

static const PresetPair g_presets[EyeEmotion_Count] = {
    { &EyePreset_Normal,      &EyePreset_Normal      },  /* Normal       */
    { &EyePreset_Angry,       &EyePreset_Angry       },  /* Angry        */
    { &EyePreset_Glee,        &EyePreset_Glee        },  /* Glee         */
    { &EyePreset_Happy,       &EyePreset_Happy       },  /* Happy        */
    { &EyePreset_Sad,         &EyePreset_Sad         },  /* Sad          */
    { &EyePreset_Worried,     &EyePreset_Worried_Alt },  /* Worried      */
    { &EyePreset_Focused,     &EyePreset_Focused     },  /* Focused      */
    { &EyePreset_Annoyed,     &EyePreset_Annoyed_Alt },  /* Annoyed      */
    { &EyePreset_Surprised,   &EyePreset_Surprised   },  /* Surprised    */
    { &EyePreset_Skeptic,     &EyePreset_Skeptic_Alt },  /* Skeptic      */
    { &EyePreset_Frustrated,  &EyePreset_Frustrated  },  /* Frustrated   */
    { &EyePreset_Unimpressed, &EyePreset_Unimpressed_Alt },  /* Unimpressed */
    { &EyePreset_Sleepy,      &EyePreset_Sleepy_Alt  },  /* Sleepy       */
    { &EyePreset_Suspicious,  &EyePreset_Suspicious_Alt },  /* Suspicious  */
    { &EyePreset_Squint,      &EyePreset_Squint_Alt  },  /* Squint       */
    { &EyePreset_Furious,     &EyePreset_Furious     },  /* Furious      */
    { &EyePreset_Scared,      &EyePreset_Scared      },  /* Scared       */
    { &EyePreset_Awe,         &EyePreset_Awe         },  /* Awe          */
};

/* ========================================================================
 *  INTERNAL: Mirror an EyeConfig (swap X signs for left eye)
 * ======================================================================== */

static EyeConfig mirror_config(const EyeConfig *src) {
    EyeConfig c = *src;
    c.OffsetX = -c.OffsetX;
    c.Slope_Top = -c.Slope_Top;
    c.Slope_Bottom = -c.Slope_Bottom;
    return c;
}

/* ========================================================================
 *  INTERNAL: Set target config for one eye with mirroring
 * ======================================================================== */

static void eye_set_target(EyeState *eye, const EyeConfig *preset) {
    eye->target_config = eye->is_left ? mirror_config(preset) : *preset;
    eye->transition_start = millis();
}

/* ========================================================================
 *  INTERNAL: Update transition interpolation for one eye
 * ======================================================================== */

static void eye_update_transition(EyeState *eye) {
    unsigned long elapsed = millis() - eye->transition_start;
    float t = (elapsed >= TRANSITION_DURATION_MS) ? 1.0f
              : (float)elapsed / TRANSITION_DURATION_MS;
    eye->config = lerp_config(&eye->config, &eye->target_config, t);
}

/* ========================================================================
 *  INTERNAL: Update look-at interpolation for one eye
 * ======================================================================== */

static void eye_update_look(EyeState *eye) {
    const unsigned long look_duration = 500;
    unsigned long elapsed = millis() - eye->look_start;
    float t = (elapsed >= look_duration) ? 1.0f
              : (float)elapsed / look_duration;
    /* Smooth step */
    t = t * t * (3.0f - 2.0f * t);
    eye->move_x = eye->move_x + (eye->target_move_x - eye->move_x) * t;
    eye->move_y = eye->move_y + (eye->target_move_y - eye->move_y) * t;
}

/* ========================================================================
 *  RENDER: Draw one eye (with blink, look-at offset applied)
 * ======================================================================== */

static void eye_render(EyeState *eye) {
    EyeConfig draw_cfg = eye->config;

    /* Apply look-at offset */
    draw_cfg.OffsetX += (int16_t)eye->move_x;
    draw_cfg.OffsetY -= (int16_t)eye->move_y;

    /* Apply blink squash */
    if (eye->blink_t > 0.0f) {
        /* Squash height, anti-squash width for realism */
        int16_t h = draw_cfg.Height;
        int16_t w = draw_cfg.Width;
        float bt = eye->blink_t;
        if (bt > 1.0f) bt = 1.0f;
        /* Blink curve: fast close, slow open (asymmetric) */
        float squash = bt;
        draw_cfg.Height = (int16_t)(h * (1.0f - squash * 0.95f));
        if (draw_cfg.Height < 2) draw_cfg.Height = 2;
        draw_cfg.Width  = (int16_t)(w * (1.0f + squash * 0.15f));
    }

    eye_draw(eye->cx, eye->cy, &draw_cfg);
}

/* ========================================================================
 *  INTERNAL: Tick blink animation
 * ======================================================================== */

static void eye_update_blink(EyeState *eye) {
    if (!eye->blink_pending) return;

    unsigned long elapsed = millis() - eye->blink_start;
    unsigned long dur = eye->blink_duration;

    if (elapsed >= dur) {
        eye->blink_t = 0.0f;
        eye->blink_pending = false;
    } else {
        float t = (float)elapsed / dur;
        /* Asymmetric: close in first 30%, hold 5%, open in remaining 65% */
        if (t < 0.30f) {
            eye->blink_t = t / 0.30f;
        } else if (t < 0.35f) {
            eye->blink_t = 1.0f;
        } else {
            eye->blink_t = 1.0f - (t - 0.35f) / 0.65f;
        }
        if (eye->blink_t < 0.0f) eye->blink_t = 0.0f;
        if (eye->blink_t > 1.0f) eye->blink_t = 1.0f;
    }
}

/* ========================================================================
 *  INTERNAL: Trigger blink on an eye
 * ======================================================================== */

static void eye_trigger_blink(EyeState *eye, unsigned long duration_ms) {
    eye->blink_start = millis();
    eye->blink_duration = duration_ms;
    eye->blink_pending = true;
}

/* ========================================================================
 *  INTERNAL: Set look-at target for an eye
 * ======================================================================== */

static void eye_set_look(EyeState *eye, float mx, float my) {
    eye->target_move_x = eye->is_left ? -mx : mx;
    eye->target_move_y = my;
    eye->look_start = millis();
}

/* ========================================================================
 *  PUBLIC API
 * ======================================================================== */

void EyeExpression_Init() {
    memset(&g_face, 0, sizeof(g_face));

    /* Left eye = on-screen left (mirrored), Right eye = on-screen right */
    g_face.left.is_left  = true;
    g_face.right.is_left = false;

    /* Position eyes on screen:
     *   128x64 display, eyes centered vertically at y=32
     *   Left eye center: x=32, Right eye center: x=96
     *   (40px between centers, 24px margins on edges)
     */
    g_face.left.cx  = 32;
    g_face.left.cy  = 32;
    g_face.right.cx = 96;
    g_face.right.cy = 32;

    g_face.random_blink    = true;
    g_face.random_look     = true;
    g_face.random_behavior = false;

    g_face.next_blink_time   = millis() + random(5000, 12000);
    g_face.next_emotion_time = 0;
    g_face.next_look_time    = millis() + random(3000, 7000);

    /* Initialize with Normal expression */
    const PresetPair *pp = &g_presets[EyeEmotion_Normal];
    g_face.left.config  = g_face.left.is_left  ? mirror_config(pp->left)  : *pp->left;
    g_face.left.target_config  = g_face.left.config;
    g_face.right.config = g_face.right.is_left ? mirror_config(pp->right) : *pp->right;
    g_face.right.target_config = g_face.right.config;
    g_face.current_emotion = EyeEmotion_Normal;
}

void EyeExpression_Update() {
    EyeState *left  = &g_face.left;
    EyeState *right = &g_face.right;
    unsigned long now = millis();

    /* ---- Transitions ---- */
    eye_update_transition(left);
    eye_update_transition(right);

    /* ---- Look-at ---- */
    eye_update_look(left);
    eye_update_look(right);

    /* ---- Blink ---- */
    eye_update_blink(left);
    eye_update_blink(right);

    /* ---- Random blink ---- */
    if (g_face.random_blink && now >= g_face.next_blink_time) {
        eye_trigger_blink(left,  300);
        eye_trigger_blink(right, 300);
        g_face.next_blink_time = now + random(5000, 12000);
    }

    /* ---- Random look ---- */
    if (g_face.random_look && now >= g_face.next_look_time) {
        float lx = (float)(random(-80, 80)) / 100.0f;
        float ly = (float)(random(-60, 60)) / 100.0f;
        eye_set_look(left,  lx, ly);
        eye_set_look(right, lx, ly);
        g_face.next_look_time = now + random(5000, 12000);
    }

    /* ---- Random emotion ---- */
    if (g_face.random_behavior && g_face.next_emotion_time > 0
        && now >= g_face.next_emotion_time) {
        EyeEmotion e = (EyeEmotion)random(0, (int)EyeEmotion_Count);
        EyeExpression_SetEmotion(e);
        g_face.next_emotion_time = now + random(5000, 12000);
    }

    /* ---- Render ---- */
    fb_clear();
    eye_render(left);
    eye_render(right);
    fb_flush();
}

void EyeExpression_SetEmotion(EyeEmotion emotion) {
    if (emotion >= EyeEmotion_Count) return;
    g_face.current_emotion = emotion;
    const PresetPair *pp = &g_presets[emotion];
    eye_set_target(&g_face.left,  pp->left);
    eye_set_target(&g_face.right, pp->right);
}

void EyeExpression_Blink() {
    eye_trigger_blink(&g_face.left,  150);
    eye_trigger_blink(&g_face.right, 150);
}

void EyeExpression_LookAt(float x, float y) {
    /* Clamp */
    if (x < -1.0f) x = -1.0f; if (x > 1.0f) x = 1.0f;
    if (y < -1.0f) y = -1.0f; if (y > 1.0f) y = 1.0f;
    /* Scale: 25px max horizontal, 20px max vertical offset */
    eye_set_look(&g_face.left,  x * 12.0f, y * 10.0f);
    eye_set_look(&g_face.right, x * 12.0f, y * 10.0f);
}

void EyeExpression_SetRandomBehavior(bool enable) {
    g_face.random_behavior = enable;
    if (enable) {
        g_face.next_emotion_time = millis() + random(5000, 12000);
    } else {
        g_face.next_emotion_time = 0;
    }
}

void EyeExpression_SetRandomBlink(bool enable) {
    g_face.random_blink = enable;
    if (enable) {
        g_face.next_blink_time = millis() + random(2000, 5000);
    }
}

void EyeExpression_SetRandomLook(bool enable) {
    g_face.random_look = enable;
    if (enable) {
        g_face.next_look_time = millis() + random(2000, 4000);
    }
}

EyeEmotion EyeExpression_GetCurrentEmotion() {
    return g_face.current_emotion;
}
