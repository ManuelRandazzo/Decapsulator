#pragma once

// -*- Gruppo 1-FreeRTOS-C++

/*
 * This file is part of {{ Decapsulator }}.
 *
 * Developed for the ITS Meccatronico Montebelluna-Castelfranco Veneto 2024-2026 Gruppo 1.
 * This product includes software developed by the Gruppo 1 Project.
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is NOT free software: you can NOT redistribute it and/or modify it.
 *
 * The Documentation is made following the Doxygen standard documentation style
 *
 */

/// Include la libreria custom per la gestione delle task
#include "Tasks.hpp"
/// Inlcude il file per la gestione del debugger
#include "Debug.hpp"
/// Include la libreria custom del motion
#include "MotionControl.hpp"
/// Include la libreria per la gestione del servomotore
#include "ESP32Servo.h"
/// Include la libreria per il controllo della ventola
#include "FanCtrl.hpp"
/// Include la libreria per la gestione dei pin e del debounce
#include "DebouncePinHandler.hpp"
/// Include il file per la gestione della coda
#include "queue.h"
/// Include una classe per rilevare i Rising Trigger
#include "R_TRIG.hpp"
/// Include una classe per rilevare i Falling Trigger
#include "F_TRIG.hpp"

/**
 * @other_defines:
 */
#define MAX_CAPSULE_CONTAINER 5
#define MAX_STABLE_MOTOR_SPEED 900 // Gradi/secondo [°/s]
#define DIAMETRO_CAPSULA_MM 54 //millimetri [mm]
#define ALTEZZA_CAPSULA_MM  37 //millimetri [mm]
#define TEMPO_CADUTA_CAPSULA_MS (float)(sqrt((float)DIAMETRO_CAPSULA_MM / (2 * 9.80665))) // t_caduta = sqrt( h / (2g) );


#pragma region (SENSORI PRESENZA E CADUTA CAPSULE)

#define PIECE_PRESENCE_PIN      1
#define PIECE_PASSED_PIN        2

#pragma endregion (SENSORI PRESENZA E CADUTA CAPSULE)


#pragma region (AUTOKILL)

#define AUTOKILL_DETECT_PIN    41
#define AUTOKILL_SHUTDOWN_PIN  42

#pragma endregion (AUTOKILL)


#pragma region (VENTOLA)

/**
 * 
 * VENTOLA: defines per i dati dell'oggetto della ventola di raffreddamento
 * 
 */
/// Risoluzione in numero di bit della PWM. 
/// Risoluzione MASSIMA @formula: log₂(f_clk_periferica / f_pwm) - 1 --> log₂(80000000 / f_pwm) - 1
/// 7 Bit calcolato con : (log₂(f_clk_periferica / f_pwm) - 1) - 30% = (log₂(80000000 / 35000) - 1) * 0.7
#define VENTOLA_RES             7
/// Hz. Frequenza della PWM generata da LEDC (LED Control)
#define VENTOLA_FREQ        35000 
#define VENTOLA_PIN            12

#pragma endregion (VENTOLA)




#pragma region (GHIGLIOTTINA)

/**
 * 
 *  GHIGLIOTTINA: defines per i dati dell'oggetto del servomotore per la ghigliottina
 * 
 */
#define SERVO_PIN              11
#define SERVO_CLOSED_POS      180
#define SERVO_OPEN_POS          0

#pragma endregion (GHIGLIOTTINA)



#pragma region (TAMBURO_SETTINGS)

/**
 * 
 *  TAMBURO: defines per i dati dell'oggetto del MotionControl del tamburo
 * 
 */
#define RALLA_MOTOR_STEPS     200
#define RALLA_TASK_PRIORITY     2           /** @attention: è importante che sia <= della priorità della task */
#define RALLA_MICROSTEP       FULL_STEP //STEP_1_TO_16
#define RALLA_SPEED           MAX_STABLE_MOTOR_SPEED / (double)(RALLA_MICROSTEP)  // Velocità di esecuzione relativo al tamburo in gradi al secondo [°/s]
#define RALLA_ACC            2000.0   /* Gradi al secondo quadrato */
#define RALLA_DEC            2000.0   /* Gradi al secondo quadrato */
#define GEAR_RATIO_RALLA        3           // Imposta un gear ratio 1/3 per la ralla

/// Driver DRV8825 pins
#define RALLA_DIRECTION_PIN    39
#define RALLA_STEP_PIN         38
#define RALLA_ENABLE_PIN       48
#define RALLA_RESET_PIN       255
#define RALLA_SLEEP_PIN       255
#define RALLA_FAULT_PIN        40


/// Parametri HOMING Ralla
#define RALLA_CALIB_INTR_OR_POLL INTR
#define RALLA_CALIB_PIN         9
#define RALLA_INPUT_PULL      INPUT_PULLUP /// Input pullup desidera che l'uscita del sensore sia dritta per funzionare in falling
#define RALLA_HOME_SPEED      240.0   /* Gradi al secondo */
#define RALLA_HOME_ACC        500.0   /* Gradi al secondo quadrato */
#define RALLA_HOME_DEC        500.0   /* Gradi al secondo quadrato */
#define RALLA_CAM_SIGNAL      ACTIVE_LOW
#define RALLA_HOME_DIR        DIR_NEGATIVE
#define RALLA_POST_HOME_POS   (-120.0) /*Gradi*//** @attention  Ancora da definire*/

