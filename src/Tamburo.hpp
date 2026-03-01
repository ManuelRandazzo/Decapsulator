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
/// Include la libreria per la gestione dei pin e del debounce
#include "DebouncePinHandler.hpp"




/**
 * 
 *  @_not_defined_things: defines per i dati dell'oggetto del MotionControl del tamburo
 * 
 */
#define PIECE_PRESENCE_PIN     12 /** @attention  Ancora da definire */
#define PIECE_PASSED_PIN       11 /** @attention  Ancora da definire */
#define nFAULT_TAMBURO          7 /** @attention  Ancora da definire */
#define nFAULT_PUNZONE         39 /** @attention  Ancora da definire */
#define AUTOKILL_DETECT_PIN   150 /** @attention  Ancora da definire */
#define AUTOKILL_SHUTDOWN_PIN 150 /** @attention  Ancora da definire */


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
#define SERVO_PIN         37   /** @attention Ancora da definire. Pin del servo motore */
#define SERVO_MIN         0    /** Gradi minimi a cui il servo può arrivare */
#define SERVO_MAX         180  /** Gradi massimi a cui il servo può arrivare */
#define SERVO_CLOSED_POS  0    /** @attention Ancora da definire. Angolo di chiusura della paratia*/
#define SERVO_OPEN_POS    90  /** @attention Ancora da definire. Angolo di apertura della paratia*/



/**
 * 
 *  TAMBURO: defines per i dati dell'oggetto del MotionControl del tamburo
 * 
 */
#define RALLA_MOTOR_STEPS     200
#define RALLA_TASK_PRIORITY   1           /** @attention: è importante che sia <= della priorità della task */
#define RALLA_SPEED_MOVEMENT  2000        // Velocità di esecuzione relativo al tamburo in gradi al secondo [°/s]
#define GEAR_RATIO_RALLA      3           // Imposta un gear ratio 1/3 per la ralla

/// Driver DRV8825 pins
#define RALLA_DIRECTION_PIN   4             /** @attention  Ancora da definire*/
#define RALLA_STEP_PIN        5             /** @attention  Ancora da definire*/
#define RALLA_ENABLE_PIN      6             /** @attention  Ancora da definire*/
#define RALLA_RESET_PIN       255           /** @attention  Ancora da definire*/
#define RALLA_SLEEP_PIN       255           /** @attention  Ancora da definire*/
#define RALLA_FAULT_PIN       7             /** @attention  Ancora da definire*/


/// Parametri HOMING
#define RALLA_CALIB_PIN       14            /** @attention  Ancora da definire*/
#define RALLA_TRIGGER_MODE    FALLING
#define RALLA_HOME_SPEED      30            /** Gradi al secondo*/
#define RALLA_MICROSTEP       FULL_STEP //STEP_1_TO_16
#define RALLA_CAM_SIGNAL      ACTIVE_LOW
#define RALLA_HOME_DIR        DIR_NEGATIVE
#define RALLA_POST_HOME_POS   NULL          /** @attention  Ancora da definire*/
#define RALLA_TOCCHI_SENSORE  2             /** @attention  Ancora da definire*/


/**
 * 
 *  PUNZONE: defines per i dati dell'oggetto del MotionControl
 * 
 */

#define PUNZ_MOTOR_STEPS           200
#define PUNZ_TASK_PRIORITY         1             /** @attention: è importante che sia <= della priorità della task */
#define PUNZ_LOW_TORQUE_MOVEMENT   2800.0        // Velocità di esecuzione relativo al punzone in gradi al secondo [°/s]
#define PUNZ_HIGH_TORQUE_MOVEMENT  180.0
#define PUNZ_LOW_TORQUE_ROTATIONS  10.0
#define PUNZ_HIGH_TORQUE_ROTATIONS 15.0


