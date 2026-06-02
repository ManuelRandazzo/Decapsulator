#pragma once

#include "Arduino.h"
#include "FreeRTOS.h"
#include "semphr.h"


/// Tempo massimo in cui i mutex bloccano se non hanno risposta
#define MAX_MUTEX_BLOCK_TIME_TICKS pdMS_TO_TICKS(100)

SemaphoreHandle_t mutex_contatore_caps_totali = nullptr;

SemaphoreHandle_t mutex_contatore_caps_ses = nullptr;

SemaphoreHandle_t mutex_stato_avvio_macchina = nullptr;

SemaphoreHandle_t mutex_comando_macchina = nullptr;

SemaphoreHandle_t mutex_password_corretta = nullptr;

SemaphoreHandle_t mutex_str_logger_txt = nullptr;

SemaphoreHandle_t mutex_comando_motore_ralla = nullptr;

SemaphoreHandle_t mutex_comando_motore_punzone = nullptr;

SemaphoreHandle_t mutex_comando_avanti_motore_punzone = nullptr;

SemaphoreHandle_t mutex_comando_indietro_motore_punzone = nullptr;

SemaphoreHandle_t mutex_comando_avanti_motore_ralla = nullptr;

SemaphoreHandle_t mutex_comando_indietro_motore_ralla = nullptr;

SemaphoreHandle_t mutex_stato_motore_punzone = nullptr;

SemaphoreHandle_t mutex_speed_motore_punz = nullptr;

SemaphoreHandle_t mutex_stato_motore_ralla = nullptr;

SemaphoreHandle_t mutex_gradi_per_click_ralla = nullptr;

SemaphoreHandle_t mutex_gradi_per_click_punz = nullptr;

SemaphoreHandle_t mutex_speed_motore_ralla = nullptr;

SemaphoreHandle_t mutex_stato_sensore_di_calibrazione = nullptr;

SemaphoreHandle_t mutex_stato_finecorsa_max = nullptr;

SemaphoreHandle_t mutex_stato_finecorsa_min = nullptr;

SemaphoreHandle_t mutex_homing = nullptr;



bool ui_init_var_mutexs()
{
    mutex_contatore_caps_totali = xSemaphoreCreateMutex();
    if(mutex_contatore_caps_totali == nullptr)
        return false;


    mutex_contatore_caps_ses = xSemaphoreCreateMutex();
    if(mutex_contatore_caps_ses == nullptr)
        return false;


    mutex_stato_avvio_macchina = xSemaphoreCreateMutex();
    if(mutex_stato_avvio_macchina == nullptr)
        return false;


    mutex_comando_macchina = xSemaphoreCreateMutex();
    if(mutex_comando_macchina == nullptr)
        return false;


    mutex_password_corretta = xSemaphoreCreateMutex();
    if(mutex_password_corretta == nullptr)
        return false;


    mutex_str_logger_txt = xSemaphoreCreateMutex();
    if(mutex_str_logger_txt == nullptr)
        return false;


    mutex_comando_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_comando_motore_ralla == nullptr)
        return false;


    mutex_comando_motore_punzone = xSemaphoreCreateMutex();
    if(mutex_comando_motore_punzone == nullptr)
        return false;


    mutex_comando_avanti_motore_punzone = xSemaphoreCreateMutex();
    if(mutex_comando_avanti_motore_punzone == nullptr)
        return false;


    mutex_comando_indietro_motore_punzone = xSemaphoreCreateMutex();
    if(mutex_comando_indietro_motore_punzone == nullptr)
        return false;


    mutex_comando_avanti_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_comando_avanti_motore_ralla == nullptr)
        return false;


    mutex_comando_indietro_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_comando_indietro_motore_ralla == nullptr)
        return false;


    mutex_stato_motore_punzone = xSemaphoreCreateMutex();
    if(mutex_stato_motore_punzone == nullptr)
        return false;


    mutex_speed_motore_punz = xSemaphoreCreateMutex();
    if(mutex_speed_motore_punz == nullptr)
        return false;


    mutex_stato_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_stato_motore_ralla == nullptr)
        return false;


    mutex_gradi_per_click_ralla = xSemaphoreCreateMutex();
    if(mutex_gradi_per_click_ralla == nullptr)
        return false;


    mutex_gradi_per_click_punz = xSemaphoreCreateMutex();
    if(mutex_gradi_per_click_punz == nullptr)
        return false;


    mutex_speed_motore_ralla = xSemaphoreCreateMutex();
    if(mutex_speed_motore_ralla == nullptr)
        return false;


    mutex_stato_sensore_di_calibrazione = xSemaphoreCreateMutex();
    if(mutex_stato_sensore_di_calibrazione == nullptr)
        return false;


    mutex_stato_finecorsa_max = xSemaphoreCreateMutex();
    if(mutex_stato_finecorsa_max == nullptr)
        return false;


    mutex_stato_finecorsa_min = xSemaphoreCreateMutex();
    if(mutex_stato_finecorsa_min == nullptr)
        return false;


    mutex_homing = xSemaphoreCreateMutex();
    if(mutex_homing == nullptr)
        return false;

        
    return true;
}