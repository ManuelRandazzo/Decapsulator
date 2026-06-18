/**
 * This file is part of {{ Decapsulator }}.
 *
 * Developed for the ITS Meccatronico Montebelluna-Castelfranco Veneto 2024-2026 Gruppo 1.
 * This product includes software developed by the Gruppo 1 Project.
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is NOT free software: you can NOT redistribute it and/or modify it.
 *
 * The Documentation is made following the Doxygen standard documentation style
 * 
 * @author Alessio Pisan
 */

#include "HMI.hpp"
#include "HMI_UI_EEZ/ui.h"
#include "HMI_UI_EEZ/vars.h"
#include "SavingFilesSD.hpp"


#ifndef LV_COLOR_DEPTH
#error "LV_COLOR_DEPTH is not defined!"
#endif

#if LV_COLOR_DEPTH != 16
#error "This driver only supports LV_COLOR_DEPTH 16 (RGB565)"
#endif

static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;

enum BoardConstants { LVGL_BUFFER_RATIO = 10 };
enum { SCREENBUFFER_SIZE_PIXELS = screenWidth * screenHeight / BoardConstants::LVGL_BUFFER_RATIO };

static uint16_t buf[SCREENBUFFER_SIZE_PIXELS] __attribute__((aligned(4)));

/* -------------------------------------------------------------------------- */
/*  LVGL logging (ultra vital for mental health)                              */
/* -------------------------------------------------------------------------- */

#if LV_USE_LOG != 0
    void my_print(const char *buf) {
        Serial.printf(buf);
        Serial.flush();
}
#endif

/* -------------------------------------------------------------------------- */
/*  Display flush callback                                                    */
/* -------------------------------------------------------------------------- */

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *pixelmap) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)pixelmap, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/* -------------------------------------------------------------------------- */
/*  Touchpad callback                                                         */
/* -------------------------------------------------------------------------- */

void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data) {
    uint16_t touchX = 0, touchY = 0;

    if(!tft.getTouch(&touchX, &touchY, 5))
        data->state = LV_INDEV_STATE_REL;
    else
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

/* -------------------------------------------------------------------------- */
/*  LVGL tick callback                                                        */
/* -------------------------------------------------------------------------- */

static uint32_t my_tick_get_cb(void) { return millis(); }


void prgHMITask(void* pvParameters)
{
    lv_init();
    
    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(my_tick_get_cb);
    
    static lv_display_t *disp;
    disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, my_disp_flush);

    static lv_indev_t *indev;
    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);
    

    uint16_t calib_data[5] = { 365, 3394, 265, 3073, 5 };
    uint16_t old_data[5] = { 0, 0, 0, 0, 0 };
    bool useDefaulCalibDatas = false;
    /// Ottiene i dati di calibrazione dalla SD
    for(uint8_t i = 0; i < 5; i++)
    {
        old_data[i] = SD_Card.getValueByKey<uint16_t>("/touch_calibration.txt", "data[" + String(i) + "]");
        if(old_data[i] == 0)
        {
            /// Usa i dati default se quelli in SD non sono corretti
            useDefaulCalibDatas = true;
            break;
        }
    }

    tft.setTouch(useDefaulCalibDatas ? calib_data : old_data);

    /// Init EEZ-Studio UI
    ui_init();
    
    while(1)
    {
        /// Update EEZ-Studio UI
        ui_tick(); 

        /// Reference https://lvgl.io/docs/open/integration/overview Sezione "Sleep Management"
        uint32_t time_until_next = lv_timer_handler();
        /// Se non c'è nulla da fare ora, controllerà ancora fra un po'.
        if(time_until_next == LV_NO_TIMER_READY)
            time_until_next = LV_DEF_REFR_PERIOD; //16 ms (60Hz) in lv_conf.h
        
        /// Sempre consigliato dalla reference sopracitata
        vTaskDelay(pdMS_TO_TICKS(time_until_next)); //Sleep the thread
    }
}