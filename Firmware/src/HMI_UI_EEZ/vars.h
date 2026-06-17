#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_STATO_COMANDO_MACCHINA = 0,
    FLOW_GLOBAL_VARIABLE_STATO_AVVIO = 1,
    FLOW_GLOBAL_VARIABLE_PASSWORD_INSERITA = 2,
    FLOW_GLOBAL_VARIABLE_ESITO_CONTROLLO_PSWW = 3,
    FLOW_GLOBAL_VARIABLE_TITOLO_BOTTONE_RALLA = 4,
    FLOW_GLOBAL_VARIABLE_TITOLO_BOTTONE_PUNZONE = 5,
    FLOW_GLOBAL_VARIABLE_COMANDO_BOTTONE_RALLA = 6,
    FLOW_GLOBAL_VARIABLE_COMANDO_BOTTONE_PUNZONE = 7,
    FLOW_GLOBAL_VARIABLE_LED_MOVIMENTO_PUNZONE = 8,
    FLOW_GLOBAL_VARIABLE_LED_MOVIMENTO_RALLA = 9,
    FLOW_GLOBAL_VARIABLE_TITOLO_STATO_SENSORE_CALIBRAZIONE = 10,
    FLOW_GLOBAL_VARIABLE_TITOLO_STATO_FINECORSA_MIN = 11,
    FLOW_GLOBAL_VARIABLE_TITOLO_STATO_FINECORSA_MAX = 12,
    FLOW_GLOBAL_VARIABLE_MESSAGGIO_CONFIG_WI_FI = 13,
    FLOW_GLOBAL_VARIABLE_QR_VISIBLE = 14
};

// Native global variables

extern int32_t get_var_contatore_caps_totali();
extern void set_var_contatore_caps_totali(int32_t value);
extern int32_t get_var_contatore_caps_ses();
extern void set_var_contatore_caps_ses(int32_t value);
extern bool get_var_stato_avvio_macchina();
extern void set_var_stato_avvio_macchina(bool value);
extern bool get_var_comando_macchina();
extern void set_var_comando_macchina(bool value);
extern const char *get_var_password_corretta();
extern void set_var_password_corretta(const char *value);
extern const char *get_var_str_logger_txt();
extern void set_var_str_logger_txt(const char *value);
extern bool get_var_comando_motore_ralla();
extern void set_var_comando_motore_ralla(bool value);
extern bool get_var_comando_motore_punzone();
extern void set_var_comando_motore_punzone(bool value);
extern bool get_var_comando_avanti_motore_punzone();
extern void set_var_comando_avanti_motore_punzone(bool value);
extern bool get_var_comando_indietro_motore_punzone();
extern void set_var_comando_indietro_motore_punzone(bool value);
extern bool get_var_comando_avanti_motore_ralla();
extern void set_var_comando_avanti_motore_ralla(bool value);
extern bool get_var_comando_indietro_motore_ralla();
extern void set_var_comando_indietro_motore_ralla(bool value);
extern bool get_var_stato_motore_punzone();
extern void set_var_stato_motore_punzone(bool value);
extern bool get_var_stato_motore_ralla();
extern void set_var_stato_motore_ralla(bool value);
extern const char *get_var_speed_motore_punz();
extern void set_var_speed_motore_punz(const char *value);
extern const char *get_var_gradi_per_click_ralla();
extern void set_var_gradi_per_click_ralla(const char *value);
extern const char *get_var_gradi_per_click_punz();
extern void set_var_gradi_per_click_punz(const char *value);
extern const char *get_var_speed_motore_ralla();
extern void set_var_speed_motore_ralla(const char *value);
extern bool get_var_stato_sensore_di_calibrazione();
extern void set_var_stato_sensore_di_calibrazione(bool value);
extern bool get_var_stato_finecorsa_max();
extern void set_var_stato_finecorsa_max(bool value);
extern bool get_var_stato_finecorsa_min();
extern void set_var_stato_finecorsa_min(bool value);
extern bool get_var_homing();
extern void set_var_homing(bool value);
extern bool get_var_calibrazione_touch_finita();
extern void set_var_calibrazione_touch_finita(bool value);
extern const char *get_var_nome_rete_inserita();
extern void set_var_nome_rete_inserita(const char *value);
extern const char *get_var_password_rete_inserita();
extern void set_var_password_rete_inserita(const char *value);
extern bool get_var_presenza_errore();
extern void set_var_presenza_errore(bool value);
extern const char *get_var_nome_errore();
extern void set_var_nome_errore(const char *value);
extern bool get_var_pulsante_errore();
extern void set_var_pulsante_errore(bool value);
extern bool get_var_wi_fi_success();
extern void set_var_wi_fi_success(bool value);
extern bool get_var_presenza_wi_fi();
extern void set_var_presenza_wi_fi(bool value);
extern const char *get_var_date_time_string();
extern void set_var_date_time_string(const char *value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/