/// Driver DRV8825 pins
#define PUNZ_DIRECTION_PIN      4             /** @attention  Ancora da definire*/
#define PUNZ_STEP_PIN           5             /** @attention  Ancora da definire*/
#define PUNZ_ENABLE_PIN         15            /** @attention  Ancora da definire*/
#define PUNZ_RESET_PIN          7             /** @attention  Ancora da definire*/
#define PUNZ_SLEEP_PIN          6             /** @attention  Ancora da definire*/
#define PUNZ_FAULT_PIN          16            /** @attention  Ancora da definire*/

/// Parametri HOMING Punzone
#define PUNZ_MAX_POS_PIN 13 /** @attention  Ancora da definire*/
#define PUNZ_MIN_POS_PIN 21 /** @attention  Ancora da definire*/

#define PUNZ_CALIB_PIN          PUNZ_MAX_POS_PIN
#define PUNZ_TRIGGER_MODE       FALLING
#define PUNZ_HOME_SPEED         30            /** Gradi al secondo*/
#define PUNZ_MICROSTEP          FULL_STEP //STEP_1_TO_16
#define PUNZ_CAM_SIGNAL         ACTIVE_LOW
#define PUNZ_HOME_DIR           DIR_POSITIVE
#define PUNZ_POST_HOME_POS      NULL          /** @attention  Ancora da definire*/
#define PUNZ_TOCCHI_SENSORE     2             /** @attention  Ancora da definire*/



 
/**
 * 
 *        GLOBAL VARIABLE LIST :
 * 
 */

TaskHandle_t MainPrgHandler = nullptr;

typedef enum __sequence__ : uint8_t
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


/// Crea gli oggetti
/// Crea l'oggetto della classe TaskTypeDef, ovvero la task di gestione del decapsulator
TaskTypeDef DecapsulatorHandleTask;
MOTION MotRalla;
MOTION MotPunzone;
Servo ServoParatia;

/// Mutex e spinlock
SemaphoreHandle_t _DecapsulatorMutex = nullptr;
portMUX_TYPE _DecapsulatorSpinlock = portMUX_INITIALIZER_UNLOCKED;



