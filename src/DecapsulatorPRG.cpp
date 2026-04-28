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

#include "DecapsulatorPRG.hpp"





#pragma region (GLOBAL VARIABLE LIST)


/// @brief Handler della coda che riceve i comandi che arrivano dall'HMI
QueueHandle_t QueueHandlerHMI_CMD = nullptr;

/// @brief Handler della coda che invia gli eventi (errori, avvisi, ecc.) all'HMI
QueueHandle_t QueueHandlerHMI_EVT = nullptr; 

/// @brief Handler della task che gestisce il programma principale del decapsulator
TaskHandle_t MainPrgHandler = nullptr;

typedef enum __sequence__ : uint8_t
{
  EMERGENCY_STATE,
  CONTAINER_FULL,
  TIMEOUT_STATE,
  MACHINE_STARTUP_STATE,
  PUNZONE_STARTUP_STATE,
  TAMBURO_STARTUP_STATE,
  QUIETE_STATE,
  SERVO_LOADER_OPEN_STATE,
  SERVO_LOADER_CLOSE_STATE,
  REACH_NEXT_STATION_STATE,
  PUNCHER_DOWN_FAST_STATE,
  PUNCHER_DOWN_SLOW_STATE,
  PUNCHER_UP_FAST_STATE,
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

/// @brief false = da il comando, true = esegue il comando
bool cmd_exec = false; 

#pragma endregion (GLOBAL VARIABLE LIST)



#pragma region (FUNCTION_PROTOTIPES)

BaseType_t checkUpdateHMI(CommandQueueHMI_t* receivingQueue); /// Direction: Frontend --> Backend
BaseType_t checkUpdateHMI(EventQueueHMI_t* receivingQueue);   /// Direction: Backend --> Frontend
BaseType_t sendUpdateHMI(CommandQueueHMI_t* sendingQueue, bool* sendChanges); /// Direction: Frontend --> Backend
BaseType_t sendUpdateHMI(EventQueueHMI_t* sendingQueue, bool* sendChanges);   /// Direction: Backend --> Frontend
const char* state_name_to_string(Sequence_t seq_switch);

#pragma endregion (FUNCTION_PROTOTIPES)



#pragma region (DECAPSULATOR MAIN PROGRAM)


/**
 *  @brief PROGRAMMA PRINCIPALE
 */
void prgDecapsulatorTask(void *pvParameters)
{
  /**
   *    @setup:
   */
  TickType_t getLastTick = xTaskGetTickCount();

  ///ATTENZIONE: Programma con variabili a caso ancora da definire
  ///            e da rendere THREAD SAFE tramite notifiche e/o code
  bool FORCE_THE_STARTUP = false;//true;
  
  /// @brief Handler della coda che riceve i comandi che arrivano dall'HMI
  QueueHandlerHMI_CMD = xQueueCreate(HMI_CMD_QUEUE_LEN, sizeof(CommandQueueHMI_t));
  CommandQueueHMI_t FromHMI = defaultCommandQueueHMI;

  /// @brief Handler della coda che invia gli eventi (errori, avvisi, ecc.) all'HMI
  QueueHandlerHMI_EVT = xQueueCreate(HMI_EVT_QUEUE_LEN, sizeof(EventQueueHMI_t));
  EventQueueHMI_t ToHMI = defaultEventQueueHMI;
  bool sendChangesToHMI = false;

  uint8_t cntContainerFull = 0;
  int nCicliRimanenti = 0; // prende il numero dalla SD Card

  Sequence_t sequenza = MACHINE_STARTUP_STATE;  // Gestione della sequenza del movimento del Decapsulator

  /// Motore Tamburo
  drv_err_t drvErr;
  drvErr = MotRalla.Init(RALLA_MOTOR_STEPS, RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN,
                         RALLA_TASK_PRIORITY, RALLA_MICROSTEP);
                  
  MotRalla.detach();

  MotRalla.setHardLimits(RALLA_CALIB_PIN, 255, RALLA_CALIB_INTR_OR_POLL, 30, RALLA_INPUT_PULL, RALLA_CAM_SIGNAL);

  R_TRIG MotRalla_IsStepDone;

  if(drvErr != DRV_OK)
  {
    #ifdef LOG_ACTIVE_MAIN_PRG
      LogError("Main Prg", "Errore nell'inizializzazione del MOTION della Ralla, codice errore = %s (%d)", drv_err_to_name(drvErr), drvErr);
    #endif
    while(1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
  }

  /// Motore Punzone
  drvErr = MotPunzone.Init(PUNZ_MOTOR_STEPS, PUNZ_DIRECTION_PIN, PUNZ_STEP_PIN, PUNZ_ENABLE_PIN, PUNZ_RESET_PIN, PUNZ_SLEEP_PIN,
                           PUNZ_TASK_PRIORITY, PUNZ_MICROSTEP);
  MotPunzone.detach();
  
  R_TRIG MotPunzone_IsStepDone;
                
  if(drvErr != DRV_OK)
  {
    #ifdef LOG_ACTIVE_MAIN_PRG
      LogError("Main Prg", "Errore nell'inizializzazione del MOTION del Punzone, codice errore = %s (%d)", drv_err_to_name(drvErr), drvErr);
    #endif
    while(1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
  }

  MotPunzone.setHardLimits(PUNZ_MAX_POS_PIN, PUNZ_MIN_POS_PIN, PUNZ_HARD_LIM_INTR_OR_POLL, 30, PUNZ_INPUT_PULL, PUNZ_CAM_SIGNAL);


  /// Inizializzazione Servo e relativi suoi timer[0-3] dell'hardware ledc
  for(uint8_t i = 0; i < 4; i++)
    ESP32PWM::allocateTimer(i);
	ServoParatia.setPeriodHertz(50);    // standard 50 hz servo
  
  /// Inizializzazione dei pin di debounce
  DebPinHandler autoKill     (INTR, AUTOKILL_DETECT_PIN, "Autokill Detection Pin", 10/* ms */, FALLING, INPUT);
  DebPinHandler cadutaCaps   (INTR, PIECE_PASSED_PIN   , "Caduta Capsule Pin"    , 10/* ms */, FALLING , INPUT);
  DebPinHandler presenzaCaps (INTR, PIECE_PRESENCE_PIN , "Presenza Capsule Pin"  , 30/* ms */, FALLING, INPUT);
  bool doAnotherCycle = false;

  if(cadutaCaps.event())
  {
    /// ATTENZIONE: Loggare Qualcosa nel display
    #ifdef LOG_ACTIVE_MAIN_PRG
      LogError("Main Prg", "C'è una capsula incastrata nello scivolo, consultare il Manuale di uso e manutenzione");
    #endif
    
    int8_t rawRead;
    do
    {
      rawRead = cadutaCaps.rawRead();
      vTaskDelay(pdMS_TO_TICKS(1000));
      #ifdef LOG_ACTIVE_MAIN_PRG
        LogError("Main Prg", "Valore lettura = %s", rawRead ? "HIGH" : "LOW");
      #endif
    } while(rawRead == cadutaCaps.getLevelTrig());
  }

  /// TIMEOUTS: TIMERS:
  uint32_t tmoPunzHome = 0;
  uint32_t tmoRallaHome = 0;
  uint32_t tmoCaduta = 0;

  #ifdef LOG_ACTIVE_MAIN_PRG
    LogInfo("setup main prg", "creata la tasks - Free Stack Space: %d\nSwitch Initial : %s", uxTaskGetStackHighWaterMark(NULL), state_name_to_string(sequenza));
  #endif
  
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
     * @todo Farlo Funzionare :)
     */

    /// acquisisce il tempo attuale di millis()
    const uint32_t MILLIS = millis();

    /// Update dei pin d'evento
    autoKill.intrUpdate();
    cadutaCaps.intrUpdate();
    presenzaCaps.intrUpdate();

    MotRalla_IsStepDone.CLK(MotRalla.isStepDone());
    MotPunzone_IsStepDone.CLK(MotPunzone.isStepDone());

    /// Se arriva una capsula allora bisogna può essere eseguito un altro ciclo
    if(presenzaCaps.event())
      doAnotherCycle = true;

    checkUpdateHMI(&FromHMI);

    Serial.printf("Sequenza : %s\npresenzaCaps.event() : %d\ncadutaCaps.event() : %d", state_name_to_string(sequenza), presenzaCaps.event(), cadutaCaps.event());
    
    switch(sequenza)
    {
      case EMERGENCY_STATE :
      {        
        /// @todo
        MainPrgStopAllMotors();
        LogError("EMERGENCY", "Si è entrati in uno stato di EMERGENZA");

        break;
      }

      case TIMEOUT_STATE :
      {
        if(!cmd_exec)
        {
          if(ToHMI.xErrorInitPunz || ToHMI.xErrorInitRalla)
            MainPrgStopAllMotors();

          cmd_exec = true;
        }
        /// @todo
        break;
      }

      case CONTAINER_FULL :
      {
        if(FromHMI.restartAfterContainerEmptied == true)
        {
          FromHMI.restartAfterContainerEmptied = false;
          cntContainerFull = 0;
        }

        break;
      }

      /// Inizializza il macchinario
      case MACHINE_STARTUP_STATE :
      {
        /// Fa tutti gli attach dei motori e li prepara ad essere comandati
        ServoParatia.attach(SERVO_PIN);
        ServoParatia.write(SERVO_CLOSED_POS); // Chiude la paratia mossa dal servomotore

        cmd_exec = false;

        if(FORCE_THE_STARTUP == false)
          sequenza = PUNZONE_STARTUP_STATE;
        else
          sequenza = QUIETE_STATE;
    
        break;
      }

      case PUNZONE_STARTUP_STATE :
      {
        if(!cmd_exec) // Da il comando
        {
          MotPunzone.attach();
          MotPunzone.Start();
          MotPunzone.home(PUNZ_HOME_SPEED, PUNZ_HOME_DIR, PUNZ_POST_HOME_POS);
          tmoPunzHome = MILLIS;
          cmd_exec = true;
        }
        else
        {
          if(1/*MILLIS - tmoPunzHome < TIMEOUT_PUNZ_HOME_MS*/)
          {
            if(MotPunzone.isHomeDone() == true) // Aspetta la fine del comando
            {
              // Deve assicurarsi di portare in posizione il punzone prima di poter muovere la ralla
              sequenza = TAMBURO_STARTUP_STATE;
              cmd_exec = false;
            }
          }
          else
          {
            cmd_exec = false;
            sequenza = TIMEOUT_STATE;
          }
        }

        break;
      }

      case TAMBURO_STARTUP_STATE :
      {
        if(!cmd_exec) // Da il comando
        {
          MotRalla.attach();
          MotRalla.Start();
          MotRalla.reattachHardLimits();
          MotRalla.home(RALLA_HOME_SPEED, RALLA_HOME_DIR, RALLA_POST_HOME_POS);
          tmoRallaHome = MILLIS;
          cmd_exec = true;
        }
        else
        {
          if(1/*MILLIS - tmoRallaHome < TIMEOUT_RALLA_HOME_MS*/)
          {
            if(MotRalla.isHomeDone() == true) // Aspetta la fine del comando
            {
              /// Rimuove il sensore di calibrazione
              MotRalla.removeHardLimits();
              sequenza = QUIETE_STATE;
              cmd_exec = false;
            }
          }
          else
          {
            sequenza = TIMEOUT_STATE;
            cmd_exec = false;
          }
        }

        break;
      }

      case QUIETE_STATE :
      {
        /// Se sono presenti le capsule nello scivolo e c'è stato il segnale di start inizia il ciclo
        if(FromHMI.StartMachine)
        {
          FromHMI.StartMachine = false;
          
          if(doAnotherCycle == true)
            sequenza = SERVO_LOADER_OPEN_STATE;
        }

        break;
      }
      
      case SERVO_LOADER_OPEN_STATE :
      {
        if(doAnotherCycle == true)
        {
          if(cntContainerFull <= MAX_CAPSULE_CONTAINER) // Impedisce che scendano le capsule quando il contatore segnala  
          {
            if(!cmd_exec) // Da il comando
            {
              ServoParatia.write(SERVO_OPEN_POS);
              tmoCaduta = MILLIS;
              cmd_exec = true;
            }
            else
            {
             if(1/*MILLIS - tmoCaduta <= TIMEOUT_CADUTA_CAPS_MS*/)
              {
                /// Cambio di stato dovuto dall'Interrupt della Fotocellula conferma capsula nel tamburo
                if(cadutaCaps.event() == true)
                {
                  //cntContainerFull++;
                  doAnotherCycle = false;
                  sequenza = SERVO_LOADER_CLOSE_STATE;
                  cmd_exec = false;
                }
              }
              else
              {
                ToHMI.xErrorCapsIncastrata = true;
                sequenza = TIMEOUT_STATE;
                cmd_exec = false;
              }
            }
          }
          else
          {
            cmd_exec = false;
            sequenza = CONTAINER_FULL;
          }
        }
        else
          sequenza = QUIETE_STATE;
        
        break;
      }

      case SERVO_LOADER_CLOSE_STATE :
      {
        /// Se il pezzo è passato, i tot ms di debounce sono passati e non si è intasato
        ServoParatia.write(SERVO_CLOSED_POS);
        sequenza = REACH_NEXT_STATION_STATE;
        
        break;
      }

      case REACH_NEXT_STATION_STATE :
      {
        if(!cmd_exec) // Dà il comando
        {
          /// Setta la direzione di marcia del tamburo e si muove alla posizione successiva
          MotRalla.moveRel(+90.0 * GEAR_RATIO_RALLA, RALLA_SPEED);
          cmd_exec = true;
        }
        else if(MotRalla_IsStepDone.Q() == true) // Aspetta la fine del comando
        {
          sequenza = PUNCHER_DOWN_FAST_STATE;
          cmd_exec = false;
        }
        
        break;
      }

      case PUNCHER_DOWN_FAST_STATE :
      {
        if(!cmd_exec) // Dà il comando
        {
          /// Setta la direzione di marcia del punzone e mette in coda 
          /// due movimenti uno veloce (bassa coppia) e uno lento (alta coppia)
          MotPunzone.moveRel(PUNZ_FAST_ROTATIONS * -360.0, PUNZ_FAST_SPEED); // Fa 10 giri = 20mm lineari ad alta velocità
          cmd_exec = true;
        }
        else if(MotPunzone_IsStepDone.Q() == true) // Aspetta la fine del comando
        {
          sequenza = PUNCHER_DOWN_SLOW_STATE;
          cmd_exec = false;
        }

        break;
      }

      case PUNCHER_DOWN_SLOW_STATE :
      {
        if(!cmd_exec) // Dà il comando
        {
          /// Fa 15 giri = 30mm lineari ad alta coppia
          MotPunzone.moveRel(PUNZ_SLOW_ROTATIONS * -360.0, PUNZ_SLOW_SPEED);    
          cmd_exec = true;
        }
        else if(MotPunzone_IsStepDone.Q() == true) // Aspetta la fine del comando
        {
          sequenza = PUNCHER_UP_FAST_STATE;     
          cmd_exec = false;
        }

        break;
      }

      case PUNCHER_UP_FAST_STATE :
      {
        if(!cmd_exec) // Dà il comando
        {
          /// Torna nella posizione 
          MotPunzone.moveRel(PUNZ_ROTATIONS_TOT * +360.0, PUNZ_FAST_SPEED);
          vTaskDelay(100);
          cmd_exec = true;
        }
        else if(MotPunzone_IsStepDone.Q() == true) // Aspetta la fine del comando
        {
          sequenza = SERVO_LOADER_OPEN_STATE; // Ricomincia il ciclo
          cmd_exec = false;
        }

        break;
      }
    }

    /// Aggiorna in caso vengano richiesti dei cambiamenti da segnalare all'HMI
    sendUpdateHMI(&ToHMI, &sendChangesToHMI);

    //LogDebug("debug", "TaskTime : %d\n", millis() - MILLIS);

    xTaskDelayUntil(&getLastTick, MainPrg_delay);
  }

  /// Elimina la task qualora uscisse dal while(1)
  vTaskDelete(NULL);
}


#pragma endregion (DECAPSULATOR MAIN PROGRAM)




/**
 *  @brief funzione chiamata in caso di emergenza, di servizio o di timeout
 * 
 *  @note Dopo aver chiamato questa funzione va rifatto l'homing
 * 
 *  @details Fa l'ABORT dei commandi attuali dei motori, FERMA i motori e fa DETACH dei pin RILASCIANDO la coppia
 */
void MainPrgStopAllMotors()
{  
  /// Abort dei comandi attuali ai motori
  MotPunzone.abortCurrentCommand();
  MotRalla.abortCurrentCommand();

  /// Ferma gli stepper e rilasciano la coppia
  MotPunzone.Stop(RELEASE);
  MotRalla.Stop(RELEASE);

  /// Scollega i pin dei motori
  MotPunzone.detach();
  MotRalla.detach();
  ServoParatia.detach();
}


/**
 * @brief Cerca e aggiorna in caso trovi dei cambiamenti dall'HMI
 * 
 * @dir Direction: Frontend --> Backend
 * 
 * @param receivingQueue Struct che contiene i dati che vengono inviati dall'HMI
 * 
 * @internal Usa "QueueHandlerHMI_CMD" come handler per la coda
 * 
 * @return pdTRUE se è stato ricevuto un comando con successo
 * 
 * @return pdFALSE se è fallita
 */
BaseType_t checkUpdateHMI(CommandQueueHMI_t* receivingQueue)
{
  #ifdef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
    if(receivingQueue == NULL)
    {
      LogError("checkUpdateHMI HMI-->Backend", "Il parametro \"receivingQueue\" is a null pointer");
      return;
    }
  #endif

  return xQueueReceive(QueueHandlerHMI_CMD, &receivingQueue, TICKS_TO_WAIT_QUEUE_RECEIVE_HMI);
}


/**
 * @brief Cerca e aggiorna in caso trovi dei cambiamenti dall'HMI
 * 
 * @dir Direction: Backend --> Frontend
 * 
 * @param receivingQueue Struct che contiene i dati che vengono inviati dall'HMI
 * 
 * @internal Usa "QueueHandlerHMI_EVT" come handler per la coda
 * 
 * @return BaseType_t Queue Error
 */
BaseType_t checkUpdateHMI(EventQueueHMI_t* receivingQueue)
{
  #ifdef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
    if(receivingQueue == NULL)
    {
      LogError("checkUpdateHMI Backend-->HMI", "Il parametro \"receivingQueue\" is a null pointer");
      return;
    }
  #endif

  return xQueueReceive(QueueHandlerHMI_EVT, receivingQueue, TICKS_TO_WAIT_QUEUE_RECEIVE_HMI);
}


/**
 * @brief Aggiorna in caso vengano richiesti dei cambiamenti da segnalare al Backend
 * 
 * @dir Direction: Frontend --> Backend
 * 
 * @param sendingQueue Struct che contiene i dati che vengono inviati al Backend
 * 
 * @param sendChanges Flag che determina se deve essere inviata la queue.
 *                    Il flag viene abbassato se è stato inviato il messaggio.
 * 
 * @internal Usa "QueueHandlerHMI_CMD" come handler per la coda
 * 
 * @return BaseType_t Queue Error
 */
BaseType_t sendUpdateHMI(CommandQueueHMI_t* sendingQueue, bool* sendChanges)
{
  #ifdef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
    if(sendingQueue == NULL)
    {
      LogError("sendUpdateHMI HMI-->Backend", "Il parametro \"sendingQueue\" is a null pointer");
      return;
    }

    if(sendChanges == NULL)
    {
      LogError("sendUpdateHMI HMI-->Backend", "Il parametro \"sendChanges\" is a null pointer");
      return;
    }
  #endif


  BaseType_t err = pdTRUE;

  if(*sendChanges == true)
  {
    err = xQueueSend(QueueHandlerHMI_CMD, sendingQueue, TICKS_TO_WAIT_QUEUE_SEND_HMI);
    if(err == pdTRUE)
      *sendChanges = false; // abbassa il flag se l'invio è riuscito
  }

  return err;
}


/**
 * @brief Aggiorna in caso vengano richiesti dei cambiamenti da segnalare all'HMI
 * 
 * @dir Direction: Backend --> Frontend
 * 
 * @param sendingQueue Struct che contiene i dati che vengono inviati all'HMI
 * 
 * @param sendChanges Flag che determina se deve essere inviata la queue.
 *                    Il flag viene abbassato se è stato inviato il messaggio.
 * 
 * @internal Usa "QueueHandlerHMI_EVT" come handler per la coda
 * 
 * @return BaseType_t Queue Error
 */
BaseType_t sendUpdateHMI(EventQueueHMI_t* sendingQueue, bool* sendChanges)
{
  #ifdef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
    if(sendingQueue == NULL)
    {
      LogError("sendUpdateHMI Backend-->HMI", "Il parametro \"sendingQueue\" is a null pointer");
      return;
    }
    
    if(sendChanges == NULL)
    {
      LogError("sendUpdateHMI Backend-->HMI", "Il parametro \"sendChanges\" is a null pointer");
      return;
    }
  #endif

  BaseType_t err = pdTRUE;

  if(*sendChanges == true)
  {
    err = xQueueSend(QueueHandlerHMI_EVT, sendingQueue, TICKS_TO_WAIT_QUEUE_SEND_HMI);
    if(err == pdTRUE)
      *sendChanges = false; // abbassa il flag se l'invio è riuscito
  }

  return err;
}





/**
 * @brief Restituisce una stringa dello stato corrente (SOLO SE LOG ATTIVI)
 */
const char* state_name_to_string(Sequence_t seq_switch)
{
  #ifdef LOG_ACTIVE_MAIN_PRG
    switch(seq_switch)
    {
      case EMERGENCY_STATE          : return "EMERGENCY_STATE";
      case CONTAINER_FULL           : return "CONTAINER_FULL";                
      case TIMEOUT_STATE            : return "TIMEOUT_STATE";           
      case MACHINE_STARTUP_STATE    : return "MACHINE_STARTUP_STATE";       
      case PUNZONE_STARTUP_STATE    : return "PUNZONE_STARTUP_STATE";        
      case TAMBURO_STARTUP_STATE    : return "TAMBURO_STARTUP_STATE";      
      case QUIETE_STATE             : return "QUIETE_STATE";            
      case SERVO_LOADER_OPEN_STATE  : return "SERVO_LOADER_OPEN_STATE";  
      case SERVO_LOADER_CLOSE_STATE : return "SERVO_LOADER_CLOSE_STATE"; 
      case REACH_NEXT_STATION_STATE : return "REACH_NEXT_STATION_STATE"; 
      case PUNCHER_DOWN_FAST_STATE  : return "PUNCHER_DOWN_FAST_STATE";   
      case PUNCHER_DOWN_SLOW_STATE  : return "PUNCHER_DOWN_SLOW_STATE";   
      case PUNCHER_UP_FAST_STATE    : return "PUNCHER_UP_FAST_STATE";
      default                       : return "INVALID MAIN PRG SEQUENCE STATE";       
    }
  #endif
}