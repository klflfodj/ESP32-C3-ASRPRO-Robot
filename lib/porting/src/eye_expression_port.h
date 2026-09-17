#pragma once

/*
 * ============================================================================
 *  EyeExpression - Dynamic Expression Display Module (128x64 SSD1306 OLED)
 * ============================================================================
 *
 *  Overview:
 *    Draws animatable vector-style cartoon eyes on a 128x64 SSD1306 OLED,
 *    supporting 18 preset emotions, smooth transitions, random blinking,
 *    and gaze following.
 *
 *  Dependencies:
 *    Requires "OLED_Ruan.h" for OLED_SetCursor() / OLED_WriteData() / OLED_Clear()
 *    and "OLED_Font.h" for OLED_F8x16 font data.
 *
 *  Does NOT modify any existing driver code (OELD_Ruan.cpp / OLED_Ruan.h /
 *  OLED_Font.h) and does NOT modify Rotsssss.ino. All logic is self-contained.
 *
 * ============================================================================
 *  QUICK START
 * ============================================================================
 *
 *  1. Copy EyeExpression.h and EyeExpression.cpp into your project folder.
 *
 *  2. Add one line at the top of Rotsssss.ino:
 *         #include "EyeExpression.h"
 *
 *  3. In setup(), after OLED_Init():
 *         EyeExpression_Init();
 *         EyeExpression_SetEmotion(EyeEmotion_Normal);
 *
 *  4. Create a FreeRTOS task to drive the animation, e.g. in setup():
 *
 *         xTaskCreate(
 *             [](void*) {
 *                 while(1) {
 *                     EyeExpression_Update();
 *                     vTaskDelay(pdMS_TO_TICKS(40));  // ~25 FPS
 *                 }
 *             },
 *             "Eyes", 4096, NULL, 2, NULL
 *         );
 *
 *     (If TaskOLED is running simultaneously, OLED_Clear will overwrite
 *     the expression screen. Run one at a time, or use a mutex to
 *     coordinate screen access.)
 *
 *  5. Switch emotions at any time:
 *         EyeExpression_SetEmotion(EyeEmotion_Happy);
 *         EyeExpression_Blink();
 *
 * ============================================================================
 *  API REFERENCE
 * ============================================================================
 *
 *  void EyeExpression_Init()
 *      Initialize the expression system (call once first).
 *
 *  void EyeExpression_Update()
 *      Update one animation frame and render to OLED. Call repeatedly in a
 *      timer loop or FreeRTOS task.
 *
 *  void EyeExpression_SetEmotion(EyeEmotion emotion)
 *      Switch to the given emotion with a smooth transition animation.
 *      See the EyeEmotion enum below for valid values.
 *
 *  void EyeExpression_Blink()
 *      Manually trigger a single blink.
 *
 *  void EyeExpression_LookAt(float x, float y)
 *      Direct the gaze to (x, y). x=-1 far left, x=+1 far right,
 *      y=-1 far down, y=+1 far up.
 *
 *  void EyeExpression_SetRandomBehavior(bool enable)
 *      Enable/disable random emotion switching (default: off).
 *
 *  void EyeExpression_SetRandomBlink(bool enable)
 *      Enable/disable random blinking (default: on).
 *
 *  void EyeExpression_SetRandomLook(bool enable)
 *      Enable/disable random gaze movement (default: on).
 *
 *  EyeEmotion EyeExpression_GetCurrentEmotion()
 *      Return the current emotion enum value.
 *
 * ============================================================================
 */

#include <stdint.h>

/* ---- Eye shape configuration (ported from the original project) ---- */
struct EyeConfig {
    int16_t OffsetX;
    int16_t OffsetY;
    int16_t Height;
    int16_t Width;
    float   Slope_Top;
    float   Slope_Bottom;
    int16_t Radius_Top;
    int16_t Radius_Bottom;
    int16_t Inverse_Radius_Top;
    int16_t Inverse_Radius_Bottom;
    int16_t Inverse_Offset_Top;
    int16_t Inverse_Offset_Bottom;
};

/* ---- Emotion enum ---- */
enum EyeEmotion : uint8_t {
    EyeEmotion_Normal      = 0,
    EyeEmotion_Angry       = 1,
    EyeEmotion_Glee        = 2,
    EyeEmotion_Happy       = 3,
    EyeEmotion_Sad         = 4,
    EyeEmotion_Worried     = 5,
    EyeEmotion_Focused     = 6,
    EyeEmotion_Annoyed     = 7,
    EyeEmotion_Surprised   = 8,
    EyeEmotion_Skeptic     = 9,
    EyeEmotion_Frustrated  = 10,
    EyeEmotion_Unimpressed = 11,
    EyeEmotion_Sleepy      = 12,
    EyeEmotion_Suspicious  = 13,
    EyeEmotion_Squint      = 14,
    EyeEmotion_Furious     = 15,
    EyeEmotion_Scared      = 16,
    EyeEmotion_Awe         = 17,
    EyeEmotion_Count       = 18
};

