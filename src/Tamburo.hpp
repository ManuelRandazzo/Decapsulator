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
 *  DEBOUNCE: TIME:
 */
#define DEB_TIME_CADUTA_CAPSULA_MS 30 // <= 110ms senza considerare overhead
#define DEB_TIME_INTERRUPT 30 // 30ms debounce per confermare il valore di un pin rilevato dall'interrupt



/**
 * 
 *  GHIGLIOTTINA: defines per i dati dell'oggetto del servomotore per la ghigliottina
 * 
 */
#define SERVO_PIN         37   /** @warning Ancora da definire. Pin del servo motore */
#define SERVO_MIN         0    /** Gradi minimi a cui il servo può arrivare */
#define SERVO_MAX         180  /** Gradi massimi a cui il servo può arrivare */
#define SERVO_CLOSED_POS  0    /** @warning Ancora da definire. Angolo di chiusura della paratia*/
#define SERVO_OPEN_POS    90  /** @warning Ancora da definire. Angolo di apertura della paratia*/



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
#define PUNZ_LOW_TORQUE_ROTATIONS  10.0
#define PUNZ_HIGH_TORQUE_ROTATIONS 15.0


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


typedef struct __debounce_pin_management__
{
  unsigned level : 1;      // Stato reale del pin dopo il debounce
  const uint8_t pin;
  const uint32_t debounce_ms; // Tempo di debounce in millisecondi (ms)

  /// flag da usare nell'ISR con accesso atomico di natura
  volatile unsigned flag : 1;

  /// Per debounce function
  unsigned precPinState : 1;
  uint32_t lastTime;
  unsigned debState : 5;
  
} DebPinMgmt_t;

/// Così oppure direttamente un array di queste struct da passare al pvParameter della task (sempre globale)
DebPinMgmt_t cadutaCapsInt = { .pin = PIECE_PASSED_PIN,   .debounce_ms = 30 };
DebPinMgmt_t presenzaCaps  = { .pin = PIECE_PRESENCE_PIN, .debounce_ms = 30 };
DebPinMgmt_t rallaInt      = { .pin = RALLA_CALIB_PIN,    .debounce_ms = 30 };
DebPinMgmt_t punzMinInt    = { .pin = PUNZ_MIN_POS_PIN,   .debounce_ms = 30 };
DebPinMgmt_t punzMaxInt    = { .pin = PUNZ_MAX_POS_PIN,   .debounce_ms = 30 };




/**
 * 
 *    Prototipi di funzioni utili al programma
 *  
 */
