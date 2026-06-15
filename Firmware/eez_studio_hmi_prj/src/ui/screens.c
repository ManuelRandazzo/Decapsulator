#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

static const char *screen_names[] = { "Schermata Avvio", "Schermata principale", "Schermata Password", "Schermata Diagnostica", "Schermata Jogger", "Schermata Easter Egg", "Schermata Logger", "Scheramta Config WiFi", "Schermata Calibrazione" };
static const char *object_names[] = { "schermata_avvio", "schermata_principale", "schermata_password", "schermata_diagnostica", "schermata_jogger", "schermata_easter_egg", "schermata_logger", "scheramta_config_wi_fi", "schermata_calibrazione", "stato_avvio", "logo", "conteggio_capsule", "capsule_sessione", "vis_cont_caps_ses", "cont_caps_ses", "cont_caps_tot", "vis_cont_caps_tot", "capsule_totali", "impostazioni", "puls_start_stop", "start_stop", "finestra_errore", "puls_ok", "ok", "titolo_errore", "obj0", "tastiera_pssw", "password", "annulla_pssw", "annulla", "messaggio_pssw", "annula_diagnostica", "annulla_diag", "logger", "logger_titolo", "contrmot", "contrmot_titolo", "blocco_jogger", "obj1", "obj2", "jog_motori", "annulla_jogger", "annulla_3", "controllo_m_otori", "contrl_mot_ralla", "contr_ralla", "puls_en_dis_ralla", "en_dis_ralla", "puls_jog_av_ralla", "av_ralla", "puls_jog_ind_ralla", "ind_ralla", "stato_movimento_ralla", "led_stato_movimento_ralla", "titolo_stato_ralla", "contrl_mot_pun", "contr_punz", "puls_en_dis_punz", "en_dis_punz", "puls_jog_av_punz", "av_punz", "puls_jog_ind_punz", "ind_punz", "stato_movimento_punz", "led_stato_movimento_punz", "titolo_stato_punz", "calibrazione", "sensore_di_calibrazione", "titolo_sensore_di_calibrazione", "stato_sensore_di_calibrazione", "gradi_per_click_ralla", "finecorsa_max", "titolo_finecorsa_max", "stato_finecorsa_max", "finecorsa_min", "titolo_finecorsa_min", "stato_finecorsa_min", "homing", "tit_homing", "gradi_per_click_punz", "speed_motore_punz", "speed_motore_ralla", "inserimento_valori", "annulla_easter_egg", "annulla_diag_2", "annulla_pssw_2", "annulla_diag_1", "obj3", "obj4", "annulla_config_wifi", "annulla_config_wifi2", "password_rete_wifi", "nome_rete_wifi", "config_wifi", "tastiera_config_wi_fi", "messaggio_config_wi_fi" };

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

static void event_handler_cb_schermata_avvio_schermata_avvio(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 1, 0, e);
    }
}

static void event_handler_cb_schermata_principale_impostazioni(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 9, 0, e);
    }
}

static void event_handler_cb_schermata_principale_puls_start_stop(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 10, 0, e);
    }
}

static void event_handler_cb_schermata_principale_puls_ok(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 13, 0, e);
    }
}

static void event_handler_cb_schermata_principale_obj0(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 16, 0, e);
    }
}

static void event_handler_cb_schermata_password_tastiera_pssw(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_READY) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 0, 0, e);
    }
}

static void event_handler_cb_schermata_password_password(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 2, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

static void event_handler_cb_schermata_password_annulla_pssw(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 3, 0, e);
    }
}

static void event_handler_cb_schermata_password_messaggio_pssw(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 5, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

static void event_handler_cb_schermata_diagnostica_annula_diagnostica(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 0, 0, e);
    }
}

static void event_handler_cb_schermata_diagnostica_logger(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 3, 0, e);
    }
}

static void event_handler_cb_schermata_diagnostica_contrmot(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 5, 0, e);
    }
}

static void event_handler_cb_schermata_diagnostica_obj1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 9, 0, e);
    }
}

static void event_handler_cb_schermata_diagnostica_obj2(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 11, 0, e);
    }
}

static void event_handler_cb_schermata_jogger_annulla_jogger(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 2, 0, e);
    }
}

static void event_handler_cb_schermata_jogger_puls_en_dis_ralla(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 7, 0, e);
    }
}

static void event_handler_cb_schermata_jogger_puls_jog_av_ralla(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 9, 0, e);
    }
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 9, 1, e);
    }
}

static void event_handler_cb_schermata_jogger_puls_jog_ind_ralla(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 11, 0, e);
    }
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 11, 1, e);
    }
}

static void event_handler_cb_schermata_jogger_puls_en_dis_punz(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 18, 0, e);
    }
}