#pragma endregion (TAMBURO_SETTINGS)

#pragma region (PUNZONE_SETTINGS)

/**
 * 
 *  PUNZONE: defines per i dati dell'oggetto del MotionControl
 * 
 */

#define PUNZ_MOTOR_STEPS      200  
#define PUNZ_TASK_PRIORITY      2 /** @attention: è importante che sia <= della priorità della task */
#define PUNZ_MICROSTEP        FULL_STEP //STEP_1_TO_16
#define PUNZ_FAST_SPEED       MAX_STABLE_MOTOR_SPEED / (double)(PUNZ_MICROSTEP) // Velocità di esecuzione relativo al punzone in gradi al secondo [°/s]
#define PUNZ_SLOW_SPEED       500.0  /** @attention  Ancora da definire*/
#define PUNZ_ACC             2000.0  /* Gradi al secondo quadrato */
#define PUNZ_DEC             2000.0  /* Gradi al secondo quadrato */
#define PUNZ_ROTATIONS_TOT     15.8
#define PUNZ_SLOW_ROTATIONS    15.0 /** Altezza della capsula + 1mm *//** @attention  Ancora da definire*/
#define PUNZ_FAST_ROTATIONS   (PUNZ_ROTATIONS_TOT - PUNZ_SLOW_ROTATIONS)

/// Driver DRV8825 pins
#define PUNZ_DIRECTION_PIN     47
#define PUNZ_STEP_PIN          21
#define PUNZ_ENABLE_PIN        14
#define PUNZ_RESET_PIN        255
#define PUNZ_SLEEP_PIN        255
#define PUNZ_FAULT_PIN         13

/// Parametri HOMING Punzone
#define PUNZ_HARD_LIM_INTR_OR_POLL INTR
#define PUNZ_MAX_POS_PIN       18
#define PUNZ_MIN_POS_PIN        8
#define PUNZ_INPUT_PULL       INPUT_PULLUP /// Input pullup desidera che l'uscita del sensore sia dritta per funzionare in falling
#define PUNZ_HOME_SPEED       240.0   /* Gradi al secondo */  /** @attention  Ancora da definire*/
#define PUNZ_HOME_ACC         500.0   /* Gradi al secondo quadrato */
#define PUNZ_HOME_DEC         500.0   /* Gradi al secondo quadrato */
#define PUNZ_CAM_SIGNAL       ACTIVE_LOW
#define PUNZ_HOME_DIR         DIR_POSITIVE
#define PUNZ_POST_HOME_POS    (1.5 * 360.0) /*Gradi*/  /// Torna indietro di 1.5 giri

#pragma endregion (PUNZONE_SETTINGS)



#pragma region (TIMEOUTS)

#define TIMEOUT_X_MS "ESEMPIO"

#define TIMEOUT_PUNZ_HOME_MS (uint32_t)(200 + ((PUNZ_ROTATIONS_TOT*360.0 / PUNZ_HOME_SPEED) * 1000.0))

#define TIMEOUT_RALLA_HOME_MS (uint32_t)(200 + ((GEAR_RATIO_RALLA*90.0 / RALLA_HOME_SPEED) * 1000.0))

#define TIMEOUT_CADUTA_CAPS_MS (2 * (uint32_t)(TEMPO_CADUTA_CAPSULA_MS))

#pragma endregion (TIMEOUTS)






#pragma region (EXTERNS)

/// @brief Handler della task del programma
extern TaskHandle_t MainPrgHandler;
extern MOTION MotRalla;
extern MOTION MotPunzone;
extern Servo ServoParatia;
extern FanCtrl Ventola;

/// @brief PROGRAMMA PRINCIPALE
extern void prgDecapsulatorTask(void *pvParameters);

/**
 *  @brief funzione chiamata in caso di emergenza, di servizio o di timeout
 * 
 *  @param ptrStepsLeftTamburo puntatore a cui viene settato il numero di gradi rimanenti
 *         per completare il movimento che verrà abortito chiamando questa funziione 
 *  
 *  @param ptrStepsLeftPunzone puntatore a cui viene settato il numero di gradi rimanenti
 *         per completare il movimento che verrà abortito chiamando questa funziione
 * 
 *  @note Se non si implementa una logica per riprendere l'ultimo movimento allora
 *        dopo aver chiamato questa funzione va rifatto l'homing
 * 
 *  @details Fa l'ABORT dei commandi attuali dei motori, FERMA i motori e fa DETACH dei pin RILASCIANDO la coppia
 */
extern void MainPrgStopAllMotors(double* ptrStepsLeftTamburo = nullptr, double* ptrStepsLeftPunzone = nullptr);

#pragma endregion (EXTERNS)




#pragma region (HMI)

