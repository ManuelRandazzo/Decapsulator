#include "Arduino.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "ui_vars_mutexs.h"
#include "vars.h"




float contatore_caps_totali = 0.0;
extern "C" float get_var_contatore_caps_totali()
{
    if(xSemaphoreTake(mutex_contatore_caps_totali, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0.0;

    float get = contatore_caps_totali;
        
    xSemaphoreGive(mutex_contatore_caps_totali);

    return get;
}

extern "C" void set_var_contatore_caps_totali(float value)
{
    if(xSemaphoreTake(mutex_contatore_caps_totali, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    contatore_caps_totali = value;

    xSemaphoreGive(mutex_contatore_caps_totali);

    return;
}



float contatore_caps_ses = 0.0;
extern "C" float get_var_contatore_caps_ses()
{
    if(xSemaphoreTake(mutex_contatore_caps_ses, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0.0;
    
    float get = contatore_caps_ses;

    xSemaphoreGive(mutex_contatore_caps_ses);

    return get;
}

extern "C" void set_var_contatore_caps_ses(float value)
{
    if(xSemaphoreTake(mutex_contatore_caps_ses, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    contatore_caps_ses = value;

    xSemaphoreGive(mutex_contatore_caps_ses);

    return;
}



int32_t stato_avvio_macchina = 0;
extern "C" int32_t get_var_stato_avvio_macchina()
{
    if(xSemaphoreTake(mutex_stato_avvio_macchina, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;

    int32_t get = stato_avvio_macchina;
    
    xSemaphoreGive(mutex_stato_avvio_macchina);

    return get;
}

extern "C" void set_var_stato_avvio_macchina(int32_t value)
{
    if(xSemaphoreTake(mutex_stato_avvio_macchina, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    stato_avvio_macchina = value;

    xSemaphoreGive(mutex_stato_avvio_macchina);

    return;
}




int32_t comando_macchina = 0;
extern "C" int32_t get_var_comando_macchina()
{
    if(xSemaphoreTake(mutex_comando_macchina, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;
    
    int32_t get = comando_macchina;

    xSemaphoreGive(mutex_comando_macchina);

    return get;
}

extern "C" void set_var_comando_macchina(int32_t value)
{
    if(xSemaphoreTake(mutex_comando_macchina, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_macchina = value;

    xSemaphoreGive(mutex_comando_macchina);

    return;
}




const char *password_corretta = "";
extern "C" const char *get_var_password_corretta()
{
    if(xSemaphoreTake(mutex_password_corretta, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = password_corretta;

    xSemaphoreGive(mutex_password_corretta);

    return get;
}

extern "C" void set_var_password_corretta(const char *value)
{
    if(xSemaphoreTake(mutex_password_corretta, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    password_corretta = value;

    xSemaphoreGive(mutex_password_corretta);

    return;
}




const char *str_logger_txt = "";
extern "C" const char *get_var_str_logger_txt()
{
    if(xSemaphoreTake(mutex_str_logger_txt, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return "";
    
    const char *get = str_logger_txt;

    xSemaphoreGive(mutex_str_logger_txt);

    return get;
}

extern "C" void set_var_str_logger_txt(const char *value)
{
    if(xSemaphoreTake(mutex_str_logger_txt, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    str_logger_txt = value;

    xSemaphoreGive(mutex_str_logger_txt);

    return;
}




int32_t comando_motore_ralla = 0;
extern "C" int32_t get_var_comando_motore_ralla()
{
    if(xSemaphoreTake(mutex_comando_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;
    
    int32_t get = comando_motore_ralla;

    xSemaphoreGive(mutex_comando_motore_ralla);

    return get;
}

extern "C" void set_var_comando_motore_ralla(int32_t value)
{
    if(xSemaphoreTake(mutex_comando_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_motore_ralla = value;

    xSemaphoreGive(mutex_comando_motore_ralla);

    return;
}




int32_t comando_motore_punzone = 0;
extern "C" int32_t get_var_comando_motore_punzone()
{
    if(xSemaphoreTake(mutex_comando_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;
    
    int32_t get = comando_motore_punzone;

    xSemaphoreGive(mutex_comando_motore_punzone);

    return get;
}

extern "C" void set_var_comando_motore_punzone(int32_t value)
{
    if(xSemaphoreTake(mutex_comando_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    comando_motore_punzone = value;

    xSemaphoreGive(mutex_comando_motore_punzone);

    return;
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

    return;
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

    return;
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

    return;
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

    return;
}




int32_t stato_motore_punzone = 0;
extern "C" int32_t get_var_stato_motore_punzone()
{
    if(xSemaphoreTake(mutex_stato_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;
    
    int32_t get = stato_motore_punzone;

    xSemaphoreGive(mutex_stato_motore_punzone);

    return get;
}

extern "C" void set_var_stato_motore_punzone(int32_t value)
{
    if(xSemaphoreTake(mutex_stato_motore_punzone, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    stato_motore_punzone = value;

    xSemaphoreGive(mutex_stato_motore_punzone);

    return;
}




float speed_motore_punz = 0.0;
extern "C" float get_var_speed_motore_punz()
{
    if(xSemaphoreTake(mutex_speed_motore_punz, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0.0;
    
    float get = speed_motore_punz;

    xSemaphoreGive(mutex_speed_motore_punz);

    return get;
}

extern "C" void set_var_speed_motore_punz(float value)
{
    if(xSemaphoreTake(mutex_speed_motore_punz, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    speed_motore_punz = value;

    xSemaphoreGive(mutex_speed_motore_punz);

    return;
}




int32_t stato_motore_ralla = 0;
extern "C" int32_t get_var_stato_motore_ralla()
{
    if(xSemaphoreTake(mutex_stato_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;
    
    int32_t get = stato_motore_ralla;

    xSemaphoreGive(mutex_stato_motore_ralla);

    return get;
}

extern "C" void set_var_stato_motore_ralla(int32_t value)
{
    if(xSemaphoreTake(mutex_stato_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    stato_motore_ralla = value;

    xSemaphoreGive(mutex_stato_motore_ralla);

    return;
}




double gradi_per_click_ralla = 0.0;
extern "C" double get_var_gradi_per_click_ralla()
{
    if(xSemaphoreTake(mutex_gradi_per_click_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0.0;
    
    double get = gradi_per_click_ralla;

    xSemaphoreGive(mutex_gradi_per_click_ralla);

    return get;
}

extern "C" void set_var_gradi_per_click_ralla(double value)
{
    if(xSemaphoreTake(mutex_gradi_per_click_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    gradi_per_click_ralla = value;

    xSemaphoreGive(mutex_gradi_per_click_ralla);

    return;
}




double gradi_per_click_punz = 0.0;
extern "C" double get_var_gradi_per_click_punz()
{
    if(xSemaphoreTake(mutex_gradi_per_click_punz, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0.0;
    
    double get = gradi_per_click_punz;

    xSemaphoreGive(mutex_gradi_per_click_punz);

    return get;
}

extern "C" void set_var_gradi_per_click_punz(double value)
{
    if(xSemaphoreTake(mutex_gradi_per_click_punz, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    gradi_per_click_punz = value;

    xSemaphoreGive(mutex_gradi_per_click_punz);

    return;
}




double speed_motore_ralla = 0.0;
extern "C" double get_var_speed_motore_ralla()
{
    if(xSemaphoreTake(mutex_speed_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0.0;
    
    double get = speed_motore_ralla;

    xSemaphoreGive(mutex_speed_motore_ralla);

    return get;
}

extern "C" void set_var_speed_motore_ralla(double value)
{
    if(xSemaphoreTake(mutex_speed_motore_ralla, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    speed_motore_ralla = value;

    xSemaphoreGive(mutex_speed_motore_ralla);

    return;
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

    return;
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

    return;
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

    return;
}




int32_t homing = 0;
extern "C" int32_t get_var_homing()
{
    if(xSemaphoreTake(mutex_homing, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return 0;
    
    int32_t get = homing;

    xSemaphoreGive(mutex_homing);

    return get;
}

extern "C" void set_var_homing(int32_t value)
{
    if(xSemaphoreTake(mutex_homing, MAX_MUTEX_BLOCK_TIME_TICKS) == pdFAIL)
        return;
    
    homing = value;

    xSemaphoreGive(mutex_homing);

    return;
}