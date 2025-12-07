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
/// Include il file custom della gestione degli Interrupts
#include "Interrupts.hpp"
/// Include la libreria custom del motion
#include "MotionControl.hpp"
/// Include la libreria per la gestione del servomotore
#include "ESP32Servo.h"

#if HAS_ATOMIC_BLOCK
// Acquire AVR-specific ATOMIC_BLOCK(ATOMIC_RESTORESTATE) macro.
#include <util/atomic.h>
#endif




/**
 * 
 *  @_not_defined_things: defines per i dati dell'oggetto del MotionControl del tamburo
 * 
 */
#define PIECE_PRESENCE_PIN 12  /** @warning  Ancora da definire*/
#define PIECE_PASSED_PIN   11  /** @warning  Ancora da definire*/
#define nFAULT_TAMBURO     7   /** @warning  Ancora da definire*/
#define nFAULT_PUNZONE     39  /** @warning  Ancora da definire*/


/**
 *  TIMEOUTS:
 */
#define TIMEOUT_X
#define TIMEOUT_Y



/**
 * 
 *  GHIGLIOTTINA: defines per i dati dell'oggetto del servomotore per la ghigliottina
 * 
 */
#define SERVO_PIN         37   /** @warning Ancora da definire. Pin del servo motore */
#define SERVO_MIN         0    /** Gradi minimi a cui il servo può arrivare */
#define SERVO_MAX         180  /** Gradi massimi a cui il servo può arrivare */
#define SERVO_CLOSED_POS  0    /** @warning Ancora da definire. Angolo di chiusura della paratia*/
#define SERVO_OPEN_POS    180  /** @warning Ancora da definire. Angolo di apertura della paratia*/



/**
 * 
 *  TAMBURO: defines per i dati dell'oggetto del MotionControl del tamburo
 * 
 */
#define RALLA_MOTOR_STEPS     200
#define RALLA_TASK_PRIORITY   3
#define RALLA_SPEED_MOVEMENT  2000        // Velocità di esecuzione relativo al tamburo in gradi al secondo [°/s]
#define GEAR_RATIO_RALLA      3           // Imposta un gear ratio 1/3 per la ralla

/// Driver DRV8825 pins
#define RALLA_DIRECTION_PIN   4             /** @warning  Ancora da definire*/
#define RALLA_STEP_PIN        5             /** @warning  Ancora da definire*/
#define RALLA_ENABLE_PIN      6             /** @warning  Ancora da definire*/
#define RALLA_RESET_PIN       255           /** @warning  Ancora da definire*/
#define RALLA_SLEEP_PIN       255           /** @warning  Ancora da definire*/
#define RALLA_FAULT_PIN       7             /** @warning  Ancora da definire*/


/// Parametri HOMING
#define RALLA_CALIB_PIN       14           /** @warning  Ancora da definire*/
#define RALLA_TRIGGER_MODE    FALLING
#define RALLA_HOME_SPEED      30            /** Gradi al secondo*/
#define RALLA_MICROSTEP       FULL_STEP //STEP_1_TO_16
#define RALLA_CAM_SIGNAL      ACTIVE_LOW
#define RALLA_HOME_DIR        DIR_NEGATIVE
#define RALLA_POST_HOME_POS   NULL          /** @warning  Ancora da definire*/
#define RALLA_TOCCHI_SENSORE  2             /** @warning  Ancora da definire*/


/**
 * 
 *  PUNZONE: defines per i dati dell'oggetto del MotionControl
 * 
 */

#define PUNZ_MOTOR_STEPS           200
#define PUNZ_TASK_PRIORITY         3
#define PUNZ_LOW_TORQUE_MOVEMENT   2800.0        // Velocità di esecuzione relativo al punzone in gradi al secondo [°/s]
#define PUNZ_HIGH_TORQUE_MOVEMENT  180.0
#define PUNZ_LOW_TORQUE_ROTATIONS  15.0
#define PUNZ_HIGH_TORQUE_ROTATIONS 10.0


/// Driver DRV8825 pins
#define PUNZ_DIRECTION_PIN      4             /** @warning  Ancora da definire*/
#define PUNZ_STEP_PIN           5             /** @warning  Ancora da definire*/
#define PUNZ_ENABLE_PIN         15            /** @warning  Ancora da definire*/
#define PUNZ_RESET_PIN          7             /** @warning  Ancora da definire*/
#define PUNZ_SLEEP_PIN          6             /** @warning  Ancora da definire*/
#define PUNZ_FAULT_PIN          16            /** @warning  Ancora da definire*/

