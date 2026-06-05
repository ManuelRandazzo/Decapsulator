#pragma once

#include "Arduino.h"
#include "FreeRTOS.h"
#include "semphr.h"


/// Tempo massimo in cui i mutex bloccano se non hanno risposta
#define MAX_MUTEX_BLOCK_TIME_TICKS pdMS_TO_TICKS(100)

extern SemaphoreHandle_t mutex_contatore_caps_totali;

extern SemaphoreHandle_t mutex_contatore_caps_ses;

extern SemaphoreHandle_t mutex_stato_avvio_macchina;

extern SemaphoreHandle_t mutex_comando_macchina;

extern SemaphoreHandle_t mutex_password_corretta;

extern SemaphoreHandle_t mutex_str_logger_txt;

extern SemaphoreHandle_t mutex_comando_motore_ralla;

extern SemaphoreHandle_t mutex_comando_motore_punzone;

extern SemaphoreHandle_t mutex_comando_avanti_motore_punzone;

extern SemaphoreHandle_t mutex_comando_indietro_motore_punzone;

extern SemaphoreHandle_t mutex_comando_avanti_motore_ralla;

extern SemaphoreHandle_t mutex_comando_indietro_motore_ralla;

extern SemaphoreHandle_t mutex_stato_motore_punzone;

extern SemaphoreHandle_t mutex_speed_motore_punz;

extern SemaphoreHandle_t mutex_stato_motore_ralla;

extern SemaphoreHandle_t mutex_gradi_per_click_ralla;

extern SemaphoreHandle_t mutex_gradi_per_click_punz;

extern SemaphoreHandle_t mutex_speed_motore_ralla;

extern SemaphoreHandle_t mutex_stato_sensore_di_calibrazione;

extern SemaphoreHandle_t mutex_stato_finecorsa_max;

extern SemaphoreHandle_t mutex_stato_finecorsa_min;

extern SemaphoreHandle_t mutex_homing;

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Inizializza i semafori delle variabili della UI
/// @return Se sono stati inizializzati i mutex delle variabili della UI
extern bool ui_init_var_mutexs();


#ifdef __cplusplus
}
#endif