static void event_handler_cb_schermata_jogger_puls_jog_av_punz(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 20, 0, e);
    }
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 20, 1, e);
    }
}

static void event_handler_cb_schermata_jogger_puls_jog_ind_punz(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 22, 0, e);
    }
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 22, 1, e);
    }
}

static void event_handler_cb_schermata_jogger_stato_sensore_di_calibrazione(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 30, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

static void event_handler_cb_schermata_jogger_gradi_per_click_ralla(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 33, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 33, 0, e);
    }
}

static void event_handler_cb_schermata_jogger_stato_finecorsa_max(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 36, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

static void event_handler_cb_schermata_jogger_stato_finecorsa_min(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 39, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

static void event_handler_cb_schermata_jogger_homing(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 40, 0, e);
    }
}

static void event_handler_cb_schermata_jogger_gradi_per_click_punz(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 44, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 44, 0, e);
    }
}

static void event_handler_cb_schermata_jogger_speed_motore_punz(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 47, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 47, 0, e);
    }
}

static void event_handler_cb_schermata_jogger_speed_motore_ralla(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 50, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 50, 0, e);
    }
}

static void event_handler_cb_schermata_jogger_inserimento_valori(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_READY) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 51, 0, e);
    }
}

static void event_handler_cb_schermata_easter_egg_annulla_easter_egg(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 0, 0, e);
    }
}

static void event_handler_cb_schermata_logger_annulla_pssw_2(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 0, 0, e);
    }
}

static void event_handler_cb_schermata_logger_obj3(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 3, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

static void event_handler_cb_schermata_logger_obj4(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 4, 0, e);
    }
}

static void event_handler_cb_scheramta_config_wi_fi_annulla_config_wifi(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 0, 0, e);
    }
}

static void event_handler_cb_scheramta_config_wi_fi_password_rete_wifi(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 3, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 3, 0, e);
    }
}

static void event_handler_cb_scheramta_config_wi_fi_nome_rete_wifi(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 4, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 4, 0, e);
    }
}

static void event_handler_cb_scheramta_config_wi_fi_messaggio_config_wi_fi(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 7, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

//
// Screens
//

void create_screen_schermata_avvio() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.schermata_avvio = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_add_event_cb(obj, event_handler_cb_schermata_avvio_schermata_avvio, LV_EVENT_ALL, flowState);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_image_src(obj, &img_logo_scuola, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xa70000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_spinner_create(parent_obj);
            lv_obj_set_pos(obj, 158, 272);
            lv_obj_set_size(obj, 43, 48);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 63, 17);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_titoli_schermata_iniziale(obj);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "GRUPPO MB N.1 \"DECAPSULATOR\"");
        }
        {
            // Stato Avvio
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.stato_avvio = obj;
            lv_obj_set_pos(obj, 215, 288);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_titoli_schermata_iniziale(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
    }
    
    tick_screen_schermata_avvio();
}

void tick_screen_schermata_avvio() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 3, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.stato_avvio);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.stato_avvio;
            lv_label_set_text(objects.stato_avvio, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_schermata_principale() {
    void *flowState = getFlowState(0, 1);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.schermata_principale = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_image_src(obj, &img_sfondo, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // LOGO
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.logo = obj;
            lv_obj_set_pos(obj, 145, 44);
            lv_obj_set_size(obj, 194, 165);
            lv_image_set_src(obj, &img_logo_gruppo);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_bg_image_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // CONTEGGIO CAPSULE
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.conteggio_capsule = obj;
            lv_obj_set_pos(obj, 343, 103);
            lv_obj_set_size(obj, 120, 203);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // capsule sessione
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.capsule_sessione = obj;
                    lv_obj_set_pos(obj, 1, 136);
                    lv_obj_set_size(obj, 82, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "capsule sessione");
                }
                {
                    // Vis Cont Caps Ses
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.vis_cont_caps_ses = obj;
                    lv_obj_set_pos(obj, -8, 91);
                    lv_obj_set_size(obj, 100, 45);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // Cont Caps Ses
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.cont_caps_ses = obj;
                            lv_obj_set_pos(obj, 0, 1);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "");
                        }
                    }
                }
                {
                    // Cont Caps Tot
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.cont_caps_tot = obj;
                    lv_obj_set_pos(obj, -8, -8);
                    lv_obj_set_size(obj, 100, 45);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // Vis Cont Caps Tot
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.vis_cont_caps_tot = obj;
                            lv_obj_set_pos(obj, 0, 1);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "");
                        }
                    }
                }
                {
                    // capsule totali
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.capsule_totali = obj;
                    lv_obj_set_pos(obj, 2, 37);
                    lv_obj_set_size(obj, 82, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "capsule totali");
                }
            }
        }
        {
            // Impostazioni
            lv_obj_t *obj = lv_imagebutton_create(parent_obj);
            objects.impostazioni = obj;
            lv_obj_set_pos(obj, 38, 164);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, 80);
            lv_imagebutton_set_src(obj, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &img_logo_impostazioni, NULL);
            lv_imagebutton_set_src(obj, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &img_logo_impostazioni, NULL);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_principale_impostazioni, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xb7b7b7), LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xb7b7b7), LV_PART_MAIN | LV_STATE_PRESSED);
        }
        {
            // PULS START-STOP
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.puls_start_stop = obj;
            lv_obj_set_pos(obj, 192, 235);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_principale_puls_start_stop, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_puls_rosso_verde(obj);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // START-STOP
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.start_stop = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
        {
            // Finestra errore
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.finestra_errore = obj;
            lv_obj_set_pos(obj, 92, 28);
            lv_obj_set_size(obj, 300, 200);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_opa(obj, 200, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Puls OK
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.puls_ok = obj;
                    lv_obj_set_pos(obj, 78, 108);
                    lv_obj_set_size(obj, 108, 50);
                    lv_obj_add_event_cb(obj, event_handler_cb_schermata_principale_puls_ok, LV_EVENT_ALL, flowState);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // OK
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.ok = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "OK");
                        }
                    }
                }
                {
                    // Titolo errore
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.titolo_errore = obj;
                    lv_obj_set_pos(obj, -18, -18);
                    lv_obj_set_size(obj, 300, 116);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 61, 50);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_principale_obj0, LV_EVENT_ALL, flowState);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Button");
                }
            }
        }
    }
    
    tick_screen_schermata_principale();
}

