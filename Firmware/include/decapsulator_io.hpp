#pragma once

/// Include la libreria per l'Arduino Environment
#include "Arduino.h"
/// Include la libreria per FreeRTOS
#include "FreeRTOS.h"
/// Include la libreria per la gestione dei pin e del debounce
#include "DebouncePinHandler.hpp"
/// Include la libreria custom del motion
#include "MotionControl.hpp"
/// Include la libreria per la gestione del servomotore
#include "ESP32Servo.h"
/// Include la libreria per il controllo della ventola
#include "FanCtrl.hpp"
/// Include la libreria per la gestione della SD Card
#include "SavingFilesSD.hpp"
/// Invlude la libreria per il TFT LCD
#include "TFT_eSPI.h"


#pragma region (EXTERNS)

extern MOTION MotRalla;
extern MOTION MotPunzone;
extern Servo ServoParatia;
extern FanCtrl Ventola;
extern DebPinHandler autoKill;
extern DebPinHandler cadutaCaps;
extern DebPinHandler presenzaCaps;
extern DebPinHandler rallaFault;
extern DebPinHandler punzoneFault;
extern TFT_eSPI tft;

/**
 * @brief Inizializza Motori, sensori, input e output del Decapsulator
 */
extern BaseType_t decapsulator_io_begin(void);

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




#pragma region (CAPSULA PROPERTIES)

#define MAX_CAPSULE_CONTAINER    (2)    // Ogni X Capsule deve essere svuotato
#define MAX_COFFEE_CONTAINER    (10)    // Ogni X Capsule deve essere svuotato
#define MAX_STABLE_MOTOR_SPEED (900.0)  // Gradi/secondo [°/s]
#define DIAMETRO_CAPSULA_MM     (54)    // millimetri [mm]
#define ALTEZZA_CAPSULA_MM      (37)    // millimetri [mm]
#define TEMPO_CADUTA_CAPSULA_MS ((float)(sqrt((float)DIAMETRO_CAPSULA_MM / (2 * 9.80665)))) // t_caduta = sqrt( h / (2g) );

#pragma region (CAPSULA PROPERTIES)




#pragma region (SENSORI PRESENZA E CADUTA CAPSULE)

#define PIECE_PRESENCE_PIN      (1)
#define PIECE_PASSED_PIN        (2)

#pragma endregion (SENSORI PRESENZA E CADUTA CAPSULE)




#pragma region (AUTOKILL)

#define AUTOKILL_DETECT_PIN    (41)
#define AUTOKILL_SHUTDOWN_PIN  (42)

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
#define VENTOLA_RES             (7)
/// Hz. Frequenza della PWM generata da LEDC (LED Control)
#define VENTOLA_FREQ        (35000) 
#define VENTOLA_PIN            (12)
/// [%], è il valore del duty cycle quando la macchina è inattiva ma deve comunque raffreddare i drivers
#define VENTOLA_SLOW_DUTY      (30) 

#pragma endregion (VENTOLA)




#pragma region (GHIGLIOTTINA)

/**
 * 
 *  GHIGLIOTTINA: defines per i dati dell'oggetto del servomotore per la ghigliottina
 * 
 */
#define SERVO_PIN              (11)
#define SERVO_CLOSED_POS      (180)
#define SERVO_OPEN_POS          (0)

#pragma endregion (GHIGLIOTTINA)




#pragma region (TAMBURO_SETTINGS)

/**
 * 
 *  TAMBURO: defines per i dati dell'oggetto del MotionControl del tamburo
 * 
 */
#define RALLA_MOTOR_STEPS     (200)
#define RALLA_TASK_PRIORITY     (9)           /** @attention: è importante che sia <= della priorità della task */
#define RALLA_MICROSTEP       (STEP_1_TO_8)
#define RALLA_SPEED           (450.0)  // Velocità di esecuzione relativo al tamburo in gradi al secondo [°/s]
#define GEAR_RATIO_RALLA        (3)           // Imposta un gear ratio 1/3 per la ralla