#define TICKS_TO_WAIT_QUEUE_RECEIVE_HMI (pdMS_TO_TICKS(0))
#define TICKS_TO_WAIT_QUEUE_SEND_HMI    (pdMS_TO_TICKS(0))
#define HMI_CMD_QUEUE_LEN 5
#define HMI_EVT_QUEUE_LEN 5


#pragma region (COMMAND_QUEUE)

typedef enum : int8_t
{
  NO_CMD = -1,
  TAMBURO_ENABLE,
  TAMBURO_DISABLE,
  TAMBURO_JOG_POSITIVE,
  TAMBURO_JOG_NEGATIVE,

  PUNZONE_ENABLE,
  PUNZONE_DISABLE,
  PUNZONE_JOG_POSITIVE,
  PUNZONE_JOG_NEGATIVE,
  
  HOMING,

} JogState;


/// @brief Handler della coda che riceve i comandi che arrivano dall'HMI
extern QueueHandle_t QueueHandlerHMI_CMD;

/// @brief Direction: Frontend --> Backend 
struct CommandQueueHMI_t
{
  unsigned StartMachine : 1;
  unsigned StopMachine : 1;
  unsigned restartAfterContainerEmptied : 1;

  /// JOG
  unsigned jogPageActive : 1;

  JogState jogStateCMD;

  float    jogRallaGradiPerClick;   /// Quanti gradi fa ad ogni click del pulsante
  float    jogRallaSpeed;           /// Velocità del jog in gradi al secondo

  float    jogPunzoneGradiPerClick;   /// Quanti gradi fa ad ogni click del pulsante
  float    jogPunzoneSpeed;           /// Velocità del jog in gradi al secondo

  unsigned unusedBits : 20; // = 32 - n° bit occupati dagli altri membri
};

/// Struct di inizializzazione (evita errori nell'utilizzo di membri non inizializzati)
constexpr CommandQueueHMI_t defaultCommandQueueHMI =
{
  .StartMachine = false,
  .StopMachine = false,
  .restartAfterContainerEmptied = false,
  
  /// JOG
  .jogPageActive = false,

  .jogStateCMD = NO_CMD,
  
  .jogRallaGradiPerClick = 0,     /// Quanti gradi fa ad ogni click del pulsante
  .jogRallaSpeed = 0.0,         /// Velocità del jog in gradi al secondo
  
  .jogPunzoneGradiPerClick = 0.0, /// Quanti gradi fa ad ogni click del pulsante
  .jogPunzoneSpeed = 0.0,         /// Velocità del jog in gradi al secondo
  
  .unusedBits = 0,
};

#pragma endregion (COMMAND_QUEUE)

#pragma region (EVENT_QUEUE)

/// @brief Handler della coda che invia gli eventi (errori, avvisi, ecc.) all'HMI
extern QueueHandle_t QueueHandlerHMI_EVT; 

/// @brief Direction: Backend --> Frontend 
struct EventQueueHMI_t
{
  unsigned xShutDown : 1;
  
  /// ERRORI:
  unsigned xErrorInitPunz : 1;
  unsigned xErrorInitRalla : 1;
  unsigned xErrorCapsIncastrata : 1;
  //unsigned  xError : 1;
  
  /// JOG
  unsigned jogRallaIsMoving : 1;    /// Indica se il motore si sta muovendo
  unsigned jogRallaCalibrationStatus : 1;
  unsigned jogPunzoneIsMoving : 1;    /// Indica se il motore si sta muovendo
  unsigned jogPunzoneFineCorsaMaxStatus : 1;
  unsigned jogPunzoneFineCorsaMinStatus : 1;

  unsigned unusedBits : 23; // = 32 - n° bit occupati dagli altri membri
};

/// Struct di inizializzazione (evita errori nell'utilizzo di membri non inizializzati)
constexpr EventQueueHMI_t defaultEventQueueHMI =
{
  .xErrorInitPunz = false,
  .xErrorInitRalla = false,
  .xErrorCapsIncastrata = false,


  /// JOG
  .jogRallaIsMoving = false,   /// Indica se il motore si sta muovendo
  .jogRallaCalibrationStatus = false,
  .jogPunzoneIsMoving = false, /// Indica se il motore si sta muovendo
  .jogPunzoneFineCorsaMaxStatus = false,
  .jogPunzoneFineCorsaMinStatus = false,

  .unusedBits = 0,
};

#pragma endregion (EVENT_QUEUE)

extern BaseType_t checkUpdateHMI(CommandQueueHMI_t* receivingQueue); /// Direction: Frontend --> Backend
extern BaseType_t checkUpdateHMI(EventQueueHMI_t* receivingQueue);   /// Direction: Backend --> Frontend
extern BaseType_t sendUpdateHMI(CommandQueueHMI_t* sendingQueue, bool* sendChanges); /// Direction: Frontend --> Backend
extern BaseType_t sendUpdateHMI(EventQueueHMI_t* sendingQueue, bool* sendChanges);   /// Direction: Backend --> Frontend 

#pragma endregion (HMI)