void tick_screen_schermata_principale() {
    void *flowState = getFlowState(0, 1);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 5, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.cont_caps_ses);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.cont_caps_ses;
            lv_label_set_text(objects.cont_caps_ses, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 7, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.vis_cont_caps_tot);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.vis_cont_caps_tot;
            lv_label_set_text(objects.vis_cont_caps_tot, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 11, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.start_stop);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.start_stop;
            lv_label_set_text(objects.start_stop, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 15, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.titolo_errore);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.titolo_errore;
            lv_label_set_text(objects.titolo_errore, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        float timeline_position = getTimelinePosition(flowState);
        
        static struct {
            float last_timeline_position;
        } anim_state = { -1 };
        
        if (anim_state.last_timeline_position == -1) {
            anim_state.last_timeline_position = 0;
        }
        
        if (timeline_position != anim_state.last_timeline_position) {
            anim_state.last_timeline_position = timeline_position;
            
            {
                lv_obj_t *obj = objects.start_stop;

while (1) {
                    // keyframe #1
                    if (timeline_position < 0) {
                        break;
                    }
                    
                    break;
                }
                
                lv_style_value_t value;
            }
        }
    }
}

void create_screen_schermata_password() {
    void *flowState = getFlowState(0, 2);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.schermata_password = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_image_src(obj, &img_sfondo, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // TASTIERA PSSW
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.tastiera_pssw = obj;
            lv_obj_set_pos(obj, 8, 85);
            lv_obj_set_size(obj, 465, 226);
            lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_NUMBER);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_password_tastiera_pssw, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Password
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.password = obj;
            lv_obj_set_pos(obj, 8, 24);
            lv_obj_set_size(obj, 300, 40);
            lv_textarea_set_accepted_chars(obj, "0123456789");
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_placeholder_text(obj, "Inserire PASSWORD");
            lv_textarea_set_one_line(obj, true);
            lv_textarea_set_password_mode(obj, true);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_password_password, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ANNULLA PSSW
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.annulla_pssw = obj;
            lv_obj_set_pos(obj, 351, 24);
            lv_obj_set_size(obj, 114, 45);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_password_annulla_pssw, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_pulsante_rosso(obj);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Annulla
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.annulla = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ANNULLA");
                }
            }
        }
        {
            // Messaggio PSSW
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.messaggio_pssw = obj;
            lv_obj_set_pos(obj, 65, 85);
            lv_obj_set_size(obj, 350, 145);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_one_line(obj, false);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_password_messaggio_pssw, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    lv_keyboard_set_textarea(objects.tastiera_pssw, objects.password);
    
    tick_screen_schermata_password();
}