/// Parametri HOMING Punzone
#define PUNZ_MAX_POS_PIN 13 /** @warning  Ancora da definire*/
#define PUNZ_MIN_POS_PIN 21 /** @warning  Ancora da definire*/

#define PUNZ_CALIB_PIN          PUNZ_MAX_POS_PIN
#define PUNZ_TRIGGER_MODE       CHANGE
#define PUNZ_HOME_SPEED         30            /** Gradi al secondo*/
#define PUNZ_MICROSTEP          FULL_STEP //STEP_1_TO_16
#define PUNZ_CAM_SIGNAL         ACTIVE_LOW
#define PUNZ_HOME_DIR           DIR_POSITIVE
#define PUNZ_POST_HOME_POS      NULL          /** @warning  Ancora da definire*/
#define PUNZ_TOCCHI_SENSORE     2             /** @warning  Ancora da definire*/



/**
 * 
 *    Modelli di funzioni utili al programma
 *  
 */
void capsulaPassataISR();
void fc_PunzoneAltoISR();
void fc_PunzoneBassoISR();
void calib_ralla_positionISR();



 
/**
 * 
 *        GLOBAL VARIABLE LIST :
 * 
 */

TaskHandle_t MainPrgHandler = nullptr;

typedef enum __sequence__
{
  EMERGENCY_STATE,
  MACHINE_STARTUP_STATE,
  MACHINE_STARTUP_FINISHED_STATE,
  QUIETE_STATE,
  REACH_NEXT_STATION_STATE,
  PUNCHER_DOWN_LOW_TORQUE_STATE,
  PUNCHER_DOWN_HIGH_TORQUE_STATE,
  SERVO_LOADER_OPEN_STATE,
  SERVO_LOADER_CLOSE_STATE,
  PUNCHER_UP_STATE,
} Sequence_t;
static Sequence_t sequenza = MACHINE_STARTUP_STATE;  // Gestione della sequenza del movimento del Decapsulator

static bool punzone_in_max_pos = false;
static bool punzone_in_min_pos = false;
static bool ralla_calibrated   = false;
static bool capsulaPassata     = false;
static uint32_t tmrDebounceServo;





/// Crea gli oggetti
/// Crea l'oggetto della classe TaskTypeDef, ovvero la task di gestione del decapsulator
TaskTypeDef DecapsulatorHandleTask;
MOTION MotRalla;
MOTION MotPunzone;
Servo ServoParatia;

/// Mutex e spinlock
SemaphoreHandle_t _DecapsulatorMutex = nullptr;
portMUX_TYPE _DecapsulatorSpinlock = portMUX_INITIALIZER_UNLOCKED;



/**
 *  @brief funzione chiamata in caso di emergenza
 */
extern void MainProgramEmergencyFunction()
{
  /// Stop del motore togliendo la coppia
  MotRalla.Stop(RELEASE);

  /// Stop del motore togliendo la coppia
  MotPunzone.Stop(RELEASE);

  /// Stop del servomotore togliendo la coppia e scollegando il pin
  ServoParatia.detach();
}



