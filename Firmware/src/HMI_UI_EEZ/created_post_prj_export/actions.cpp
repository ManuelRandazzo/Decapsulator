#include "HMI_UI_EEZ/actions.h"
#include "HMI_UI_EEZ/vars.h"
#include "decapsulator_io.hpp"
#include "HMI_UI_EEZ/screens.h"

void action_calibrazione_touch(lv_event_t *e)
{
    /// Comunica alla UI che faccia la calibrazione del touch
    set_var_calibrazione_touch_finita(false);

    /// Background color
    constexpr uint32_t BG_COLOR = TFT_BLACK;
    /// Foreground color
    constexpr uint32_t FG_COLOR = TFT_RED;

    uint16_t calib_datas[5] = { 0, 0, 0, 0, 0 };
    
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);

    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    /// Crea la label
    lv_obj_t * label = lv_label_create(lv_screen_active());

    /// Imposta il colore del testo a bianco
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN); 
    
    /// Centra la label
    lv_obj_center(label);

    /// Setta il font
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);

    /// Imposta il testo
    lv_label_set_text(label, "                  Calibrazione del display.\
                            \n                  Si consiglia di utilizzare un pennino.\
                            \n                  Toccare gli angoli indicati dalla freccia\
                            \n                  nella sequenza richiesta.\
                            \n                  Prendersi tutto il tempo necessario");
    
    /// Refresh forzato
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(lv_obj_get_display(lv_screen_active())); 
    
    tft.calibrateTouch(calib_datas, FG_COLOR, BG_COLOR, 40);
    tft.setTouch(calib_datas);

    /// Scrive i dati di calibrazione nella SD
    for(uint8_t i=0; i < 5; i++)
        SD_Card.setValueByKey("/touch_calibration.txt", "data[" + String(i) + "]", calib_datas[i]);

    /// Comunica alla UI che la calibrazione del touch è terminata
    set_var_calibrazione_touch_finita(true);

    /// Elimina la label
    lv_obj_delete(label);
    
    /// Refresh forzato
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(lv_obj_get_display(lv_screen_active())); 
};

void action_logger_page_init(lv_event_t * e){
};

void action_logger_page_deinit(lv_event_t * e){
};

void action_jogger_page_init(lv_event_t * e){
};

void action_jogger_page_deinit(lv_event_t * e){
};

void action_conf_wi_fi_page_init(lv_event_t * e){
};

void action_conf_wi_fi_page_deinit(lv_event_t * e){
};