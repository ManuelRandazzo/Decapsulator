#pragma once

#include "Arduino.h"
#include "FreeRTOS.h"
#include "semphr.h"


/// Tempo massimo in cui i mutex bloccano se non hanno risposta
#define MAX_MUTEX_BLOCK_TIME_TICKS pdMS_TO_TICKS(1000)
#define HMI_JOG_DIRECTION_QUEUE_LEN 5


extern SemaphoreHandle_t mutex_contatore_caps_totali;

extern SemaphoreHandle_t mutex_contatore_caps_ses;

extern SemaphoreHandle_t mutex_stato_avvio_macchina;

extern SemaphoreHandle_t mutex_comando_macchina;

extern SemaphoreHandle_t mutex_password_corretta;

extern SemaphoreHandle_t mutex_str_logger_txt;

extern SemaphoreHandle_t mutex_comando_motore_ralla;

extern SemaphoreHandle_t mutex_comando_motore_punzone;

extern QueueHandle_t queue_direzione_comando_punzone;

extern SemaphoreHandle_t mutex_comando_avanti_motore_punzone;

extern SemaphoreHandle_t mutex_comando_indietro_motore_punzone;

extern QueueHandle_t queue_direzione_comando_ralla;

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

extern SemaphoreHandle_t mutex_calibrazione_touch_finita;

extern SemaphoreHandle_t mutex_password_rete_corretta;

extern SemaphoreHandle_t mutex_nome_rete_corretta;

extern SemaphoreHandle_t mutex_presenza_errore;

extern SemaphoreHandle_t mutex_nome_errore;

extern SemaphoreHandle_t mutex_pulsante_errore;

extern SemaphoreHandle_t mutex_nome_rete_inserita;

extern SemaphoreHandle_t mutex_password_rete_inserita;

extern SemaphoreHandle_t mutex_wi_fi_success;

extern SemaphoreHandle_t mutex_presenza_wi_fi;

extern SemaphoreHandle_t mutex_date_time_string;

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Inizializza i semafori delle variabili della UI
/// @return Se sono stati inizializzati i mutex delle variabili della UI
extern bool ui_init_var_mutexs();


#ifdef __cplusplus
}
#endif