volatile QueueHandle_t ptrAutokillSharedVars;







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
  bool FORCE_THE_STARTUP = false;//true;
  bool startCycleFromHMI = false;
  int nCicliRimanenti = 0; // prende il numero dalla SD Card

  Sequence_t sequenza = MACHINE_STARTUP_STATE;  // Gestione della sequenza del movimento del Decapsulator


  LogDebug("Main Prg", "Prima dell'init RALLA");


  /// Motore Tamburo
  drv_err_t drvErr = MotRalla.Init(RALLA_MOTOR_STEPS, RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN,
                RALLA_TASK_PRIORITY, RALLA_MICROSTEP);
  while(drvErr != DRV_OK)
  {
    LogError("Main Prg", "Errore nell'inizializzazione del MOTION della Ralla, codice errore = %s (%d)", drv_err_to_name(drvErr), drvErr);
    while(1);
  }
  LogDebug("Main Prg", "Prima dell'init PUNZONE");
  
  /// Motore Punzone
  drvErr = MotPunzone.Init(PUNZ_MOTOR_STEPS, PUNZ_DIRECTION_PIN, PUNZ_STEP_PIN, PUNZ_ENABLE_PIN, PUNZ_RESET_PIN, PUNZ_SLEEP_PIN,
                  PUNZ_TASK_PRIORITY, PUNZ_MICROSTEP);
  while(drvErr != DRV_OK)
  {
    LogError("Main Prg", "Errore nell'inizializzazione del MOTION del Punzone, codice errore = %s (%d)", drv_err_to_name(drvErr), drvErr);
    while(1);
  }

  /// Inizializzazione Servo e relativi suoi timer[0-4] dell'hardware ledc
  for(uint8_t i = 0; i < 4; i++)
    ESP32PWM::allocateTimer(i);
	ServoParatia.setPeriodHertz(50);    // standard 50 hz servo
  
  /// Inizializzazione dei pin di debounce
  DebPinHandler autoKill     (INTR, AUTOKILL_DETECT_PIN, "Autokill Detection Pin", 10/* ms */, FALLING, INPUT);
  DebPinHandler cadutaCaps   (INTR, PIECE_PASSED_PIN   , "Caduta Capsule Pin"    , 30/* ms */, RISING , INPUT);
  DebPinHandler presenzaCaps (POLL, PIECE_PRESENCE_PIN , "Presenza Capsule Pin"  , 30/* ms */, FALLING, INPUT);
  DebPinHandler ralla        (INTR, RALLA_CALIB_PIN    , "Ralla Calibration Pin" , 30/* ms */, RALLA_TRIGGER_MODE , INPUT);
  DebPinHandler punzMin      (INTR, PUNZ_MIN_POS_PIN   , "Punzone FC Minimo"     , 30/* ms */, PUNZ_TRIGGER_MODE , INPUT);
  DebPinHandler punzMax      (INTR, PUNZ_MAX_POS_PIN   , "Punzone FC Massimo"    , 30/* ms */, PUNZ_TRIGGER_MODE , INPUT);
     

  /**
   *    @loop:
   */
  while(1)
  {
    /**
     * @info: Gestione della sequenza completa del decapsulator
     * 
     * @attention
     * 
     * @todo
     */


    /// Update dei pin di debounce
    autoKill.intrUpdate();
    cadutaCaps.intrUpdate();
    presenzaCaps.pollUpdate();
    ralla.intrUpdate();     
    punzMin.intrUpdate();
    punzMax.intrUpdate();



    /// @brief 
    switch(sequenza)
    {
      case EMERGENCY_STATE :
        
      break;

      /// Se il macchinario è chiuso (se non lo è non si accende l'ESP32) è possibile inizializzarlo
      case MACHINE_STARTUP_STATE :

        LogDebug("Main Prg", "Inizializzando il macchinario ...");

        /// Fa tutti gli attach dei motori e li prepara ad essere comandati
        MotRalla.attach();
        MotRalla.Start();
        /// Esegue l'homing del tamburo
        /*MotRalla.home(RALLA_CALIB_PIN, INPUT, RALLA_TRIGGER_MODE, RALLA_HOME_SPEED, RALLA_HOME_DIR,
                      25.0 * 360.0, RALLA_TOCCHI_SENSORE, RALLA_CAM_SIGNAL);*/
                      
        /// Se non è ancora in posizione allora raggiunge una posizione nota (Posizione Home della ralla)
        if(ralla.rawRead() != RALLA_CAM_SIGNAL)
          MotRalla.moveContinuous(RALLA_HOME_DIR, RALLA_HOME_SPEED);

        MotPunzone.attach();
        MotPunzone.Start();
        /// Se non è ancora in posizione arretrata allora va indietro (Posizione Home del punzone)
        if(punzMax.rawRead() == PUNZ_CAM_SIGNAL)
          MotPunzone.moveContinuous(DIR_NEGATIVE, PUNZ_HOME_SPEED);

        ServoParatia.attach(SERVO_PIN, SERVO_MIN, SERVO_MAX);
        ServoParatia.write(SERVO_CLOSED_POS); // Chiude la paratia mossa dal servomotore

        sequenza = MACHINE_STARTUP_FINISHED_STATE;
        LogWarning("Decapsulator PRG", "Wait init to finish");
    
      break;

      /// Una volta Inizializzato tutto...(motori in posizione ecc.)
      /// Passa allo stato di quiete in cui attende il comando di start
      case MACHINE_STARTUP_FINISHED_STATE :
      {            
        /// DebugOverrideVar<bool>("forceStartup/value", &FORCE_THE_STARTUP); <-- @bug
        if(FORCE_THE_STARTUP || (punzMax.event() == true && punzMin.event() == false && ralla.event() == true))
        {
          ralla.detach();
          LogWarning("Decapsulator PRG", "Entro nello stato: QUIETE");
          sequenza = QUIETE_STATE;
        }
        break;
      }

      case QUIETE_STATE :
      {
        /// Se sono presenti le capsule nello scivolo e c'è stato il segnale di start inizia il ciclo
        if(startCycleFromHMI && presenzaCaps.event() && MotRalla.isStepDone() && MotPunzone.isStepDone())
        {
          LogInfo("Decapsulator PRG", "Entro nello stato: REACH_NEXT_STATION_STATE");
          sequenza = REACH_NEXT_STATION_STATE;
        }
        break;
      }

      case REACH_NEXT_STATION_STATE :
      {

        if(MotRalla.isStepDone() && MotPunzone.isStepDone())
        {
          /// Setta la direzione di marcia del tamburo e si muove alla posizione successiva
          MotRalla.moveRel(+90.0 * GEAR_RATIO_RALLA, RALLA_SPEED_MOVEMENT);

          /// Se c'è il pezzo vuol dire che rimangono sempre almeno 3 cicli, altrimenti decrementa
          nCicliRimanenti = presenzaCaps.event() ? 3 : nCicliRimanenti-1;

          if(nCicliRimanenti == 0)
          {
            LogInfo("Decapsulator PRG", "Entro nello stato: QUITE_STATE");
            sequenza = QUIETE_STATE;
          }
          else
          {
            LogInfo("Decapsulator PRG", "Entro nello stato: PUNCHER_DOWN_LOW_TORQUE_STATE");
            sequenza = PUNCHER_DOWN_LOW_TORQUE_STATE;
          }
        }
        break;
      }

      case PUNCHER_DOWN_LOW_TORQUE_STATE :
      {
        if(MotRalla.isStepDone())
        {
          /// Setta la direzione di marcia del punzone e mette in coda 
          /// due movimenti uno veloce (bassa coppia) e uno lento (alta coppia)
          MotPunzone.moveRel(PUNZ_LOW_TORQUE_ROTATIONS * 360.0, PUNZ_LOW_TORQUE_MOVEMENT); // Fa 10 giri = 20mm lineari ad alta velocità
          LogInfo("Decapsulator PRG", "Entro nello stato: PUNCHER_DOWN_HIGH_TORQUE_STATE");
          sequenza = PUNCHER_DOWN_HIGH_TORQUE_STATE;
        }
        break;
      }

      case PUNCHER_DOWN_HIGH_TORQUE_STATE :
      {
        if(MotRalla.isStepDone())
        {
          MotPunzone.moveRel(PUNZ_HIGH_TORQUE_ROTATIONS * 360.0, PUNZ_HIGH_TORQUE_MOVEMENT);    // Fa 15 giri = 30mm lineari ad alta coppia
          LogInfo("Decapsulator PRG", "Entro nello stato: SERVO_LOADER_OPEN_STATE");
          sequenza = SERVO_LOADER_OPEN_STATE;      
        }
        break;
      }

      case SERVO_LOADER_OPEN_STATE :
      {
        if(MotRalla.isStepDone())
        {
          ServoParatia.write(SERVO_OPEN_POS);
          /// Cambio di stato dovuto dall'Interrupt della Fotocellula conferma capsula nel tamburo
        }
        break;
      }

      case SERVO_LOADER_CLOSE_STATE :
      {
        /// Se il pezzo è passato, i 30ms di debounce sono passati e non si è intasato
        if(!digitalRead(PIECE_PASSED_PIN))
        {
          ServoParatia.write(SERVO_CLOSED_POS);
          LogInfo("Decapsulator PRG", "Entro nello stato: PUNCHER_UP_STATE");
          sequenza = PUNCHER_UP_STATE;
        }
        break;
      }

      case PUNCHER_UP_STATE :
      {
        MotPunzone.moveRel((PUNZ_LOW_TORQUE_ROTATIONS + PUNZ_HIGH_TORQUE_ROTATIONS) * -360.0, PUNZ_LOW_TORQUE_MOVEMENT);
        LogInfo("Decapsulator PRG", "Entro nello stato: PUNCHER_UP_STATE");
        break;
      }
    }

     
/*
    debounce(&punzMaxInt);
    debounce(&punzMinInt);
    debounce(&cadutaCapsInt);
    debounce(&presenzaCaps);
    debounce(&rallaInt);
*/
    

    xTaskDelayUntil(&getLastTick, MainPrg_delay);
  }
}