/// Driver DRV8825 pins
#define RALLA_DIRECTION_PIN    (39)
#define RALLA_STEP_PIN         (38)
#define RALLA_ENABLE_PIN       (48)
#define RALLA_RESET_PIN       (255)
#define RALLA_SLEEP_PIN       (255)
#define RALLA_FAULT_PIN        (40)


/// Parametri HOMING Ralla
#define RALLA_CALIB_INTR_OR_POLL (INTR)
#define RALLA_CALIB_PIN         (9)
#define RALLA_INPUT_PULL      (INPUT_PULLUP) /// Input pullup desidera che l'uscita del sensore sia dritta per funzionare in falling
#define RALLA_HOME_CONT_SPEED (600.0)   /* Gradi al secondo */
#define RALLA_HOME_SPEED      (250.0)   /* Gradi al secondo */
#define RALLA_CAM_SIGNAL      (ACTIVE_LOW)
#define RALLA_HOME_DIR        (DIR_POSITIVE)
#define RALLA_POST_HOME_POS_CONT (0.0) /*Gradi*/
#define RALLA_POST_HOME_POS   (90.0) /*Gradi*/

#pragma endregion (TAMBURO_SETTINGS)




#pragma region (PUNZONE_SETTINGS)

/**
 * 
 *  PUNZONE: defines per i dati dell'oggetto del MotionControl
 * 
 */

#define PUNZ_MOTOR_STEPS      (200)
#define PUNZ_TASK_PRIORITY      (9) /** @attention: è importante che sia <= della priorità della task */
#define PUNZ_MICROSTEP        (STEP_1_TO_2)
#define PUNZ_SPEED            (750.0) // Velocità di esecuzione relativo al punzone in gradi al secondo [°/s]
#define PUNZ_ROTATIONS_TOT     (15.0)
#define PUNZ_POST_HOME_ROTATE   (1.0) /* Torna indietro di una rotazione (2mm lineari)*/
#define PUNZ_MOVE_ROTATIONS    (14.8 - PUNZ_POST_HOME_ROTATE)

/// Driver DRV8825 pins
#define PUNZ_DIRECTION_PIN     (47)
#define PUNZ_STEP_PIN          (21)
#define PUNZ_ENABLE_PIN        (14)
#define PUNZ_RESET_PIN        (255)
#define PUNZ_SLEEP_PIN        (255)
#define PUNZ_FAULT_PIN         (13)

/// Parametri HOMING Punzone
#define PUNZ_HARD_LIM_INTR_OR_POLL (INTR)
#define PUNZ_MAX_POS_PIN       (18)
#define PUNZ_MIN_POS_PIN        (8)
#define PUNZ_INPUT_PULL       (INPUT_PULLUP) /// Input pullup desidera che l'uscita del sensore sia dritta per funzionare in falling
#define PUNZ_HOME_SPEED       (900.0)   /* Gradi al secondo */
#define PUNZ_CAM_SIGNAL       (ACTIVE_LOW)
#define PUNZ_HOME_DIR         (DIR_POSITIVE)
#define PUNZ_POST_HOME_POS    (PUNZ_POST_HOME_ROTATE * 360.0) /*Gradi*/

#pragma endregion (PUNZONE_SETTINGS)




#pragma region (TIMEOUTS)

#define TIMEOUT_X_MS "ESEMPIO"

#define TIMEOUT_PUNZ_HOME_MS (uint32_t)(200 + ((PUNZ_ROTATIONS_TOT*360.0 / PUNZ_HOME_SPEED) * 1000.0))

#define TIMEOUT_RALLA_HOME_MS (uint32_t)(200 + ((GEAR_RATIO_RALLA*90.0 / RALLA_HOME_SPEED) * 1000.0))

#define TIMEOUT_CADUTA_CAPS_MS (2 * (uint32_t)(TEMPO_CADUTA_CAPSULA_MS))

#pragma endregion (TIMEOUTS)