void tick_screen_schermata_password() {
    void *flowState = getFlowState(0, 2);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 2, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.password);
        uint32_t max_length = lv_textarea_get_max_length(objects.password);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.password;
            lv_textarea_set_text(objects.password, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 5, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.messaggio_pssw);
        uint32_t max_length = lv_textarea_get_max_length(objects.messaggio_pssw);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.messaggio_pssw;
            lv_textarea_set_text(objects.messaggio_pssw, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_schermata_diagnostica() {
    void *flowState = getFlowState(0, 3);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.schermata_diagnostica = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_image_src(obj, &img_sfondo, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ANNULA DIAGNOSTICA
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.annula_diagnostica = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 114, 45);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_diagnostica_annula_diagnostica, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_pulsante_rosso(obj);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Annulla_Diag
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.annulla_diag = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ANNULLA");
                }
            }
        }
        {
            // Logger
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.logger = obj;
            lv_obj_set_pos(obj, 20, 54);
            lv_obj_set_size(obj, 200, 100);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_diagnostica_logger, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Logger_titolo
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.logger_titolo = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Logger");
                }
            }
        }
        {
            // contrmot
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.contrmot = obj;
            lv_obj_set_pos(obj, 260, 55);
            lv_obj_set_size(obj, 200, 100);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_diagnostica_contrmot, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // contrmot_titolo
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.contrmot_titolo = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Jogger");
                }
            }
        }
        {
            // Blocco jogger
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.blocco_jogger = obj;
            lv_obj_set_pos(obj, 260, 69);
            lv_obj_set_size(obj, 200, 61);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_opa(obj, 155, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 2, -3);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text_static(obj, "Impossibile accedere\nmacchina in funzione");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 20, 194);
            lv_obj_set_size(obj, 200, 100);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_diagnostica_obj1, LV_EVENT_ALL, flowState);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Calibrazione\nTouch");
                }
            }
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 260, 194);
            lv_obj_set_size(obj, 200, 100);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_diagnostica_obj2, LV_EVENT_ALL, flowState);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Impostazioni\nWiFi");
                }
            }
        }
    }
    
    tick_screen_schermata_diagnostica();
}

void tick_screen_schermata_diagnostica() {
    void *flowState = getFlowState(0, 3);
    (void)flowState;
}