/* ---- Preset eye shapes for each emotion ---- */
static const EyeConfig EyePreset_Normal = {
    0,0, 40,40, 0,0, 8,8, 0,0,0,0
};
static const EyeConfig EyePreset_Happy = {
    0,0, 10,40, 0,0, 10,0, 0,0,0,0
};
static const EyeConfig EyePreset_Glee = {
    0,0, 8,40, 0,0, 8,0, 0,5,0,0
};
static const EyeConfig EyePreset_Sad = {
    0,0, 15,40, -0.5f,0, 1,10, 0,0,0,0
};
static const EyeConfig EyePreset_Worried = {
    0,0, 25,40, -0.1f,0, 6,10, 0,0,0,0
};
static const EyeConfig EyePreset_Worried_Alt = {
    0,0, 35,40, -0.2f,0, 6,10, 0,0,0,0
};
static const EyeConfig EyePreset_Focused = {
    0,0, 14,40, 0.2f,0, 3,1, 0,0,0,0
};
static const EyeConfig EyePreset_Annoyed = {
    0,0, 12,40, 0,0, 0,10, 0,0,0,0
};
static const EyeConfig EyePreset_Annoyed_Alt = {
    0,0, 5,40, 0,0, 0,4, 0,0,0,0
};
static const EyeConfig EyePreset_Surprised = {
    -2,0, 45,45, 0,0, 16,16, 0,0,0,0
};
static const EyeConfig EyePreset_Skeptic = {
    0,0, 40,40, 0,0, 10,10, 0,0,0,0
};
static const EyeConfig EyePreset_Skeptic_Alt = {
    0,-6, 26,40, 0.3f,0, 1,10, 0,0,0,0
};
static const EyeConfig EyePreset_Frustrated = {
    3,-5, 12,40, 0,0, 0,10, 0,0,0,0
};
static const EyeConfig EyePreset_Unimpressed = {
    3,0, 12,40, 0,0, 1,10, 0,0,0,0
};
static const EyeConfig EyePreset_Unimpressed_Alt = {
    3,-3, 22,40, 0,0, 1,16, 0,0,0,0
};
static const EyeConfig EyePreset_Sleepy = {
    0,-2, 14,40, -0.5f, -0.5f, 3,3, 0,0,0,0
};
static const EyeConfig EyePreset_Sleepy_Alt = {
    0,-2, 8,40, -0.5f, -0.5f, 3,3, 0,0,0,0
};
static const EyeConfig EyePreset_Suspicious = {
    0,0, 22,40, 0,0, 8,3, 0,0,0,0
};
static const EyeConfig EyePreset_Suspicious_Alt = {
    0,-3, 16,40, 0.2f,0, 6,3, 0,0,0,0
};
static const EyeConfig EyePreset_Squint = {
    -10,-3, 35,35, 0,0, 8,8, 0,0,0,0
};
static const EyeConfig EyePreset_Squint_Alt = {
    5,0, 20,20, 0,0, 5,5, 0,0,0,0
};
static const EyeConfig EyePreset_Angry = {
    -3,0, 20,40, 0.3f,0, 2,12, 0,0,0,0
};
static const EyeConfig EyePreset_Furious = {
    -2,0, 30,40, 0.4f,0, 2,8, 0,0,0,0
};
static const EyeConfig EyePreset_Scared = {
    -3,0, 40,40, -0.1f,0, 12,8, 0,0,0,0
};
static const EyeConfig EyePreset_Awe = {
    2,0, 35,45, -0.1f, 0.1f, 12,12, 0,0,0,0
};

#ifdef __cplusplus
extern "C" {
#endif

void EyeExpression_Init();
void EyeExpression_Update();
void EyeExpression_SetEmotion(EyeEmotion emotion);
void EyeExpression_Blink();
void EyeExpression_LookAt(float x, float y);
void EyeExpression_SetRandomBehavior(bool enable);
void EyeExpression_SetRandomBlink(bool enable);
void EyeExpression_SetRandomLook(bool enable);
EyeEmotion EyeExpression_GetCurrentEmotion();

#ifdef __cplusplus
}
#endif