void prgDecapsulatorTask(void *pvParameters)
{
  /**
   *    @setup:
   */
  TickType_t getLastTick = xTaskGetTickCount();

  ///ATTENZIONE: Programma con variabili a caso ancora da definire
  ///            e da rendere THREAD SAFE
  bool FORCE_THE_STARTUP = false;
  bool startCycleFromHMI = false;
  int nCicliRimanenti = 0; // prende il numero dalla SD Card

  /**
   *  Motore Tamburo
   */
  MotRalla.Init(RALLA_MOTOR_STEPS, RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN,
                RALLA_TASK_PRIORITY, RALLA_MICROSTEP);
  LogInfo("Tamburo", "Tamburo Initialized");

  /**
   *  Motore Punzone
   */
  MotPunzone.Init(PUNZ_MOTOR_STEPS, PUNZ_DIRECTION_PIN, PUNZ_STEP_PIN, PUNZ_ENABLE_PIN, PUNZ_RESET_PIN, PUNZ_SLEEP_PIN,
                PUNZ_TASK_PRIORITY, PUNZ_MICROSTEP);
  LogInfo("Punzone", "Punzone Initialized");

  /**
   *  Capsula Caduta
   */
  attachInterrupt(PIECE_PASSED_PIN, capsulaPassataISR, CHANGE);
  LogInfo("Piece Passed", "Interrupt pin Piece Passed Initialized");

  /**
   *  Punzone Max Position
   */
  attachInterrupt(PUNZ_MAX_POS_PIN, fc_PunzoneAltoISR, CHANGE);
  LogInfo("Punzone in Max Position", "Interrupt pin Punzone in Max Position Initialized");

  /**
   *  Punzone Min Position
   */
  attachInterrupt(PUNZ_MIN_POS_PIN, fc_PunzoneBassoISR, CHANGE);
  LogInfo("Punzone in Min Position", "Interrupt pin Punzone in Min Position Initialized");

  /**
   *  Tamburo Calibration
   */
  attachInterrupt(PUNZ_MIN_POS_PIN, fc_PunzoneBassoISR, CHANGE);
  LogInfo("Punzone in Min Position", "Interrupt pin Punzone in Min Position Initialized");



  /**
   *    @loop:
   */
  while(1)
  {
    /**
     * @info: Gestione della sequenza completa del decapsulator
     */
    switch(sequenza)
    {
      case EMERGENCY_STATE :
        
      break;

      case MACHINE_STARTUP_STATE :
        /// Se il macchinario è chiuso e il pulsante di emergenza è alzato allora è possibile inizializzarlo
        // (dopodichè entrambi verranno gestiti dagli emergency interrupts)
        if(digitalRead(nFAULT_TAMBURO) == HIGH && digitalRead(nFAULT_PUNZONE) == HIGH)
        {
          /// Fa tutti gli attach dei motori e li prepara ad essere comandati
          MotRalla.attach();
          MotRalla.Start();
          /// Esegue l'homing del tamburo
          /*MotRalla.home(RALLA_CALIB_PIN, INPUT, RALLA_TRIGGER_MODE, RALLA_HOME_SPEED, RALLA_HOME_DIR,
                        25.0 * 360.0, RALLA_TOCCHI_SENSORE, RALLA_CAM_SIGNAL);*/
          MotRalla.moveContinuous(RALLA_HOME_DIR, RALLA_HOME_SPEED);

          MotPunzone.attach();
          MotPunzone.Start();
          MotPunzone.moveContinuous(DIR_NEGATIVE, PUNZ_HOME_SPEED);

          ServoParatia.attach(SERVO_PIN, SERVO_MIN, SERVO_MAX);
          ServoParatia.write(SERVO_CLOSED_POS); // Chiude la paratia mossa dal servomotore

          sequenza = MACHINE_STARTUP_FINISHED_STATE;
        }
      break;

      case MACHINE_STARTUP_FINISHED_STATE :
          /// Una volta Inizializzato tutto...(motori in posizione ecc.)
          /// Passa allo stato di quiete in cui attende il comando di start
          DebugOverrideVar<bool>("forceStrartup/value", &FORCE_THE_STARTUP);
          if(FORCE_THE_STARTUP || (punzone_in_max_pos == true && punzone_in_min_pos == false && ralla_calibrated == true))
          {
            /// Permette la ricalibrazione della ralla
            ralla_calibrated = false;
            LogInfo("Decapsultor PRG", "Startup completato, continuo con il programma");
            LogInfo("Decapsultor PRG", "Entro nello stato: QUIETE");
            sequenza = QUIETE_STATE;
          }
      break;

      case QUIETE_STATE :
        /// Se sono presenti le capsule nello scivolo e c'è stato il segnale di start inizia il ciclo
        if(startCycleFromHMI && digitalRead(PIECE_PRESENCE_PIN) && MotRalla.isStepDone() && MotPunzone.isStepDone())
        {
          LogInfo("Decapsultor PRG", "Entro nello stato: REACH_NEXT_STATION_STATE");
          sequenza = REACH_NEXT_STATION_STATE;
        }
      break;

      case REACH_NEXT_STATION_STATE :

        if(MotRalla.isStepDone() && MotPunzone.isStepDone())
        {
          /// Setta la direzione di marcia del tamburo e si muove alla posizione successiva
          MotRalla.moveRel(+90.0 * GEAR_RATIO_RALLA, RALLA_SPEED_MOVEMENT);

          /// Se c'è il pezzo vuol dire che rimangono sempre almeno 3 cicli, altrimenti decrementa
          nCicliRimanenti = digitalRead(PIECE_PRESENCE_PIN) ? 3 : nCicliRimanenti-1;

          if(nCicliRimanenti == 0)
          {
            LogInfo("Decapsultor PRG", "Entro nello stato: QUITE_STATE");
            sequenza = QUIETE_STATE;            
          }
          else
          {
            LogInfo("Decapsultor PRG", "Entro nello stato: PUNCHER_DOWN_LOW_TORQUE_STATE");
            sequenza = PUNCHER_DOWN_LOW_TORQUE_STATE;
          }
        }
      break;

      case PUNCHER_DOWN_LOW_TORQUE_STATE :
        if(MotRalla.isStepDone())
        {
          /// Setta la direzione di marcia del punzone e mette in coda 
          /// due movimenti uno veloce (bassa coppia) e uno lento (alta coppia)
          MotPunzone.moveRel(PUNZ_LOW_TORQUE_ROTATIONS * 360.0, PUNZ_LOW_TORQUE_MOVEMENT); // Fa 10 giri = 20mm lineari ad alta velocità
          LogInfo("Decapsultor PRG", "Entro nello stato: PUNCHER_DOWN_HIGH_TORQUE_STATE");
          sequenza = PUNCHER_DOWN_HIGH_TORQUE_STATE;
        }
      break;

      case PUNCHER_DOWN_HIGH_TORQUE_STATE :
        if(MotRalla.isStepDone())
        {
          MotPunzone.moveRel(PUNZ_HIGH_TORQUE_ROTATIONS  * 360.0, PUNZ_HIGH_TORQUE_MOVEMENT);    // Fa 15 giri = 30mm lineari ad alta coppia
          LogInfo("Decapsultor PRG", "Entro nello stato: SERVO_LOADER_OPEN_STATE");
          sequenza = SERVO_LOADER_OPEN_STATE;      
        }
      break;

      case SERVO_LOADER_OPEN_STATE :
        if(MotRalla.isStepDone() && digitalRead(RALLA_CALIB_PIN))
        {
          ServoParatia.write(SERVO_OPEN_POS);
          /// Cambio di stato dovuto dall'Interrupt della Fotocellula conferma capsula nel tamburo
        }
      break;

      case SERVO_LOADER_CLOSE_STATE :
        /// Se il pezzo è passato, i 30ms di debounce sono passati e non si è intasato
        if(!digitalRead(PIECE_PASSED_PIN) && (millis() - tmrDebounceServo >= 30))
        {
          ServoParatia.write(SERVO_CLOSED_POS);
          LogInfo("Decapsultor PRG", "Entro nello stato: PUNCHER_UP_STATE");
          sequenza = PUNCHER_UP_STATE;
        }
      break;

      case PUNCHER_UP_STATE :
        MotPunzone.moveRel((PUNZ_LOW_TORQUE_ROTATIONS + PUNZ_HIGH_TORQUE_ROTATIONS) * -360, PUNZ_LOW_TORQUE_MOVEMENT);
        LogInfo("Decapsultor PRG", "Entro nello stato: PUNCHER_UP_STATE");
      break;
    }

    xTaskDelayUntil(&getLastTick, MainPrg_delay);
  }
}