void create_screen_schermata_jogger() {
    void *flowState = getFlowState(0, 4);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.schermata_jogger = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_image_src(obj, &img_sfondo, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Jog Motori
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.jog_motori = obj;
            lv_obj_set_pos(obj, 131, 11);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Jog Motori");
        }
        {
            // ANNULLA JOGGER
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.annulla_jogger = obj;
            lv_obj_set_pos(obj, 5, 5);
            lv_obj_set_size(obj, 114, 45);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_annulla_jogger, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_pulsante_rosso(obj);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Annulla_3
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.annulla_3 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ANNULLA");
                }
            }
        }
        {
            // Controllo M;otori
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.controllo_m_otori = obj;
            lv_obj_set_pos(obj, 5, 60);
            lv_obj_set_size(obj, 285, 250);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_bg_opa(obj, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Contrl Mot Ralla
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.contrl_mot_ralla = obj;
                    lv_obj_set_pos(obj, -18, -18);
                    lv_obj_set_size(obj, 285, 125);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // Contr Ralla
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.contr_ralla = obj;
                            lv_obj_set_pos(obj, 29, 10);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Controllo Motore Ralla");
                        }
                        {
                            // Puls EN-DIS Ralla
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.puls_en_dis_ralla = obj;
                            lv_obj_set_pos(obj, 93, 38);
                            lv_obj_set_size(obj, 100, 50);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_puls_en_dis_ralla, LV_EVENT_ALL, flowState);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SNAPPABLE);
                            add_style_puls_rosso_verde(obj);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // EN-DIS Ralla
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.en_dis_ralla = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SNAPPABLE);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "");
                                }
                            }
                        }
                        {
                            // Puls Jog Av Ralla
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.puls_jog_av_ralla = obj;
                            lv_obj_set_pos(obj, 205, 38);
                            lv_obj_set_size(obj, 70, 50);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_puls_jog_av_ralla, LV_EVENT_ALL, flowState);
                            lv_obj_add_state(obj, LV_STATE_DISABLED);
                            add_style_puls_jog(obj);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // Av Ralla
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.av_ralla = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text_static(obj, "Jog+");
                                }
                            }
                        }
                        {
                            // Puls Jog Ind Ralla
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.puls_jog_ind_ralla = obj;
                            lv_obj_set_pos(obj, 10, 38);
                            lv_obj_set_size(obj, 70, 50);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_puls_jog_ind_ralla, LV_EVENT_ALL, flowState);
                            lv_obj_add_state(obj, LV_STATE_DISABLED);
                            add_style_puls_jog(obj);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // Ind Ralla
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.ind_ralla = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text_static(obj, "Jog-");
                                }
                            }
                        }
                        {
                            // Stato Movimento Ralla
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.stato_movimento_ralla = obj;
                            lv_obj_set_pos(obj, 73, 95);
                            lv_obj_set_size(obj, 140, 30);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // Led Stato Movimento Ralla
                                    lv_obj_t *obj = lv_led_create(parent_obj);
                                    objects.led_stato_movimento_ralla = obj;
                                    lv_obj_set_pos(obj, 21, 5);
                                    lv_obj_set_size(obj, 18, 18);
                                    lv_led_set_brightness(obj, 255);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
                                }
                                {
                                    // Titolo Stato Ralla
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.titolo_stato_ralla = obj;
                                    lv_obj_set_pos(obj, 53, 5);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text_static(obj, "Moving");
                                }
                            }
                        }
                    }
                }
                {
                    // Contrl Mot Pun
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.contrl_mot_pun = obj;
                    lv_obj_set_pos(obj, -18, 107);
                    lv_obj_set_size(obj, 285, 125);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // Contr Punz
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.contr_punz = obj;
                            lv_obj_set_pos(obj, 29, 10);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Controllo Motore Punz");
                        }
                        {
                            // Puls EN-DIS Punz
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.puls_en_dis_punz = obj;
                            lv_obj_set_pos(obj, 93, 38);
                            lv_obj_set_size(obj, 100, 50);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_puls_en_dis_punz, LV_EVENT_ALL, flowState);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SNAPPABLE);
                            add_style_puls_rosso_verde(obj);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // EN-DIS Punz
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.en_dis_punz = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SNAPPABLE);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text(obj, "");
                                }
                            }
                        }
                        {
                            // Puls Jog Av Punz
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.puls_jog_av_punz = obj;
                            lv_obj_set_pos(obj, 205, 38);
                            lv_obj_set_size(obj, 70, 50);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_puls_jog_av_punz, LV_EVENT_ALL, flowState);
                            lv_obj_add_state(obj, LV_STATE_DISABLED);
                            add_style_puls_jog(obj);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // Av Punz
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.av_punz = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text_static(obj, "Jog+");
                                }
                            }
                        }
                        {
                            // Puls Jog Ind Punz
                            lv_obj_t *obj = lv_button_create(parent_obj);
                            objects.puls_jog_ind_punz = obj;
                            lv_obj_set_pos(obj, 10, 38);
                            lv_obj_set_size(obj, 70, 50);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_puls_jog_ind_punz, LV_EVENT_ALL, flowState);
                            lv_obj_add_state(obj, LV_STATE_DISABLED);
                            add_style_puls_jog(obj);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // Ind Punz
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.ind_punz = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text_static(obj, "Jog-");
                                }
                            }
                        }
                        {
                            // Stato Movimento Punz
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.stato_movimento_punz = obj;
                            lv_obj_set_pos(obj, 73, 95);
                            lv_obj_set_size(obj, 140, 30);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // Led Stato Movimento Punz
                                    lv_obj_t *obj = lv_led_create(parent_obj);
                                    objects.led_stato_movimento_punz = obj;
                                    lv_obj_set_pos(obj, 21, 5);
                                    lv_obj_set_size(obj, 18, 18);
                                    lv_led_set_brightness(obj, 255);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
                                }
                                {
                                    // Titolo Stato Punz
                                    lv_obj_t *obj = lv_label_create(parent_obj);
                                    objects.titolo_stato_punz = obj;
                                    lv_obj_set_pos(obj, 53, 5);
                                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                                    lv_label_set_text_static(obj, "Moving");
                                }
                            }
                        }
                    }
                }
            }
        }
        {
            // Calibrazione
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.calibrazione = obj;
            lv_obj_set_pos(obj, 293, 11);
            lv_obj_set_size(obj, 183, 299);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Sensore di Calibrazione
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.sensore_di_calibrazione = obj;
                    lv_obj_set_pos(obj, 30, -16);
                    lv_obj_set_size(obj, 88, 62);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // Titolo Sensore di Calibrazione
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.titolo_sensore_di_calibrazione = obj;
                            lv_obj_set_pos(obj, 0, -3);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Sensore \ndi Calibrazione");
                        }
                        {
                            // Stato Sensore di Calibrazione
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            objects.stato_sensore_di_calibrazione = obj;
                            lv_obj_set_pos(obj, 4, 27);
                            lv_obj_set_size(obj, 80, 35);
                            lv_textarea_set_max_length(obj, 128);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_stato_sensore_di_calibrazione, LV_EVENT_ALL, flowState);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SNAPPABLE);
                            add_style_stato_sensori(obj);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, -14, 46);
                    lv_obj_set_size(obj, 88, 62);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 5);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Gradi per click");
                        }
                        {
                            // Gradi_per_click_ralla
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            objects.gradi_per_click_ralla = obj;
                            lv_obj_set_pos(obj, 2, 20);
                            lv_obj_set_size(obj, 81, 39);
                            lv_textarea_set_max_length(obj, 10);
                            lv_textarea_set_placeholder_text(obj, "0.00");
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_gradi_per_click_ralla, LV_EVENT_ALL, flowState);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    // Finecorsa Max
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.finecorsa_max = obj;
                    lv_obj_set_pos(obj, -14, 160);
                    lv_obj_set_size(obj, 88, 53);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // titolo Finecorsa Max
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.titolo_finecorsa_max = obj;
                            lv_obj_set_pos(obj, 1, -2);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Finecorsa Max");
                        }
                        {
                            // Stato Finecorsa Max
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            objects.stato_finecorsa_max = obj;
                            lv_obj_set_pos(obj, 4, 13);
                            lv_obj_set_size(obj, 80, 31);
                            lv_textarea_set_max_length(obj, 128);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_stato_finecorsa_max, LV_EVENT_ALL, flowState);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SNAPPABLE);
                            add_style_stato_sensori(obj);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    // Finecorsa Min
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.finecorsa_min = obj;
                    lv_obj_set_pos(obj, 74, 160);
                    lv_obj_set_size(obj, 88, 53);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // Titolo Finecorsa min
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.titolo_finecorsa_min = obj;
                            lv_obj_set_pos(obj, 2, -2);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Finecorsa Min");
                        }
                        {
                            // Stato_Finecorsa_Min
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            objects.stato_finecorsa_min = obj;
                            lv_obj_set_pos(obj, 4, 13);
                            lv_obj_set_size(obj, 80, 31);
                            lv_textarea_set_max_length(obj, 128);
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_stato_finecorsa_min, LV_EVENT_ALL, flowState);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SNAPPABLE);
                            add_style_stato_sensori(obj);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    // Homing
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.homing = obj;
                    lv_obj_set_pos(obj, 20, 108);
                    lv_obj_set_size(obj, 108, 48);
                    lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_homing, LV_EVENT_ALL, flowState);
                    add_style_giallo(obj);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // Tit Homing
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.tit_homing = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Homing");
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, -14, 213);
                    lv_obj_set_size(obj, 88, 64);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 2);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Gradi per click");
                        }
                        {
                            // Gradi_per_click_punz
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            objects.gradi_per_click_punz = obj;
                            lv_obj_set_pos(obj, 4, 19);
                            lv_obj_set_size(obj, 80, 39);
                            lv_textarea_set_max_length(obj, 10);
                            lv_textarea_set_placeholder_text(obj, "0.00");
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_gradi_per_click_punz, LV_EVENT_ALL, flowState);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 74, 213);
                    lv_obj_set_size(obj, 88, 64);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 2);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Speed [gradi/s]");
                        }
                        {
                            // Speed_Motore_Punz
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            objects.speed_motore_punz = obj;
                            lv_obj_set_pos(obj, 4, 19);
                            lv_obj_set_size(obj, 80, 39);
                            lv_textarea_set_max_length(obj, 10);
                            lv_textarea_set_placeholder_text(obj, "0.00");
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_speed_motore_punz, LV_EVENT_ALL, flowState);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 74, 46);
                    lv_obj_set_size(obj, 89, 62);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 5);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "Speed [gradi/s]");
                        }
                        {
                            // Speed_Motore_Ralla
                            lv_obj_t *obj = lv_textarea_create(parent_obj);
                            objects.speed_motore_ralla = obj;
                            lv_obj_set_pos(obj, 4, 20);
                            lv_obj_set_size(obj, 80, 39);
                            lv_textarea_set_max_length(obj, 10);
                            lv_textarea_set_placeholder_text(obj, "0.00");
                            lv_textarea_set_one_line(obj, true);
                            lv_textarea_set_password_mode(obj, false);
                            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_speed_motore_ralla, LV_EVENT_ALL, flowState);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        }
                    }
                }
            }
        }
        {
            // Inserimento Valori
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.inserimento_valori = obj;
            lv_obj_set_pos(obj, 5, 60);
            lv_obj_set_size(obj, 285, 250);
            lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_NUMBER);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_jogger_inserimento_valori, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_state(obj, LV_STATE_DISABLED);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_schermata_jogger();
}

