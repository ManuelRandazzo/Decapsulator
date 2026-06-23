#ifndef EEZ_LVGL_UI_GUI_H
#define EEZ_LVGL_UI_GUI_H

#include <lvgl.h>

#include "eez-flow.h"

#ifdef __cplusplus
extern "C" {
#endif

<<<<<<< HEAD
extern const uint8_t assets[31596];
=======
extern const uint8_t assets[31260];
>>>>>>> 7dc3a87 (add(touch calibration text string) : aggiunta la stringa delle istruzioni per la calibrazione del touch.)

void ui_init();
void ui_tick();

#ifdef __cplusplus
}
#endif

#endif // EEZ_LVGL_UI_GUI_H