void initDebPin(DebPinMgmt_t* inPin, void (*ISR)() = nullptr);
void debounce(DebPinMgmt_t intPin);
void capsulaPassataISR();
void fc_PunzoneAltoISR();
void fc_PunzoneBassoISR();
void calib_ralla_positionISR();



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
  bool FORCE_THE_STARTUP = true;
  bool startCycleFromHMI = false;
  int nCicliRimanenti = 0; // prende il numero dalla SD Card

  Sequence_t sequenza = MACHINE_STARTUP_STATE;  // Gestione della sequenza del movimento del Decapsulator


  /// Motore Tamburo
  MotRalla.Init(RALLA_MOTOR_STEPS, RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN,
                RALLA_TASK_PRIORITY, RALLA_MICROSTEP);
  
  /// Motore Punzone
  MotPunzone.Init(PUNZ_MOTOR_STEPS, PUNZ_DIRECTION_PIN, PUNZ_STEP_PIN, PUNZ_ENABLE_PIN, PUNZ_RESET_PIN, PUNZ_SLEEP_PIN,
                PUNZ_TASK_PRIORITY, PUNZ_MICROSTEP);

  /// Inizializzazione pin e ISR Capsula Caduta
  initDebPin(&cadutaCapsInt, capsulaPassataISR);
  
  /// Inizializzazione pin e Polling Presenza Capsula
  initDebPin(&presenzaCaps/*, nullptr*/);
  
  /// Inizializzazione pin e ISR Punzone Min Position
  initDebPin(&punzMinInt, fc_PunzoneBassoISR);
  
  /// Inizializzazione pin e ISR Punzone Max Position
  initDebPin(&punzMaxInt, fc_PunzoneAltoISR);

  /// Inizializzazione pin e ISR Tamburo Calibration
  initDebPin(&rallaInt, calib_ralla_positionISR);


  MotRalla.attach();
  MotRalla.Start();
  LogDebug("Main Prg", "Move ...");
  MotRalla.moveContinuous(RALLA_HOME_DIR, RALLA_HOME_SPEED);
  
  while(1);

  /**
   *    @loop:
   */
  while(1)
  {
    /**
     * @info: Gestione della sequenza completa del decapsulator
     * 
     * @attention 
     */



    /// @brief 
    switch(sequenza)
    {
      case EMERGENCY_STATE :
        
      break;

      case MACHINE_STARTUP_STATE :
        /// Se il macchinario è chiuso e il pulsante di emergenza è alzato allora è possibile inizializzarlo
        // (dopodichè entrambi verranno gestiti dagli emergency interrupts)
        if(digitalRead(nFAULT_TAMBURO) == HIGH && digitalRead(nFAULT_PUNZONE) == HIGH)
        {
          LogDebug("Main Prg", "Inizializzando il macchinario ...");

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
          LogWarning("Main Prg", "Wait init to finish");
        }
      break;

      case MACHINE_STARTUP_FINISHED_STATE :
            
        /// Una volta Inizializzato tutto...(motori in posizione ecc.)
        /// Passa allo stato di quiete in cui attende il comando di start
        /// DebugOverrideVar<bool>("forceStartup/value", &FORCE_THE_STARTUP); <-- @bug
        if(FORCE_THE_STARTUP || (punzMaxInt.flag == true && punzMinInt.flag == false && rallaInt.flag == true))
        {
          /// Permette la ricalibrazione della ralla
          rallaInt.flag = false;
          //detachInterrupt(rallaInt.pin);
          LogWarning("Decapsulator PRG", "Entro nello stato: QUIETE");
          sequenza = QUIETE_STATE;
        }
      break;

      case QUIETE_STATE :
        /// Se sono presenti le capsule nello scivolo e c'è stato il segnale di start inizia il ciclo
        if(startCycleFromHMI && presenzaCaps.flag && MotRalla.isStepDone() && MotPunzone.isStepDone())
        {
          LogInfo("Decapsulator PRG", "Entro nello stato: REACH_NEXT_STATION_STATE");
          sequenza = REACH_NEXT_STATION_STATE;
        }
      break;

      case REACH_NEXT_STATION_STATE :
        if(MotRalla.isStepDone() && MotPunzone.isStepDone())
        {
          /// Setta la direzione di marcia del tamburo e si muove alla posizione successiva
          MotRalla.moveRel(+90.0 * GEAR_RATIO_RALLA, RALLA_SPEED_MOVEMENT);

          /// Se c'è il pezzo vuol dire che rimangono sempre almeno 3 cicli, altrimenti decrementa
          nCicliRimanenti = presenzaCaps.flag ? 3 : nCicliRimanenti-1;

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

      case PUNCHER_DOWN_LOW_TORQUE_STATE :
        if(MotRalla.isStepDone())
        {
          /// Setta la direzione di marcia del punzone e mette in coda 
          /// due movimenti uno veloce (bassa coppia) e uno lento (alta coppia)
          MotPunzone.moveRel(PUNZ_LOW_TORQUE_ROTATIONS * 360.0, PUNZ_LOW_TORQUE_MOVEMENT); // Fa 10 giri = 20mm lineari ad alta velocità
          LogInfo("Decapsulator PRG", "Entro nello stato: PUNCHER_DOWN_HIGH_TORQUE_STATE");
          sequenza = PUNCHER_DOWN_HIGH_TORQUE_STATE;
        }
      break;

      case PUNCHER_DOWN_HIGH_TORQUE_STATE :
        if(MotRalla.isStepDone())
        {
          MotPunzone.moveRel(PUNZ_HIGH_TORQUE_ROTATIONS * 360.0, PUNZ_HIGH_TORQUE_MOVEMENT);    // Fa 15 giri = 30mm lineari ad alta coppia
          LogInfo("Decapsulator PRG", "Entro nello stato: SERVO_LOADER_OPEN_STATE");
          sequenza = SERVO_LOADER_OPEN_STATE;      
        }
      break;

      case SERVO_LOADER_OPEN_STATE :
        if(MotRalla.isStepDone())
        {
          ServoParatia.write(SERVO_OPEN_POS);
          /// Cambio di stato dovuto dall'Interrupt della Fotocellula conferma capsula nel tamburo
        }
      break;

      case SERVO_LOADER_CLOSE_STATE :
        /// Se il pezzo è passato, i 30ms di debounce sono passati e non si è intasato
        if(!digitalRead(PIECE_PASSED_PIN))
        {
          ServoParatia.write(SERVO_CLOSED_POS);
          LogInfo("Decapsulator PRG", "Entro nello stato: PUNCHER_UP_STATE");
          sequenza = PUNCHER_UP_STATE;
        }
      break;

      case PUNCHER_UP_STATE :
        MotPunzone.moveRel((PUNZ_LOW_TORQUE_ROTATIONS + PUNZ_HIGH_TORQUE_ROTATIONS) * -360.0, PUNZ_LOW_TORQUE_MOVEMENT);
        LogInfo("Decapsulator PRG", "Entro nello stato: PUNCHER_UP_STATE");
      break;
    }

     

    debounce(punzMaxInt);
    debounce(punzMinInt);
    debounce(cadutaCapsInt);
    debounce(presenzaCaps);
    debounce(rallaInt);

    


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
 */
void IRAM_ATTR capsulaPassataISR() { cadutaCapsInt.flag = true; }






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
void IRAM_ATTR fc_PunzoneAltoISR() { punzMaxInt.flag = true; }

void IRAM_ATTR fc_PunzoneBassoISR() { punzMinInt.flag = true; }

void IRAM_ATTR calib_ralla_positionISR() { rallaInt.flag = true; }






/// leggendo direttamente i registri è più veloce (preso spunto da "gpio_ll_get_level()")
/// @attention Necessita però che in pinMode(pin, INPUT_PULLUP) oppure pinMode(pin, INPUT_PULLDOWN)
#ifndef digitalReadFast
  #define digitalReadFast(gpio_pin) ( (gpio_pin < 32) ? ((*(volatile uint32_t*)(GPIO_IN_REG) >> gpio_pin) & 0x1) : ((*(volatile uint32_t*)(GPIO_IN1_REG) >> (gpio_pin - 32)) & 0x1) )
#endif


void debounce(DebPinMgmt_t intPin)
{
  switch(intPin.debState)
  {
    case 0 : /// STATO ATTESA EVENTO
    
      /// Attende che si verifichi l'evento
      if(intPin.flag)
      {
        /// Si salva il tempo di start da quando avviene il rilevamento dell'interrupt
        intPin.lastTime = millis();    

        /// Restituisce lo stato di confronto
        intPin.precPinState = digitalReadFast(intPin.pin);

        /// Passa allo stato di attesa della conferma
        intPin.debState++;
      }

    break;

    case 1 : /// STATO ATTESA CONFERMA

      /// Si salva il tempo attuale
      const uint32_t tmpTime = millis();

      if(tmpTime - intPin.lastTime >= intPin.debounce_ms)
      {
        /// Flag reset
        intPin.flag = 0;

        /// Debounce state reset
        intPin.debState = 0;

        /// Restituisce lo stato reale del pin
        if(digitalReadFast(intPin.pin) == intPin.precPinState)
        {
          intPin.level = intPin.precPinState;
        }
      }

    break;
  }
}


/**
 * @brief Inizializza un determinato input pin per avere un debounce
 * 
 * @param inPin viene passata la struttura che contiene i dati relativi
 *        al pin e al su debounce
 * 
 * @param ISR 
 */
void initDebPin(DebPinMgmt_t* inPin, void (*ISR)())
{
  pinMode(inPin->pin, INPUT_PULLDOWN);
  inPin->level = digitalReadFast(inPin->pin);
  if(ISR != nullptr)
    attachInterrupt(digitalPinToInterrupt(inPin->pin), ISR, CHANGE);
}