/**
╔═════════════════════════════════════════════════╗
║                   INTERRUPTS:                   ║
╚═════════════════════════════════════════════════╝
*/

/**
 * @brief è la ISR per rilevare la caduta della capsula
 * 
 * @todo capire se devo implementare anche in RISING edge
 *       dell'Interrupt + timeout per capire se 
 */
void capsulaPassataISR()
{
  tmrDebounceServo = millis();
  LogInfoISR("Decapsultor PRG", "Entro nello stato: SERVO_DEBOUNCE_STATE");
  capsulaPassata = true;
}

/**
 *  @brief Queste funzioni gestiscono in modo asincrono le gestioni
 *         dell'Interrupt Service Routine dei pin di finecorsa 
 *         sia "Alto" che "Basso"
 * 
 *  @note Cambia il valore dello stato sia in RISING che in FALLING edge
 * 
 *  @todo Vedere se ha senso implementare un metodo di "AbortStep"
 *        che setta direttamente a 0 i passi che deve fare il motore
 */
void fc_PunzoneAltoISR()
{
  LogInfoISR("Punz_H", "Punz_H ISR");
  punzone_in_max_pos = !digitalRead(PUNZ_MAX_POS_PIN);
  MotPunzone.Halt();
}

void fc_PunzoneBassoISR()
{
  LogInfoISR("Punz_L", "Punz_L ISR");
  punzone_in_min_pos = !digitalRead(PUNZ_MIN_POS_PIN);
  MotPunzone.Halt();
}

void calib_ralla_positionISR()
{
  LogInfoISR("calib_ralla", "calib_ralla ISR");
  /// Se la ralla è calibrata 
  ralla_calibrated = digitalRead(RALLA_CALIB_PIN);
}