void tick_screen_schermata_jogger() {
    void *flowState = getFlowState(0, 4);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 8, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.en_dis_ralla);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.en_dis_ralla;
            lv_label_set_text(objects.en_dis_ralla, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        uint32_t new_val = evalUnsignedIntegerProperty(flowState, 14, 3, "Failed to evaluate Color in Led widget");
        new_val = lv_color_to_u32(lv_color_hex(new_val));
        uint32_t cur_val = lv_color_to_u32(((lv_led_t *)objects.led_stato_movimento_ralla)->color);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.led_stato_movimento_ralla;
            lv_led_set_color(objects.led_stato_movimento_ralla, lv_color_hex(new_val));
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 19, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.en_dis_punz);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.en_dis_punz;
            lv_label_set_text(objects.en_dis_punz, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        uint32_t new_val = evalUnsignedIntegerProperty(flowState, 25, 3, "Failed to evaluate Color in Led widget");
        new_val = lv_color_to_u32(lv_color_hex(new_val));
        uint32_t cur_val = lv_color_to_u32(((lv_led_t *)objects.led_stato_movimento_punz)->color);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.led_stato_movimento_punz;
            lv_led_set_color(objects.led_stato_movimento_punz, lv_color_hex(new_val));
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 30, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.stato_sensore_di_calibrazione);
        uint32_t max_length = lv_textarea_get_max_length(objects.stato_sensore_di_calibrazione);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.stato_sensore_di_calibrazione;
            lv_textarea_set_text(objects.stato_sensore_di_calibrazione, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 33, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.gradi_per_click_ralla);
        uint32_t max_length = lv_textarea_get_max_length(objects.gradi_per_click_ralla);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.gradi_per_click_ralla;
            lv_textarea_set_text(objects.gradi_per_click_ralla, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 36, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.stato_finecorsa_max);
        uint32_t max_length = lv_textarea_get_max_length(objects.stato_finecorsa_max);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.stato_finecorsa_max;
            lv_textarea_set_text(objects.stato_finecorsa_max, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 39, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.stato_finecorsa_min);
        uint32_t max_length = lv_textarea_get_max_length(objects.stato_finecorsa_min);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.stato_finecorsa_min;
            lv_textarea_set_text(objects.stato_finecorsa_min, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 44, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.gradi_per_click_punz);
        uint32_t max_length = lv_textarea_get_max_length(objects.gradi_per_click_punz);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.gradi_per_click_punz;
            lv_textarea_set_text(objects.gradi_per_click_punz, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 47, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.speed_motore_punz);
        uint32_t max_length = lv_textarea_get_max_length(objects.speed_motore_punz);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.speed_motore_punz;
            lv_textarea_set_text(objects.speed_motore_punz, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 50, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.speed_motore_ralla);
        uint32_t max_length = lv_textarea_get_max_length(objects.speed_motore_ralla);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.speed_motore_ralla;
            lv_textarea_set_text(objects.speed_motore_ralla, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_schermata_easter_egg() {
    void *flowState = getFlowState(0, 5);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.schermata_easter_egg = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_image_src(obj, &img_sfondo, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ANNULLA EASTER EGG
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.annulla_easter_egg = obj;
            lv_obj_set_pos(obj, 351, 24);
            lv_obj_set_size(obj, 114, 45);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_easter_egg_annulla_easter_egg, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_pulsante_rosso(obj);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Annulla_Diag_2
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.annulla_diag_2 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ANNULLA");
                }
            }
        }
    }
    
    tick_screen_schermata_easter_egg();
}

