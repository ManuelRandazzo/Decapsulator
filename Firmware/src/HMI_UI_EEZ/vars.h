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
    FLOW_GLOBAL_VARIABLE_AVVIO_MACCHINA = 0,
    FLOW_GLOBAL_VARIABLE_STATO_COMANDO_MACCHINA = 1,
    FLOW_GLOBAL_VARIABLE_STATO_AVVIO = 2,
    FLOW_GLOBAL_VARIABLE_PASSWORD_INSERITA = 3,
    FLOW_GLOBAL_VARIABLE_ESITO_CONTROLLO_PSWW = 4,
    FLOW_GLOBAL_VARIABLE_TITOLO_BOTTONE_RALLA = 5,
    FLOW_GLOBAL_VARIABLE_TITOLO_BOTTONE_PUNZONE = 6,
    FLOW_GLOBAL_VARIABLE_COMANDO_BOTTONE_RALLA = 7,
    FLOW_GLOBAL_VARIABLE_COMANDO_BOTTONE_PUNZONE = 8,
    FLOW_GLOBAL_VARIABLE_LED_MOVIMENTO_PUNZONE = 9,
    FLOW_GLOBAL_VARIABLE_LED_MOVIMENTO_RALLA = 10,
    FLOW_GLOBAL_VARIABLE_TITOLO_STATO_SENSORE_CALIBRAZIONE = 11,
    FLOW_GLOBAL_VARIABLE_TITOLO_STATO_FINECORSA_MIN = 12,
    FLOW_GLOBAL_VARIABLE_TITOLO_STATO_FINECORSA_MAX = 13
};

// Native global variables

extern float get_var_contatore_caps_totali();
extern void set_var_contatore_caps_totali(float value);
extern float get_var_contatore_caps_ses();
extern void set_var_contatore_caps_ses(float value);
extern int32_t get_var_stato_avvio_macchina();
extern void set_var_stato_avvio_macchina(int32_t value);
extern int32_t get_var_comando_macchina();
extern void set_var_comando_macchina(int32_t value);
extern const char *get_var_password_corretta();
extern void set_var_password_corretta(const char *value);
extern const char *get_var_str_logger_txt();
extern void set_var_str_logger_txt(const char *value);
extern int32_t get_var_comando_motore_ralla();
extern void set_var_comando_motore_ralla(int32_t value);
extern int32_t get_var_comando_motore_punzone();
extern void set_var_comando_motore_punzone(int32_t value);
extern bool get_var_comando_avanti_motore_punzone();
extern void set_var_comando_avanti_motore_punzone(bool value);
extern bool get_var_comando_indietro_motore_punzone();
extern void set_var_comando_indietro_motore_punzone(bool value);
extern bool get_var_comando_avanti_motore_ralla();
extern void set_var_comando_avanti_motore_ralla(bool value);
extern bool get_var_comando_indietro_motore_ralla();
extern void set_var_comando_indietro_motore_ralla(bool value);
extern int32_t get_var_stato_motore_punzone();
extern void set_var_stato_motore_punzone(int32_t value);
extern float get_var_speed_motore_punz();
extern void set_var_speed_motore_punz(float value);
extern int32_t get_var_stato_motore_ralla();
extern void set_var_stato_motore_ralla(int32_t value);
extern double get_var_gradi_per_click_ralla();
extern void set_var_gradi_per_click_ralla(double value);
extern double get_var_gradi_per_click_punz();
extern void set_var_gradi_per_click_punz(double value);
extern double get_var_speed_motore_ralla();
extern void set_var_speed_motore_ralla(double value);
extern bool get_var_stato_sensore_di_calibrazione();
extern void set_var_stato_sensore_di_calibrazione(bool value);
extern bool get_var_stato_finecorsa_max();
extern void set_var_stato_finecorsa_max(bool value);
extern bool get_var_stato_finecorsa_min();
extern void set_var_stato_finecorsa_min(bool value);
extern int32_t get_var_homing();
extern void set_var_homing(int32_t value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/