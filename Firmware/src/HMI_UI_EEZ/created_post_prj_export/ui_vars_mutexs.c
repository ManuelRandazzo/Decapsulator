#include "ui_vars_mutexs.h"

SemaphoreHandle_t mutex_contatore_caps_totali = NULL;

SemaphoreHandle_t mutex_contatore_caps_ses = NULL;

SemaphoreHandle_t mutex_stato_avvio_macchina = NULL;

SemaphoreHandle_t mutex_comando_macchina = NULL;

SemaphoreHandle_t mutex_password_corretta = NULL;

SemaphoreHandle_t mutex_str_logger_txt = NULL;

SemaphoreHandle_t mutex_comando_motore_ralla = NULL;

SemaphoreHandle_t mutex_comando_motore_punzone = NULL;

SemaphoreHandle_t mutex_comando_avanti_motore_punzone = NULL;

SemaphoreHandle_t mutex_comando_indietro_motore_punzone = NULL;

SemaphoreHandle_t mutex_comando_avanti_motore_ralla = NULL;

SemaphoreHandle_t mutex_comando_indietro_motore_ralla = NULL;

SemaphoreHandle_t mutex_stato_motore_punzone = NULL;

SemaphoreHandle_t mutex_speed_motore_punz = NULL;

SemaphoreHandle_t mutex_stato_motore_ralla = NULL;

SemaphoreHandle_t mutex_gradi_per_click_ralla = NULL;

SemaphoreHandle_t mutex_gradi_per_click_punz = NULL;

SemaphoreHandle_t mutex_speed_motore_ralla = NULL;

SemaphoreHandle_t mutex_stato_sensore_di_calibrazione = NULL;

SemaphoreHandle_t mutex_stato_finecorsa_max = NULL;

SemaphoreHandle_t mutex_stato_finecorsa_min = NULL;

SemaphoreHandle_t mutex_homing = NULL;

SemaphoreHandle_t mutex_calibrazione_touch_finita = NULL;

SemaphoreHandle_t mutex_password_rete_corretta = NULL;

SemaphoreHandle_t mutex_nome_rete_corretta = NULL;

SemaphoreHandle_t mutex_presenza_errore = NULL;

SemaphoreHandle_t mutex_nome_errore = NULL;

SemaphoreHandle_t mutex_connessione_presente = NULL;

SemaphoreHandle_t mutex_pulsante_errore = NULL;

bool ui_init_var_mutexs()
{
    mutex_contatore_caps_totali = xSemaphoreCreateMutex();
    if(mutex_contatore_caps_totali == NULL)
        return false;


    mutex_contatore_caps_ses = xSemaphoreCreateMutex();
    if(mutex_contatore_caps_ses == NULL)
        return false;


    mutex_stato_avvio_macchina = xSemaphoreCreateMutex();
    if(mutex_stato_avvio_macchina == NULL)
        return false;


    mutex_comando_macchina = xSemaphoreCreateMutex();
    if(mutex_comando_macchina == NULL)
        return false;


    mutex_password_corretta = xSemaphoreCreateMutex();
    if(mutex_password_corretta == NULL)
        return false;


    mutex_str_logger_txt = xSemaphoreCreateMutex();
    if(mutex_str_logger_txt == NULL)
        return false;


    mutex_comando_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_comando_motore_ralla == NULL)
        return false;


    mutex_comando_motore_punzone = xSemaphoreCreateMutex();
    if(mutex_comando_motore_punzone == NULL)
        return false;


    mutex_comando_avanti_motore_punzone = xSemaphoreCreateMutex();
    if(mutex_comando_avanti_motore_punzone == NULL)
        return false;


    mutex_comando_indietro_motore_punzone = xSemaphoreCreateMutex();
    if(mutex_comando_indietro_motore_punzone == NULL)
        return false;


    mutex_comando_avanti_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_comando_avanti_motore_ralla == NULL)
        return false;


    mutex_comando_indietro_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_comando_indietro_motore_ralla == NULL)
        return false;


    mutex_stato_motore_punzone = xSemaphoreCreateMutex();
    if(mutex_stato_motore_punzone == NULL)
        return false;


    mutex_speed_motore_punz = xSemaphoreCreateMutex();
    if(mutex_speed_motore_punz == NULL)
        return false;


    mutex_stato_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_stato_motore_ralla == NULL)
        return false;


    mutex_gradi_per_click_ralla = xSemaphoreCreateMutex();
    if(mutex_gradi_per_click_ralla == NULL)
        return false;


    mutex_gradi_per_click_punz = xSemaphoreCreateMutex();
    if(mutex_gradi_per_click_punz == NULL)
        return false;


    mutex_speed_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_speed_motore_ralla == NULL)
        return false;


    mutex_stato_sensore_di_calibrazione = xSemaphoreCreateMutex();
    if(mutex_stato_sensore_di_calibrazione == NULL)
        return false;


    mutex_stato_finecorsa_max = xSemaphoreCreateMutex();
    if(mutex_stato_finecorsa_max == NULL)
        return false;


    mutex_stato_finecorsa_min = xSemaphoreCreateMutex();
    if(mutex_stato_finecorsa_min == NULL)
        return false;


    mutex_homing = xSemaphoreCreateMutex();
    if(mutex_homing == NULL)
        return false;


    mutex_calibrazione_touch_finita = xSemaphoreCreateMutex();
    if(mutex_calibrazione_touch_finita == NULL)
        return false;

        
    mutex_password_rete_corretta = xSemaphoreCreateMutex();
    if(mutex_password_rete_corretta == NULL)
        return false;


    mutex_nome_rete_corretta = xSemaphoreCreateMutex();
    if(mutex_nome_rete_corretta == NULL)
        return false;

        
    mutex_presenza_errore = xSemaphoreCreateMutex();
    if(mutex_presenza_errore == NULL)
        return false;


    mutex_nome_errore = xSemaphoreCreateMutex();
    if(mutex_nome_errore == NULL)
        return false;

    
    mutex_connessione_presente = xSemaphoreCreateMutex();
    if(mutex_connessione_presente == NULL)
        return false;


    mutex_pulsante_errore = xSemaphoreCreateMutex();
    if(mutex_pulsante_errore == NULL)
        return false;
        

    return true;
}