void tick_screen_schermata_easter_egg() {
    void *flowState = getFlowState(0, 5);
    (void)flowState;
}

void create_screen_schermata_logger() {
    void *flowState = getFlowState(0, 6);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.schermata_logger = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_image_src(obj, &img_sfondo, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ANNULLA PSSW_2
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.annulla_pssw_2 = obj;
            lv_obj_set_pos(obj, 354, 24);
            lv_obj_set_size(obj, 114, 45);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_logger_annulla_pssw_2, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_pulsante_rosso(obj);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Annulla_Diag_1
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.annulla_diag_1 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ANNULLA");
                }
            }
        }
        {
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 15, 15);
            lv_obj_set_size(obj, 326, 292);
            lv_textarea_set_max_length(obj, 1e+160);
            lv_textarea_set_placeholder_text(obj, "LOGGER");
            lv_textarea_set_one_line(obj, false);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_logger_obj3, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_AUTO);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 360, 257);
            lv_obj_set_size(obj, 100, 50);
            lv_obj_add_event_cb(obj, event_handler_cb_schermata_logger_obj4, LV_EVENT_ALL, flowState);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "Refresh");
                }
            }
        }
    }
    
    tick_screen_schermata_logger();
}

void tick_screen_schermata_logger() {
    void *flowState = getFlowState(0, 6);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 3, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.obj3);
        uint32_t max_length = lv_textarea_get_max_length(objects.obj3);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.obj3;
            lv_textarea_set_text(objects.obj3, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_scheramta_config_wi_fi() {
    void *flowState = getFlowState(0, 7);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.scheramta_config_wi_fi = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_image_src(obj, &img_sfondo, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ANNULLA CONFIG WIFI
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.annulla_config_wifi = obj;
            lv_obj_set_pos(obj, 0, 3);
            lv_obj_set_size(obj, 115, 45);
            lv_obj_add_event_cb(obj, event_handler_cb_scheramta_config_wi_fi_annulla_config_wifi, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_pulsante_rosso(obj);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // Annulla_Config_wifi2
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.annulla_config_wifi2 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "ANNULLA");
                }
            }
        }
        {
            // Password Rete Wifi
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.password_rete_wifi = obj;
            lv_obj_set_pos(obj, 140, 109);
            lv_obj_set_size(obj, 234, 40);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_placeholder_text(obj, "PASSWORD");
            lv_textarea_set_one_line(obj, true);
            lv_textarea_set_password_mode(obj, true);
            lv_obj_add_event_cb(obj, event_handler_cb_scheramta_config_wi_fi_password_rete_wifi, LV_EVENT_ALL, flowState);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Nome Rete Wifi
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.nome_rete_wifi = obj;
            lv_obj_set_pos(obj, 149, 57);
            lv_obj_set_size(obj, 216, 40);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_placeholder_text(obj, "NOME RETE");
            lv_textarea_set_one_line(obj, true);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_add_event_cb(obj, event_handler_cb_scheramta_config_wi_fi_nome_rete_wifi, LV_EVENT_ALL, flowState);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Config Wifi
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.config_wifi = obj;
            lv_obj_set_pos(obj, 132, 14);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Configurazione WiFi");
        }
        {
            // Tastiera Config WiFi
            lv_obj_t *obj = lv_keyboard_create(parent_obj);
            objects.tastiera_config_wi_fi = obj;
            lv_obj_set_pos(obj, 0, 160);
            lv_obj_set_size(obj, 480, 160);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // Messaggio Config WiFi
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.messaggio_config_wi_fi = obj;
            lv_obj_set_pos(obj, 90, 54);
            lv_obj_set_size(obj, 300, 125);
            lv_textarea_set_max_length(obj, 128);
            lv_textarea_set_one_line(obj, false);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_add_event_cb(obj, event_handler_cb_scheramta_config_wi_fi_messaggio_config_wi_fi, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 200, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_scheramta_config_wi_fi();
}

void tick_screen_scheramta_config_wi_fi() {
    void *flowState = getFlowState(0, 7);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 3, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.password_rete_wifi);
        uint32_t max_length = lv_textarea_get_max_length(objects.password_rete_wifi);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.password_rete_wifi;
            lv_textarea_set_text(objects.password_rete_wifi, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 4, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.nome_rete_wifi);
        uint32_t max_length = lv_textarea_get_max_length(objects.nome_rete_wifi);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.nome_rete_wifi;
            lv_textarea_set_text(objects.nome_rete_wifi, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 7, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.messaggio_config_wi_fi);
        uint32_t max_length = lv_textarea_get_max_length(objects.messaggio_config_wi_fi);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.messaggio_config_wi_fi;
            lv_textarea_set_text(objects.messaggio_config_wi_fi, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_schermata_calibrazione() {
    void *flowState = getFlowState(0, 8);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.schermata_calibrazione = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 320);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    tick_screen_schermata_calibrazione();
}

void tick_screen_schermata_calibrazione() {
    void *flowState = getFlowState(0, 8);
    (void)flowState;
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_schermata_avvio,
    tick_screen_schermata_principale,
    tick_screen_schermata_password,
    tick_screen_schermata_diagnostica,
    tick_screen_schermata_jogger,
    tick_screen_schermata_easter_egg,
    tick_screen_schermata_logger,
    tick_screen_scheramta_config_wi_fi,
    tick_screen_schermata_calibrazione,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 9) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Styles
//

static const char *style_names[] = { "titoli schermata iniziale", "Pulsante Rosso", "puls jog", "puls rosso verde", "Stato Sensori", "giallo" };

extern void add_style(lv_obj_t *obj, int32_t styleIndex);
extern void remove_style(lv_obj_t *obj, int32_t styleIndex);

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
//
//

void create_screens() {
    // Initialize styles
    eez_flow_init_styles(add_style, remove_style);
    eez_flow_init_style_names(style_names, sizeof(style_names) / sizeof(const char *));

eez_flow_init_fonts(fonts, sizeof(fonts) / sizeof(ext_font_desc_t));

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    eez_flow_init_screen_names(screen_names, sizeof(screen_names) / sizeof(const char *));
    eez_flow_init_object_names(object_names, sizeof(object_names) / sizeof(const char *));
    
    // Create screens
    create_screen_schermata_avvio();
    create_screen_schermata_principale();
    create_screen_schermata_password();
    create_screen_schermata_diagnostica();
    create_screen_schermata_jogger();
    create_screen_schermata_easter_egg();
    create_screen_schermata_logger();
    create_screen_scheramta_config_wi_fi();
    create_screen_schermata_calibrazione();
}