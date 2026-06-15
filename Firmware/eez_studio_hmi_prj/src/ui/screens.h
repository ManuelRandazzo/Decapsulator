#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_SCHERMATA_AVVIO = 1,
    SCREEN_ID_SCHERMATA_PRINCIPALE = 2,
    SCREEN_ID_SCHERMATA_PASSWORD = 3,
    SCREEN_ID_SCHERMATA_DIAGNOSTICA = 4,
    SCREEN_ID_SCHERMATA_JOGGER = 5,
    SCREEN_ID_SCHERMATA_EASTER_EGG = 6,
    SCREEN_ID_SCHERMATA_LOGGER = 7,
    SCREEN_ID_SCHERAMTA_CONFIG_WI_FI = 8,
    SCREEN_ID_SCHERMATA_CALIBRAZIONE = 9,
    _SCREEN_ID_LAST = 9
};

typedef struct _objects_t {
    lv_obj_t *schermata_avvio;
    lv_obj_t *schermata_principale;
    lv_obj_t *schermata_password;
    lv_obj_t *schermata_diagnostica;
    lv_obj_t *schermata_jogger;
    lv_obj_t *schermata_easter_egg;
    lv_obj_t *schermata_logger;
    lv_obj_t *scheramta_config_wi_fi;
    lv_obj_t *schermata_calibrazione;
    lv_obj_t *stato_avvio;
    lv_obj_t *logo;
    lv_obj_t *conteggio_capsule;
    lv_obj_t *capsule_sessione;
    lv_obj_t *vis_cont_caps_ses;
    lv_obj_t *cont_caps_ses;
    lv_obj_t *cont_caps_tot;
    lv_obj_t *vis_cont_caps_tot;
    lv_obj_t *capsule_totali;
    lv_obj_t *impostazioni;
    lv_obj_t *puls_start_stop;
    lv_obj_t *start_stop;
    lv_obj_t *finestra_errore;
    lv_obj_t *puls_ok;
    lv_obj_t *ok;
    lv_obj_t *titolo_errore;
    lv_obj_t *obj0;
    lv_obj_t *tastiera_pssw;
    lv_obj_t *password;
    lv_obj_t *annulla_pssw;
    lv_obj_t *annulla;
    lv_obj_t *messaggio_pssw;
    lv_obj_t *annula_diagnostica;
    lv_obj_t *annulla_diag;
    lv_obj_t *logger;
    lv_obj_t *logger_titolo;
    lv_obj_t *contrmot;
    lv_obj_t *contrmot_titolo;
    lv_obj_t *blocco_jogger;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *jog_motori;
    lv_obj_t *annulla_jogger;
    lv_obj_t *annulla_3;
    lv_obj_t *controllo_m_otori;
    lv_obj_t *contrl_mot_ralla;
    lv_obj_t *contr_ralla;
    lv_obj_t *puls_en_dis_ralla;
    lv_obj_t *en_dis_ralla;
    lv_obj_t *puls_jog_av_ralla;
    lv_obj_t *av_ralla;
    lv_obj_t *puls_jog_ind_ralla;
    lv_obj_t *ind_ralla;
    lv_obj_t *stato_movimento_ralla;
    lv_obj_t *led_stato_movimento_ralla;
    lv_obj_t *titolo_stato_ralla;
    lv_obj_t *contrl_mot_pun;
    lv_obj_t *contr_punz;
    lv_obj_t *puls_en_dis_punz;
    lv_obj_t *en_dis_punz;
    lv_obj_t *puls_jog_av_punz;
    lv_obj_t *av_punz;
    lv_obj_t *puls_jog_ind_punz;
    lv_obj_t *ind_punz;
    lv_obj_t *stato_movimento_punz;
    lv_obj_t *led_stato_movimento_punz;
    lv_obj_t *titolo_stato_punz;
    lv_obj_t *calibrazione;
    lv_obj_t *sensore_di_calibrazione;
    lv_obj_t *titolo_sensore_di_calibrazione;
    lv_obj_t *stato_sensore_di_calibrazione;
    lv_obj_t *gradi_per_click_ralla;
    lv_obj_t *finecorsa_max;
    lv_obj_t *titolo_finecorsa_max;
    lv_obj_t *stato_finecorsa_max;
    lv_obj_t *finecorsa_min;
    lv_obj_t *titolo_finecorsa_min;
    lv_obj_t *stato_finecorsa_min;
    lv_obj_t *homing;
    lv_obj_t *tit_homing;
    lv_obj_t *gradi_per_click_punz;
    lv_obj_t *speed_motore_punz;
    lv_obj_t *speed_motore_ralla;
    lv_obj_t *inserimento_valori;
    lv_obj_t *annulla_easter_egg;
    lv_obj_t *annulla_diag_2;
    lv_obj_t *annulla_pssw_2;
    lv_obj_t *annulla_diag_1;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *annulla_config_wifi;
    lv_obj_t *annulla_config_wifi2;
    lv_obj_t *config_wifi;
    lv_obj_t *messaggio_config_wi_fi;
} objects_t;

extern objects_t objects;

void create_screen_schermata_avvio();
void tick_screen_schermata_avvio();

void create_screen_schermata_principale();
void tick_screen_schermata_principale();

void create_screen_schermata_password();
void tick_screen_schermata_password();

void create_screen_schermata_diagnostica();
void tick_screen_schermata_diagnostica();

void create_screen_schermata_jogger();
void tick_screen_schermata_jogger();

void create_screen_schermata_easter_egg();
void tick_screen_schermata_easter_egg();

void create_screen_schermata_logger();
void tick_screen_schermata_logger();

void create_screen_scheramta_config_wi_fi();
void tick_screen_scheramta_config_wi_fi();

void create_screen_schermata_calibrazione();
void tick_screen_schermata_calibrazione();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/