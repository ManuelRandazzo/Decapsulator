#include "Arduino.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "ui_vars_mutexs.h"
#include "HMI_UI_EEZ/vars.h"
#include "lvgl.h"




int32_t contatore_caps_totali = 0;
extern "C" int32_t get_var_contatore_caps_totali()
{
    if(xSemaphoreTake(mutex_contatore_caps_totali, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;

    int32_t get = contatore_caps_totali;
        
    xSemaphoreGive(mutex_contatore_caps_totali);

    return get;
}

extern "C" void set_var_contatore_caps_totali(int32_t value)
{
    if(xSemaphoreTake(mutex_contatore_caps_totali, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    contatore_caps_totali = value;

    xSemaphoreGive(mutex_contatore_caps_totali);
}



int32_t contatore_caps_ses = 0;
extern "C" int32_t get_var_contatore_caps_ses()
{
    if(xSemaphoreTake(mutex_contatore_caps_ses, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;
    
    int32_t get = contatore_caps_ses;

    xSemaphoreGive(mutex_contatore_caps_ses);

    return get;
}

extern "C" void set_var_contatore_caps_ses(int32_t value)
{
    if(xSemaphoreTake(mutex_contatore_caps_ses, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    contatore_caps_ses = value;

    xSemaphoreGive(mutex_contatore_caps_ses);
}



bool stato_avvio_macchina = false;
extern "C" bool get_var_stato_avvio_macchina()
{
    if(xSemaphoreTake(mutex_stato_avvio_macchina, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;

    bool get = stato_avvio_macchina;
    
    xSemaphoreGive(mutex_stato_avvio_macchina);

    return get;
}

extern "C" void set_var_stato_avvio_macchina(bool value)
{
    if(xSemaphoreTake(mutex_stato_avvio_macchina, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    stato_avvio_macchina = value;

    xSemaphoreGive(mutex_stato_avvio_macchina);
}




bool comando_macchina = false;
extern "C" bool get_var_comando_macchina()
{
    if(xSemaphoreTake(mutex_comando_macchina, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = comando_macchina;

    xSemaphoreGive(mutex_comando_macchina);

    return get;
}

extern "C" void set_var_comando_macchina(bool value)
{
    if(xSemaphoreTake(mutex_comando_macchina, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_macchina = value;
    
    xSemaphoreGive(mutex_comando_macchina);
}




String password_corretta = "";
extern "C" const char *get_var_password_corretta()
{
    if(xSemaphoreTake(mutex_password_corretta, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = password_corretta.c_str();

    xSemaphoreGive(mutex_password_corretta);

    return get;
}

extern "C" void set_var_password_corretta(const char *value)
{
    if(xSemaphoreTake(mutex_password_corretta, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    password_corretta = value;

    xSemaphoreGive(mutex_password_corretta);
}




String str_logger_txt = "";
extern "C" const char *get_var_str_logger_txt()
{
    if(xSemaphoreTake(mutex_str_logger_txt, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = str_logger_txt.c_str();

    xSemaphoreGive(mutex_str_logger_txt);

    return get;
}

extern "C" void set_var_str_logger_txt(const char *value)
{
    if(xSemaphoreTake(mutex_str_logger_txt, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    str_logger_txt = value;

    xSemaphoreGive(mutex_str_logger_txt);
}




bool comando_motore_ralla = false;
extern "C" bool get_var_comando_motore_ralla()
{
    if(xSemaphoreTake(mutex_comando_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = comando_motore_ralla;

    xSemaphoreGive(mutex_comando_motore_ralla);

    return get;
}

extern "C" void set_var_comando_motore_ralla(bool value)
{
    if(xSemaphoreTake(mutex_comando_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_motore_ralla = value;

    xSemaphoreGive(mutex_comando_motore_ralla);
}




bool comando_motore_punzone = 0;
extern "C" bool get_var_comando_motore_punzone()
{
    if(xSemaphoreTake(mutex_comando_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = comando_motore_punzone;

    xSemaphoreGive(mutex_comando_motore_punzone);

    return get;
}

extern "C" void set_var_comando_motore_punzone(bool value)
{
    if(xSemaphoreTake(mutex_comando_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_motore_punzone = value;

    xSemaphoreGive(mutex_comando_motore_punzone);
}




bool comando_avanti_motore_punzone = false;
extern "C" bool get_var_comando_avanti_motore_punzone()
{
    if(xSemaphoreTake(mutex_comando_avanti_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = comando_avanti_motore_punzone;

    xSemaphoreGive(mutex_comando_avanti_motore_punzone);

    return get;
}

extern "C" void set_var_comando_avanti_motore_punzone(bool value)
{
    if(xSemaphoreTake(mutex_comando_avanti_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_avanti_motore_punzone = value;

    xSemaphoreGive(mutex_comando_avanti_motore_punzone);

    if(value == true)
    {
        int8_t dir = +1;
        xQueueSend(queue_direzione_comando_punzone, &dir, MAX_MUTEX_BLOCK_TIME_TICKS);
    }
}




bool comando_indietro_motore_punzone = false;
extern "C" bool get_var_comando_indietro_motore_punzone()
{
    if(xSemaphoreTake(mutex_comando_indietro_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = comando_indietro_motore_punzone;

    xSemaphoreGive(mutex_comando_indietro_motore_punzone);

    return get;
}

extern "C" void set_var_comando_indietro_motore_punzone(bool value)
{
    if(xSemaphoreTake(mutex_comando_indietro_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_indietro_motore_punzone = value;

    xSemaphoreGive(mutex_comando_indietro_motore_punzone);

    if(value == true)
    {
        int8_t dir = -1;
        xQueueSend(queue_direzione_comando_punzone, &dir, MAX_MUTEX_BLOCK_TIME_TICKS);
    }
}




bool comando_avanti_motore_ralla = false;
extern "C" bool get_var_comando_avanti_motore_ralla()
{
    if(xSemaphoreTake(mutex_comando_avanti_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = comando_avanti_motore_ralla;

    xSemaphoreGive(mutex_comando_avanti_motore_ralla);

    return get;
}

extern "C" void set_var_comando_avanti_motore_ralla(bool value)
{
    if(xSemaphoreTake(mutex_comando_avanti_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_avanti_motore_ralla = value;

    xSemaphoreGive(mutex_comando_avanti_motore_ralla);

    if(value == true)
    {
        int8_t dir = +1;
        xQueueSend(queue_direzione_comando_ralla, &dir, MAX_MUTEX_BLOCK_TIME_TICKS);
    }
}




bool comando_indietro_motore_ralla = false;
extern "C" bool get_var_comando_indietro_motore_ralla()
{
    if(xSemaphoreTake(mutex_comando_indietro_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = comando_indietro_motore_ralla;

    xSemaphoreGive(mutex_comando_indietro_motore_ralla);

    return get;
}

extern "C" void set_var_comando_indietro_motore_ralla(bool value)
{
    if(xSemaphoreTake(mutex_comando_indietro_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_indietro_motore_ralla = value;

    xSemaphoreGive(mutex_comando_indietro_motore_ralla);

    if(value == true)
    {
        int8_t dir = -1;
        xQueueSend(queue_direzione_comando_ralla, &dir, MAX_MUTEX_BLOCK_TIME_TICKS);
    }
}




bool stato_motore_punzone = false;
extern "C" bool get_var_stato_motore_punzone()
{
    if(xSemaphoreTake(mutex_stato_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = stato_motore_punzone;

    xSemaphoreGive(mutex_stato_motore_punzone);

    return get;
}

extern "C" void set_var_stato_motore_punzone(bool value)
{
    if(xSemaphoreTake(mutex_stato_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    stato_motore_punzone = value;

    xSemaphoreGive(mutex_stato_motore_punzone);
}




String speed_motore_punz = "";
extern "C" const char *get_var_speed_motore_punz()
{
    if(xSemaphoreTake(mutex_speed_motore_punz, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = speed_motore_punz.c_str();

    xSemaphoreGive(mutex_speed_motore_punz);

    return get;
}

extern "C" void set_var_speed_motore_punz(const char *value)
{
    if(xSemaphoreTake(mutex_speed_motore_punz, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    speed_motore_punz = value;

    xSemaphoreGive(mutex_speed_motore_punz);
}




bool stato_motore_ralla = 0;
extern "C" bool get_var_stato_motore_ralla()
{
    if(xSemaphoreTake(mutex_stato_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = stato_motore_ralla;

    xSemaphoreGive(mutex_stato_motore_ralla);

    return get;
}

extern "C" void set_var_stato_motore_ralla(bool value)
{
    if(xSemaphoreTake(mutex_stato_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    stato_motore_ralla = value;

    xSemaphoreGive(mutex_stato_motore_ralla);
}




String gradi_per_click_ralla = "";
extern "C" const char *get_var_gradi_per_click_ralla()
{
    if(xSemaphoreTake(mutex_gradi_per_click_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = gradi_per_click_ralla.c_str();

    xSemaphoreGive(mutex_gradi_per_click_ralla);

    return get;
}

extern "C" void set_var_gradi_per_click_ralla(const char *value)
{
    if(xSemaphoreTake(mutex_gradi_per_click_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    gradi_per_click_ralla = value;

    xSemaphoreGive(mutex_gradi_per_click_ralla);
}




String gradi_per_click_punz = "";
extern "C" const char *get_var_gradi_per_click_punz()
{
    if(xSemaphoreTake(mutex_gradi_per_click_punz, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = gradi_per_click_punz.c_str();

    xSemaphoreGive(mutex_gradi_per_click_punz);

    return get;
}

extern "C" void set_var_gradi_per_click_punz(const char *value)
{
    if(xSemaphoreTake(mutex_gradi_per_click_punz, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    gradi_per_click_punz = value;

    xSemaphoreGive(mutex_gradi_per_click_punz);
}




String speed_motore_ralla = "";
extern "C" const char *get_var_speed_motore_ralla()
{
    if(xSemaphoreTake(mutex_speed_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = speed_motore_ralla.c_str();

    xSemaphoreGive(mutex_speed_motore_ralla);

    return get;
}

extern "C" void set_var_speed_motore_ralla(const char *value)
{
    if(xSemaphoreTake(mutex_speed_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    speed_motore_ralla = value;

    xSemaphoreGive(mutex_speed_motore_ralla);
}




bool stato_sensore_di_calibrazione = false;
extern "C" bool get_var_stato_sensore_di_calibrazione()
{
    if(xSemaphoreTake(mutex_stato_sensore_di_calibrazione, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = stato_sensore_di_calibrazione;

    xSemaphoreGive(mutex_stato_sensore_di_calibrazione);

    return get;
}

extern "C" void set_var_stato_sensore_di_calibrazione(bool value)
{
    if(xSemaphoreTake(mutex_stato_sensore_di_calibrazione, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    stato_sensore_di_calibrazione = value;

    xSemaphoreGive(mutex_stato_sensore_di_calibrazione);
}




bool stato_finecorsa_max = false;
extern "C" bool get_var_stato_finecorsa_max()
{
    if(xSemaphoreTake(mutex_stato_finecorsa_max, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = stato_finecorsa_max;

    xSemaphoreGive(mutex_stato_finecorsa_max);

    return get;
}

extern "C" void set_var_stato_finecorsa_max(bool value)
{
    if(xSemaphoreTake(mutex_stato_finecorsa_max, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    stato_finecorsa_max = value;

    xSemaphoreGive(mutex_stato_finecorsa_max);
}




bool stato_finecorsa_min = false;
extern "C" bool get_var_stato_finecorsa_min()
{
    if(xSemaphoreTake(mutex_stato_finecorsa_min, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = stato_finecorsa_min;

    xSemaphoreGive(mutex_stato_finecorsa_min);

    return get;
}

extern "C" void set_var_stato_finecorsa_min(bool value)
{
    if(xSemaphoreTake(mutex_stato_finecorsa_min, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    stato_finecorsa_min = value;

    xSemaphoreGive(mutex_stato_finecorsa_min);
}




bool homing = false;
extern "C" bool get_var_homing()
{
    if(xSemaphoreTake(mutex_homing, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;
    
    bool get = homing;

    xSemaphoreGive(mutex_homing);

    return get;
}

extern "C" void set_var_homing(bool value)
{
    if(xSemaphoreTake(mutex_homing, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    homing = value;

    xSemaphoreGive(mutex_homing);
}




bool calibrazione_touch_finita = false;
extern "C" bool get_var_calibrazione_touch_finita()
{
    if(xSemaphoreTake(mutex_calibrazione_touch_finita, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;
    
    bool get = calibrazione_touch_finita;

    xSemaphoreGive(mutex_calibrazione_touch_finita);

    return get;
}

extern "C" void set_var_calibrazione_touch_finita(bool value)
{
    if(xSemaphoreTake(mutex_calibrazione_touch_finita, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    calibrazione_touch_finita = value;

    xSemaphoreGive(mutex_calibrazione_touch_finita);
}




String nome_rete_inserita = "";
extern "C" const char *get_var_nome_rete_inserita()
{
    if(xSemaphoreTake(mutex_nome_rete_inserita, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = nome_rete_inserita.c_str();

    xSemaphoreGive(mutex_nome_rete_inserita);

    return get;
}

extern "C" void set_var_nome_rete_inserita(const char *value)
{
    if(xSemaphoreTake(mutex_nome_rete_inserita, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    nome_rete_inserita = value;

    xSemaphoreGive(mutex_nome_rete_inserita);
}




String password_rete_inserita = "";
extern  "C" const char *get_var_password_rete_inserita()
{
    if(xSemaphoreTake(mutex_password_rete_inserita, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = password_rete_inserita.c_str();

    xSemaphoreGive(mutex_password_rete_inserita);

    return get;
}

extern  "C" void set_var_password_rete_inserita(const char *value)
{
    if(xSemaphoreTake(mutex_password_rete_inserita, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    password_rete_inserita = value;

    xSemaphoreGive(mutex_password_rete_inserita);
}




bool presenza_errore = false;
extern "C" bool get_var_presenza_errore()
{
    if(xSemaphoreTake(mutex_presenza_errore, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;

    bool get = presenza_errore;

    xSemaphoreGive(mutex_presenza_errore);

    return get;
}

extern "C" void set_var_presenza_errore(bool value)
{
    if(xSemaphoreTake(mutex_presenza_errore, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    presenza_errore = value;

    xSemaphoreGive(mutex_presenza_errore);
}




String nome_errore = "";
extern "C" const char *get_var_nome_errore()
{
    if(xSemaphoreTake(mutex_nome_errore, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";

    const char *get = nome_errore.c_str();

    xSemaphoreGive(mutex_nome_errore);

    return get;
}

extern "C" void set_var_nome_errore(const char *value)
{
    /// Setta che è avvenuto un errore
    if(value != "")
        set_var_presenza_errore(true);

    if(xSemaphoreTake(mutex_nome_errore, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    nome_errore = value;

    xSemaphoreGive(mutex_nome_errore);
}




bool pulsante_errore = false;
extern "C" bool get_var_pulsante_errore()
{
    if(xSemaphoreTake(mutex_pulsante_errore, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;

    bool get = pulsante_errore;

    xSemaphoreGive(mutex_pulsante_errore);

    return get;
}

extern "C" void set_var_pulsante_errore(bool value)
{
    if(xSemaphoreTake(mutex_pulsante_errore, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    pulsante_errore = value;

    xSemaphoreGive(mutex_pulsante_errore);
}




bool wi_fi_success = false;
extern "C" bool get_var_wi_fi_success()
{
    if(xSemaphoreTake(mutex_wi_fi_success, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;

    bool get = wi_fi_success;

    xSemaphoreGive(mutex_wi_fi_success);

    return get;
}

extern "C" void set_var_wi_fi_success(bool value)
{
    if(xSemaphoreTake(mutex_wi_fi_success, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    wi_fi_success = value;

    xSemaphoreGive(mutex_wi_fi_success);
}




bool presenza_wi_fi = false;
extern "C" bool get_var_presenza_wi_fi()
{
    if(xSemaphoreTake(mutex_presenza_wi_fi, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;

    bool get = presenza_wi_fi;

    xSemaphoreGive(mutex_presenza_wi_fi);

    return get;
}

extern "C" void set_var_presenza_wi_fi(bool value)
{
    if(xSemaphoreTake(mutex_presenza_wi_fi, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    presenza_wi_fi = value;

    xSemaphoreGive(mutex_presenza_wi_fi);
}




bool date_time_string = false;
extern "C" bool get_var_date_time_string()
{
    if(xSemaphoreTake(mutex_date_time_string, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return false;

    bool get = date_time_string;

    xSemaphoreGive(mutex_date_time_string);

    return get;
}

extern "C" void set_var_date_time_string(bool value)
{
    if(xSemaphoreTake(mutex_date_time_string, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    date_time_string = value;

    xSemaphoreGive(mutex_date_time_string);
}