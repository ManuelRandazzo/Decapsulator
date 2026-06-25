#include "MotionControl.hpp"

/**
 *  @brief Distruttore della classe, perde la memoria delle variabili e dealloca l'oggetto
 */
MOTION::~MOTION()
{
  Stop(RELEASE);
  /// Cancella il buffer di coda dei movimenti
  vQueueDelete(MoveQueueHandler);
  vQueueDelete(HomingQueueHandler);
}

/*!< Funzione che esegue la task di update */
void MOTION::UpdateMoveHandler(void *pvParameters)
{
  MOTION *THIS = static_cast<MOTION*>(pvParameters);

  while(1)
  {
    uint32_t notifyValue = 0;
    xTaskNotifyWait(0x00, ULONG_MAX, &notifyValue, pdMS_TO_TICKS(2));
    /// Aggiorna la classe del DRV8825
    THIS->Motion.update();
  }
  
  /// Se per qualsiasi ragione dovesse uscire elimina la task
  vTaskDelete(NULL);
}

/**
 *  @private Element Of The Class
 *
 *  @brief Questo metodo ha il compito di gestire in modo ottimale per sistema FreeRTOS il movimento e la velocità del motore
 *
 *  ATTENZIONE: La task viene riattivata da Start e sospesa da Stop quindi è sicuro che non venga mai chiamato MoveHandler per ragioni di sicurezza
 */
void MOTION::MoveHandler(void *pvParameters)
{
  MOTION *THIS = static_cast<MOTION*>(pvParameters);
  TickType_t getLastTick = xTaskGetTickCount();

  uint32_t tmrDebug = 0;
  while(1)
  {
    bool flagRunOnceCMD = false;

    if(!THIS->__isHomingActive)
    {
      if(THIS->selettore == STAND_STILL)
      {
        BaseType_t queueErr = xQueueReceive(THIS->MoveQueueHandler, &THIS->receiverQueue, 0);
        if(queueErr == pdTRUE)
        {
          THIS->selettore = THIS->receiverQueue.__SwitchMove;
          flagRunOnceCMD = true;
        }
      }
      else if(THIS->Motion.isStepDone() == DRV_TRUE)
      {
        THIS->selettore = STAND_STILL;
      }

      if(THIS->HardMax.isAttached())
      {
        THIS->HardMax.update();
        if(THIS->HardMax.event())
        {
          if(THIS->HardMax.rawRead() == THIS->__calib_signal)
          {
            #ifdef LOG_ACTIVE_MOTION
              LogWarning(THIS->TAG("Handler Motion"), "Hard Max Limit Pressed");
            #endif
            THIS->__limit_direction = DIR_POSITIVE;
          }
          else if(THIS->__limit_direction == DIR_POSITIVE)
          {
            THIS->__limit_direction = NO_DIR;
          }
        }
      }

      if(THIS->HardMin.isAttached())
      {
        THIS->HardMin.update();
        if(THIS->HardMin.event())
        {
          if(THIS->HardMin.rawRead() == THIS->__calib_signal)
          {
            #ifdef LOG_ACTIVE_MOTION
              LogWarning(THIS->TAG("Handler Motion"), "Hard Min Limit Pressed");
            #endif
            THIS->__limit_direction = DIR_NEGATIVE;
          }
          else if(THIS->__limit_direction == DIR_NEGATIVE)
          {
            THIS->__limit_direction = NO_DIR;
          }
        }
      }

      if(THIS->__limit_direction != NO_DIR)
      {
        if(THIS->receiverQueue.__dir == THIS->__limit_direction)
        {
          THIS->abortCurrentCommand();
          #ifdef LOG_ACTIVE_MOTION
            LogWarning(THIS->TAG("Handler Motion"), "Hard Limit active, aborting command in dir: %s",
              THIS->receiverQueue.__dir == DIR_NEGATIVE ? "DIR_NEGATIVE" : "DIR_POSITIVE");
          #endif
          flagRunOnceCMD = false;
        }
      }
    }

    /// Halt resta un override universale, attivo anche durante l'homing
    if(THIS->__isHalted)
    {
      #ifdef LOG_ACTIVE_MOTION
        LogWarning(THIS->TAG("Handler Motion"), "Command Halted.\nErasing Queue");
      #endif

      if(uxQueueMessagesWaiting(THIS->MoveQueueHandler) > 0)
        xQueueReset(THIS->MoveQueueHandler);

      THIS->selettore = STAND_STILL;
      THIS->__isHalted = false;
      THIS->Motion.abortCurrentMovement();
      flagRunOnceCMD = false;
    }

    if(!THIS->__isHomingActive && flagRunOnceCMD)
    {
      flagRunOnceCMD = false;
      THIS->Motion.setDirection(THIS->receiverQueue.__dir);

      switch(THIS->selettore)
      {
        case STAND_STILL :
        break;
        case MOVE_REL :
        case MOVE_ABS :
          THIS->Motion.step(THIS->receiverQueue.__move_steps, THIS->receiverQueue.__speed_steps_us);
        break;
        case CONTINUOUS :
          THIS->Motion.stepContinuous(THIS->receiverQueue.__speed_steps_us);
        break;
      }

      #ifdef LOG_ACTIVE_MOTION
        LogWarning(THIS->TAG("Handler Motion"), "SwitchMove(MC state selector) attuale = %s (stato = %d)", THIS->SwitchMoveStr[THIS->selettore], THIS->selettore);
      #endif
    }

    vTaskDelayUntil(&getLastTick, pdMS_TO_TICKS(10));
  }

  vTaskDelete(NULL);
}


void MOTION::HomingHandlerTask(void *pvParameters)
{
  MOTION *THIS = static_cast<MOTION*>(pvParameters);

  TickType_t getLastTick = xTaskGetTickCount();

  MOTION::HomingQueue_t HomingQueue = THIS->defaultHomingQueue;
  DebPinHandler *ptrHardLimit  = nullptr;   /*!< Sensore target dell'homing */
  DebPinHandler *ptrOtherLimit = nullptr;   /*!< Sensore opposto, monitorato solo per sicurezza */

  uint32_t tmrDebug = 0;
  while(1)
  {
    /// Halt() è un override di emergenza: si applica indipendentemente
    /// dallo stato corrente dell'homing
    if(THIS->__homing_state != HOMING_IDLE && THIS->__isHomingHaltRequested)
    {
      THIS->__isHomingHaltRequested = false;
      THIS->abortCurrentCommand();

      #ifdef LOG_ACTIVE_MOTION
        LogWarning(THIS->TAG("Homing Task"), "Homing interrotto da Halt()");
      #endif

      THIS->__isHomeFailed   = true;
      THIS->__isHomingActive = false;
      THIS->__homing_state   = HOMING_IDLE;

      vTaskDelayUntil(&getLastTick, pdMS_TO_TICKS(10));
      continue;
    }

    switch(THIS->__homing_state)
    {
      case HOMING_IDLE :
      {
        /// Si blocca qui finché non arriva una richiesta; non consuma CPU
        xQueueReceive(THIS->HomingQueueHandler, &HomingQueue, portMAX_DELAY);

        /// Scarta eventuali richieste di Halt rimaste pendenti da prima
        THIS->__isHomingHaltRequested = false;

        if(HomingQueue.__hardLimit == HARD_NONE || HomingQueue.__calib_signal == UNKNOWN)
        {
          #ifdef LOG_ACTIVE_MOTION
            LogError(THIS->TAG("Homing Task"), "Richiesta di homing non valida");
          #endif
          THIS->__isHomeFailed   = true;
          THIS->__isHomingActive = false;
          break;
        }

        if(HomingQueue.__hardLimit == HARD_MIN)
        {
          ptrHardLimit  = &THIS->HardMin;
          ptrOtherLimit = &THIS->HardMax;
        }
        else
        {
          ptrHardLimit  = &THIS->HardMax;
          ptrOtherLimit = &THIS->HardMin;
        }

        if(!ptrHardLimit->isAttached())
        {
          #ifdef LOG_ACTIVE_MOTION
            LogError(THIS->TAG("Homing Task"), "Il finecorsa selezionato non è collegato (chiamare setHardLimits)");
          #endif
          THIS->__isHomeFailed   = true;
          THIS->__isHomingActive = false;
          break;
        }

        /// Lettura iniziale con timeout: evita un blocco indefinito se il
        /// debounce non restituisce mai un valore valido
        constexpr uint32_t HOMING_READ_TIMEOUT_MS = 100;
        int8_t initialState = -1;
        TickType_t readStart = millis();

        do
        {
          ptrHardLimit->update();
          initialState = ptrHardLimit->rawRead();
          vTaskDelay(pdMS_TO_TICKS(1));
        }
        while(initialState == -1 && (millis() - readStart) < HOMING_READ_TIMEOUT_MS);

        if(initialState == -1)
        {
          #ifdef LOG_ACTIVE_MOTION
            LogError(THIS->TAG("Homing Task"), "Timeout nella lettura iniziale del finecorsa selezionato");
          #endif
          THIS->__isHomeFailed   = true;
          THIS->__isHomingActive = false;
          break;
        }

        THIS->__isHomeFinished = false;
        THIS->__isHomeFailed   = false;

        if((uint8_t)initialState == (uint8_t)HomingQueue.__calib_signal)
        {
          /// Già sul finecorsa: salta la ricerca
          #ifdef LOG_ACTIVE_MOTION
            LogWarning(THIS->TAG("Homing Task"), "Finecorsa già attivo, salto la ricerca");
          #endif

          if(HomingQueue.__PostHomeVal != 0)
          {
            THIS->Motion.setDirection(HomingQueue.__backDir);
            THIS->Motion.step(HomingQueue.__PostHomeVal, uint64_t(HomingQueue.__home_steps_us*2));
            THIS->__homing_state = HOMING_BACKOFF;
          }
          else
          {
            THIS->Motion.setAbsPosition(0);
            THIS->__isHomeFinished = true;
            THIS->__isHomingActive = false;
            THIS->__homing_state   = HOMING_IDLE;
          }
        }
        else
        {
          #ifdef LOG_ACTIVE_MOTION
            LogWarning(THIS->TAG("Homing Task"), "Avvio ricerca del finecorsa");
          #endif
          THIS->Motion.setDirection(HomingQueue.__search_dir);
          THIS->Motion.stepContinuous(HomingQueue.__home_steps_us);
          THIS->__homing_state = HOMING_SEARCH;
        }

        getLastTick = xTaskGetTickCount();
        break;
      }

      case HOMING_SEARCH :
      {
        ptrHardLimit->update();
        ptrOtherLimit->update();

        if(ptrHardLimit->rawRead() == (uint8_t)HomingQueue.__calib_signal)
        {
          #ifdef LOG_ACTIVE_MOTION
            LogWarning(THIS->TAG("Homing Task"), "Finecorsa trovato, avvio backoff di %lld steps", (long long)HomingQueue.__PostHomeVal);
          #endif

          THIS->abortCurrentCommand();

          if(HomingQueue.__PostHomeVal != 0)
          {
            THIS->Motion.setDirection(HomingQueue.__backDir);
            THIS->Motion.step(HomingQueue.__PostHomeVal, uint64_t(HomingQueue.__home_steps_us*2));
            THIS->__homing_state = HOMING_BACKOFF;
          }
          else
          {
            THIS->Motion.setAbsPosition(0);
            THIS->__isHomeFinished = true;
            THIS->__isHomingActive = false;
            THIS->__homing_state   = HOMING_IDLE;
          }
        }
        else if(ptrOtherLimit->event() && ptrOtherLimit->rawRead() == (uint8_t)HomingQueue.__calib_signal)
        {
          /// Finecorsa OPPOSTO scattato: possibile cablaggio invertito,
          /// direzione di ricerca errata o corsa anomala
          #ifdef LOG_ACTIVE_MOTION
            LogError(THIS->TAG("Homing Task"), "Finecorsa opposto attivato durante la ricerca. Homing abortito.");
          #endif

          THIS->abortCurrentCommand();
          THIS->__isHomeFailed    = true;
          THIS->__isHomingActive  = false;
          THIS->__homing_state    = HOMING_IDLE;
        }

        vTaskDelayUntil(&getLastTick, pdMS_TO_TICKS(10));
        break;
      }

      case HOMING_BACKOFF :
      {
        ptrHardLimit->update();   /// lo aggiorniamo solo per fargli rilevare il rilascio, senza usarne l'evento
        ptrOtherLimit->update();
      
        bool unexpectedLimit = ptrOtherLimit->event() && ptrOtherLimit->rawRead() == (uint8_t)HomingQueue.__calib_signal;
      
        if(unexpectedLimit)
        {
          #ifdef LOG_ACTIVE_MOTION
            LogError(THIS->TAG("Homing Task"), "Finecorsa opposto attivato durante il backoff: corsa anomala. Homing abortito.");
          #endif
      
          THIS->abortCurrentCommand();
          THIS->__isHomeFailed   = true;
          THIS->__isHomingActive = false;
          THIS->__homing_state   = HOMING_IDLE;
        }
        else if(THIS->Motion.isStepDone() == DRV_TRUE)
        {
          THIS->Motion.setAbsPosition(0);
          THIS->__isHomeFinished  = true;
          THIS->__isHomingActive  = false;
          THIS->__homing_state    = HOMING_IDLE;
      
          #ifdef LOG_ACTIVE_MOTION
            LogWarning(THIS->TAG("Homing Task"), "Homing completato con successo");
          #endif
        }
      
        vTaskDelayUntil(&getLastTick, pdMS_TO_TICKS(10));
        break;
      }
    }
  }

  vTaskDelete(NULL);
}

/**
 *  @brief Inizializzatore della @class MOTION
 *
 *  @param numberOfSteps : imposta il numero di step per rotation del motore, non tiene conto del microstepping
 *  @param dir_pin       : imposta il pin che permette di scegliere la direzione del motore
 *  @param step_pin      : imposta il pin che permette di eseguire un passo allo stepper in base al microstepping scelto
 *  @param en_pin        : imposta il pin di abilitazione del pin
 *  @param rst_pin       : imposta il pin di reset del chip
 *  @param sleep_pin     : imposta il pin di sleep (consumo minimo)
 *  @param taskPriority  : imposta la priorità che avrà una Task interna per la gestione dei movimenti del motore
 *  @param microSteps    : imposta il valore di quanti microsteps farà, ovvero la risoluzione. @ref al datasheet per il collegamento dei pin MODE_0 - MODE_2
 *
 *  @note i prossimi due parametri possono essere settati post inizializzazione con la funzione setFaultISR.  
 *  @param fault_pin : associazione al pin nFAULT del driver
 *  @param FaultISR  : Viene associata una Interrupt Service Routine creata dall'utente che verrà eseguita in caso vi sia un problema : Overcurrent, Undervoltage, Overtemperature.
 */
drv_err_t MOTION::Init(uint16_t numberOfSteps, uint8_t dir_pin, uint8_t step_pin, uint8_t en_pin, uint8_t rst_pin, uint8_t sleep_pin, UBaseType_t taskPriority, uSteps_t microSteps, uint8_t fault_pin, void (*FaultISR)(), uint32_t FaultISR_Heap, UBaseType_t FaultISR_priority)
{
  /// Microstep scelti da HardWare
  this->uStepScelti = microSteps;

  /// Passi totali per ogni giro di motore
  this->__stepsMotore = numberOfSteps;
  
  /// Crea il buffer di coda per i movimenti del motore e dell'homing
  /// @link_ref: https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/01-xQueueCreate
  MoveQueueHandler = xQueueCreate(10, sizeof(MoveQueue_t));
  HomingQueueHandler = xQueueCreate(1, sizeof(HomingQueue_t));
  
  /// Inizializza l'Interrupt Service Routine per il pin nFAULT
  if(fault_pin != 255 && FaultISR != nullptr)
	setFaultISR(fault_pin, FaultISR, FaultISR_Heap, FaultISR_priority);

  /// Inizializza il driver e i pin
  drv_err_t errDrv = Motion.begin(dir_pin, step_pin, en_pin, rst_pin, sleep_pin, numberOfSteps);
  if(errDrv != DRV_OK)
	return errDrv;
  this->detach();

  /// Resetta i valori delle variabili della classe, fatto principalmente per portare a valori default "receiverQueue"
  reset();
						  
  /// Inizializza la task per il metodo move
  BaseType_t errTaskInit = xTaskCreatePinnedToCore(MOTION::MoveHandler, "Motion Command", 8192, this, taskPriority, &this->__MoveHandlerTask, APP_CPU_NUM); 

  #ifdef LOG_ACTIVE_MOTION
	if(errTaskInit != pdTRUE)
	{
	  LogError(TAG("Motion Init"), "Inizializzazione della task \"Motion Command\" fallita");
	  return DRV_FAIL;
	}
  #endif
  
  /// Inizializza la task per il metodo move
  errTaskInit = xTaskCreatePinnedToCore(MOTION::UpdateMoveHandler, "Motion Update", 8192, this, 22, &this->__UpdateMoveHandlerTask, PRO_CPU_NUM); 

  #ifdef LOG_ACTIVE_MOTION
	if(errTaskInit != pdTRUE)
	{
	  LogError(TAG("Motion Init"), "Inizializzazione della task di \"Motion Update\" fallita");
	  return DRV_FAIL;
	}
  #endif
  
  /// Inizializza la task per l'homing
  errTaskInit = xTaskCreatePinnedToCore(MOTION::HomingHandlerTask, "Motion Homing", 8192, this, taskPriority, &this->__HomingHandlerTask, APP_CPU_NUM);
  #ifdef LOG_ACTIVE_MOTION
    if(errTaskInit != pdTRUE)
    {
      LogError(TAG("Motion Init"), "Inizializzazione della task \"Motion Homing\" fallita");
      return DRV_FAIL;
    }
  #endif
	
  /// Setta la task in cui verrà chiamato l'update
  errDrv = Motion.setUpdateTask(this->__UpdateMoveHandlerTask);
  if(errDrv != DRV_OK)
	return errDrv;

  /// Il motore inizialmente non è in coppia e aspetta un segnale di Start
  Stop(RELEASE);
  
  return DRV_OK;
}

/**
 *  @brief Setta i limiti massimi e minimi oltre ai quali il motore non può arrivare
 * 
 *  @param pinLimMax Pin che rileva il massimo a cui può arrivare il motore
 * 
 *  @param pinLimMin Pin che rileva il minimo a cui può arrivare il motore
 *
 *  @param IntrOrPoll INTERRUPT oppure POLLING
 *   
 *  @param debounce_ms Tempo per il debounce in millisecondi
 *   
 *  @param input_mode Modalità di input del pin INPUT, INPUT_PULLUP, INPUT_PULLDOWN
 */
void MOTION::setHardLimits(uint8_t pinLimMax, uint8_t pinLimMin, bool IntrOrPoll, uint32_t debounce_ms, uint8_t input_mode, CalibSignal_t levelActive)
{
	if(pinLimMax == 255 && pinLimMin == 255)
	{
		#ifdef LOG_ACTIVE_MOTION
			LogError(TAG("setHardLimits"), "Impossibile settare hard limits senza avere almeno settato uno dei due pin diverso da 255");
		#endif
		return;
	}

	if(levelActive == UNKNOWN)
	{
	  #ifdef LOG_ACTIVE_MOTION
		LogError(TAG("setHardLimits"), "Impossibile dedurre il livello di quando gli hard limit sono attivi o no del Motion");
	  #endif
	  return;
	}
	
	this->__calib_signal = levelActive;
  
	HardMax.begin(IntrOrPoll, pinLimMax, "MotionHardPinMax", debounce_ms, CHANGE, input_mode);

	HardMin.begin(IntrOrPoll, pinLimMin, "MotionHardPinMin", debounce_ms, CHANGE, input_mode);
}

/**
 *  @brief Rimuove i limiti massimi e minimi oltre ai quali il motore non può arrivare
 */
void MOTION::removeHardLimits()
{
  this->HardMax.detach();
  this->HardMin.detach();
}

/**
 *  @brief Ricollega i limiti massimi e minimi oltre ai quali il motore non può arrivare
 */
void MOTION::reattachHardLimits()
{
  this->HardMax.reattach();
  this->HardMin.reattach();
}

/**
 *  @brief Mette in coppia il motore
 */
void MOTION::attach()
{
  Motion.enable();
  __isAttached = true;
}

/**
 *  @brief Disaccoppia il motore
 */
void MOTION::detach()
{
  Motion.disable();
  __isAttached = false;
}   

/**
 *  @brief Restituisce se il motore è attached (true) o no (false)
 */
bool MOTION::isAttached()
{
  return __isAttached;
}

/**
 *  @brief Restituisce se il motore è detached (true) o no (false)
 */
bool MOTION::isDetached()
{
  return !__isAttached;
}

/**
 *  @brief Inizializza il motore con l'Homing in modo che si sappia il punto di partenza.
 * 
 *  @param HomeVelocity_gradi_sec      : Velocità con cui verrà eseguito l'homing
 *  @param HardLimitToReach            : Finecorsa a cui arrivare HARD_MAX o HARD_MIN
 *  @param searchDirection             : Direzione in cui il motore cerca il finecorsa --> DIR_NEGATIVE (default) = clockWise DIR_POSITIVE = counterClockWise
 *  @param gradiDopoHome               : Valore di posizione dopo aver fatto l'homing
 *
 *  @warning QUESTA FUNZIONE ESCE SUBITO ED ESEGUE L'HOMING IN MODO ASINCRONO CON TASK INTERNA.
 *           Solo quando la funzione @see isHomeDone() restituisce true allora sarà effettivamente finito l'home 
 */
void MOTION::home(double HomeVelocity_gradi_sec, HardLimit_t HardLimitToReach, Direction_t searchDirection, double gradiDopoHome)
{
  if(this->__isHomingActive)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError(TAG("Homing"), "Impossibile avviare un nuovo homing: un homing è già in corso");
    #endif
    return;
  }

  if(HardLimitToReach == HARD_NONE)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError(TAG("Homing"), "Impossibile eseguire l'homing senza un HardLimit di riferimento (HARD_NONE)");
    #endif
    return;
  }

  if(this->__calib_signal == UNKNOWN)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError(TAG("Homing"), "Impossibile eseguire l'homing: livello di attivazione del finecorsa non configurato (chiamare setHardLimits)");
    #endif
    return;
  }

  if(HomeVelocity_gradi_sec <= 0)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError(TAG("Homing"), "Impossibile eseguire l'homing con una velocità di homing di : %f", HomeVelocity_gradi_sec);
    #endif
    return;
  }

  if(searchDirection == NO_DIR)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError(TAG("Homing"), "Impossibile eseguire l'homing senza avere una direzione definita (searchDirection = NO_DIR)");
    #endif
    return;
  }

  Direction_t backDir;
  if(gradiDopoHome < 0)
  {
    backDir = searchDirection;
    gradiDopoHome = abs(gradiDopoHome);
  }
  else
    backDir = (searchDirection == DIR_POSITIVE ? DIR_NEGATIVE : DIR_POSITIVE);

  HomingQueue_t HomingQueueDatas = defaultHomingQueue;
  HomingQueueDatas = 
  {
    .__hardLimit     = HardLimitToReach,
    .__backDir       = backDir,
    .__calib_signal  = this->__calib_signal,
    .__home_steps_us = getPeriodDelay(HomeVelocity_gradi_sec),
    .__PostHomeVal   = gradiToSteps(gradiDopoHome),
    .__search_dir    = searchDirection,
  };

  if(xQueueSend(this->HomingQueueHandler, &HomingQueueDatas, pdMS_TO_TICKS(1000)) != pdTRUE)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError(TAG("Homing"), "Impossibile inviare la richiesta di homing alla coda");
    #endif
    return;
  }

  this->__isHomingActive = true;
  this->__isHomeFinished = false;
  this->__isHomeFailed = false;
}



/**
 *  @brief Ritorna se l'Homing è finito o no
 *
 *  @return Restituisce true se l'homing è finito, restituisce false se non è finito
 */
bool MOTION::isHomeDone()
{
  return this->__isHomeFinished;
}




/**
 *  @brief Ritorna se l'Homing è fallito o no
 *
 *  @return Restituisce true se l'homing è fallito, restituisce false se non lo è
 */
bool MOTION::isHomeFailed()
{
  return this->__isHomeFailed;
}




/**
 *  @brief Ferma il motore con il rialascio o il mantenimento della coppia, utile in caso di EMERGENZA
 *
 *  @param rilasciaOppureMantieniCoppia è di default in RELEASE e serve per mantenere o rilasciare la coppia del motore
 *
 *  @note Il motore in RELEASE mode non riceverà più corrente dal driver ma sarà libero di girare se spostato manualmente
		   Invece in HOLD mode manterrà in coppia il motore impedendo che si sposti finchè c'è ancora corrente
 *  @note Se va in sleep consuma meno corrente e impedisce che per sbaglio vengano inviati comandi
 */
void MOTION::Stop(StopReleaseOrHold_t rilasciaOppureMantieniCoppia)
{
  /// Fin da subito non permette più il movimento ignora l'aggiornamento di step
  __isStopped = true;

  /// Rilascia o tiene in coppia il motore
  rilasciaOppureMantieniCoppia == RELEASE ? sleep(true) : Halt();

  /// Sospende le task per ragioni di siurezza in modo che non ripartano finchè non riviene dato lo Start()
  vTaskSuspend(this->__MoveHandlerTask);
  vTaskSuspend(this->__UpdateMoveHandlerTask);
  vTaskSuspend(this->__HomingHandlerTask);
}

/**
 *  @brief Restituisce se il motore è fermo e NON può essere comandato
 */
bool MOTION::isStopped()
{
  return __isStopped;
}

/**
 *  @brief Permette al motore di poter essere comandato
 *
 *  @warning Bisogna chiamare attach() prima di poterlo startare / restartare 
 *           se era stata tolta la coppia al motore 
 */
void MOTION::Start()
{
  /// Riprende le task che erano state precedentemente fermate per ragioni di siurezza con Stop()
  vTaskResume(this->__MoveHandlerTask);
  vTaskResume(this->__UpdateMoveHandlerTask);
  vTaskResume(this->__HomingHandlerTask);

  __isStopped = false;
}

/**
 *  @brief Restituisce se il motore è startato e può essere comandato
 */
bool MOTION::isStarted()
{
  return !(__isStopped);
}

/**
 *  @brief Interrompe il comando che sta attualmente avvenendo qualsiasi esso sia e cancella i movimenti successivi,
 *         ma lasciando il motore in coppia e fermo ma accetta altri comandi senza ridare Start()
 */
void MOTION::Halt()
{
  __isHalted = true;
  __isHomingHaltRequested = true;
}

/**
 *  @brief Abortisce (cancella) il comando attuale
 * 
 *  @returns Numero di step rimanenti del comando abortito
 * 
 *  @attention IsStepDone = true dopo l'esecuzione
 */
uint64_t MOTION::abortCurrentCommand()
{
  return this->Motion.abortCurrentMovement();
}


/**
 *  @brief Muove il motore in una direzione specificata alla velocità specificata o alla velocità precedentemente impostata in modo Relativo
 *
 *  @param gradi il segno determina la direzione e sono i gradi di cui si sposta
 *  @param speed_gradi_al_secondo è la velocità a cui si muove il motore
 */
void MOTION::moveRel(double gradi, double speed_gradi_al_secondo)
{
  /// Struttura temporanea da inviare in coda
  MoveQueue_t QueueDatasToSend = defaultReceiverQueue;
  
  /// Setta la direzione
  QueueDatasToSend.__dir = gradi < 0.0 ? DIR_NEGATIVE : DIR_POSITIVE; //isola il segno per riconoscere la direzione
  
  /// Conta quanti step deve fare (non tiene conto del segno perchè è già impostata la direzione) a __move_steps
  QueueDatasToSend.__move_steps = gradiToSteps(abs(gradi)); //rimuove il segno se c'è e lo associa direttamente a __move_steps

  QueueDatasToSend.__speed_steps_us = getPeriodDelay(abs(speed_gradi_al_secondo));

  /// Setta il selettore dello switch case 
  QueueDatasToSend.__SwitchMove = MOVE_REL;

  /// Invia i dati alla coda
  MoveSendToQueue(QueueDatasToSend);
}



/**
 *  @brief Muove il motore in una direzione specificata alla velocità specificata o alla velocità precedentemente impostata in modo Assoluto
 *
 *  @param gradi il segno determina la direzione e sono i gradi di cui si sposta
 *  @param speed_gradi_al_secondo è la velocità a cui si muove il motore
 *
 *  ATTENZIONE: @bug Il bug consiste nel fatto che veniva fatto un movimento relativo e non uno assoluto
 *                   poiché non teneva conto di quanti step doveva fare e in che direzione per arrivare
 *                   nel voluto punto assoluto
 */
void MOTION::moveAbs(double gradi, double speed_gradi_al_secondo)
{
  /// In base all'attuale posizione riconosce la direzione
  int64_t tmpSteps = gradiToSteps(gradi);

  /// Struttura temporanea da inviare in coda
  MoveQueue_t QueueDatasToSend = defaultReceiverQueue;;

  /// Salva e setta la direzione, va bene qualsiasi siano i segni degli step e dell'absoluteStepCounter
  QueueDatasToSend.__dir = tmpSteps < absoluteStepCounter ? DIR_NEGATIVE : DIR_POSITIVE; 

  /// Conta quanti step deve fare (non tiene conto del segno perchè è già impostata la direzione) a __move_steps
  QueueDatasToSend.__move_steps = absoluteStepCounter - gradiToSteps(abs(gradi)); 

  QueueDatasToSend.__speed_steps_us = getPeriodDelay(abs(speed_gradi_al_secondo));

  /// Setta il selettore dello switch case 
  QueueDatasToSend.__SwitchMove = MOVE_ABS;

  /// Invia i dati alla coda
  MoveSendToQueue(QueueDatasToSend);
}

/**
 *  @brief Muove il motore all'infinito verso la direzione specificata alla velocità specificata o alla velocità precedentemente impostata
 *
 *  @param Direzione in cui gira
 *  @param speed_gradi_al_secondo è la velocità a cui si muove il motore
 */
void MOTION::moveContinuous(Direction_t direzione, double speed_gradi_al_secondo)
{  
  /// Struttura temporanea da inviare in coda
  MoveQueue_t QueueDatasToSend = defaultReceiverQueue;

  QueueDatasToSend.__dir = direzione;
  
  QueueDatasToSend.__speed_steps_us = getPeriodDelay(abs(speed_gradi_al_secondo));

  /// Setta il selettore dello switch case
  QueueDatasToSend.__SwitchMove = CONTINUOUS;

  /// Invia i dati alla coda
  MoveSendToQueue(QueueDatasToSend);
}


/**
 *  @brief Restituisce se il movimento è finito o no così da poterne iniziare un altro
 *
 *  @return se è finito o no il passo e quindi è possibile dare un altro comando
 */
bool MOTION::isStepDone()
{
  return Motion.isStepDone();
}

/**
 *  @brief Driver in low power mode, Disaccoppia il motore, Ignora TUTTI gli Input, spegne : clock, pompa di carica, regolatore interno 
 *         
 *  @param state Se state = FALSE è "sveglio", se state = TRUE allora va in sleep mode, se è già spento o già acceso e viene ripetuta l'operazione non fa nulla
 */
void MOTION::sleep(bool state)
{
  state && !Motion.isSleeping() ? Motion.sleep() : (!state && Motion.isSleeping() ? Motion.wakeup() : false); //false vuol dire che non fa nulla
}



/**
 *  @brief Resetta il driver e le variabili della classe
 */
void MOTION::reset()
{
  Motion.reset();

  receiverQueue = defaultReceiverQueue;        

  absoluteStepCounter = 0;             /*!< Variabile di quanti step ha fatto il motore dall'accensione  */

  __isHomeFinished = false;            /*!< Indica se l'homing è finito o no  */

  __isHomeFailed   = false;			   /*!< Indica se l'homing è fallito o no  */

  __isHomingActive = false;            /*!< Indica se l'homing è attivo o no  */

  __isHomingHaltRequested = false;     /*!< Indica se l'homing ha richiesto l'halt */

  __isStopped = true;                  /*!< Flag di motore stoppato o avviato modificato da Start() e Stop() e restituito da
											isStopped e isStarted  */
  
  __isHalted = false;                   /*!< Flag di motore in Halt modificato da Halt e tutte le azioni di movimento  */

  __isAttached = false;                /*!< Flag di motore stoppato o avviato modificato da Start() e Stop() e restituito da
											isStopped e isStarted  */
}



/**
 *  @brief Converte da gradi a step, funziona sia per la posizione che per la velocità
 *
 *  @param gradi sono i gradi da convertire
 *
 *  @return int64_t steps
 */
int64_t MOTION::gradiToSteps(double gradi)
{
  return (int64_t)((gradi * (double)(uStepScelti * __stepsMotore)) / 360.0);
}



/**
 *  @brief Converte da step a gradi, funziona sia per la posizione che per la velocità
 *
 *  @param steps sono gli steps da convertire
 *
 *  @return double gradi
 */
double MOTION::stepsToGradi(int64_t steps)
{
  return (double)(steps) * 360.0 / (double)(__stepsMotore); 
}



/**
 *  @brief Definizione di una Interrupt Service Routine (ISR) relativa al pin nFAULT del DRV8825 per monitoraggio asincrono.
 *         Il pin nFAULT reporta dei problemi ad una MCU circa l'hardware e viene abbassato in questi casi : Overcurrent, Undervoltage, Overtemperature.
 *  @param fault_pin hardware pin a cui è connesso il pin nFAULT
 *  @param FaultISR Questa sarà la funzione che verrà eseguita quando ci sarà un interrupt nel fault_pin
 *  @param priority Imposta la priorità di Interrupt che avrà a livello globale
 *
 *  ATTENZIONE: Il context switch tra l'ISR e la task di gestione di Interrupt è immediata 
 *
 */
void MOTION::setFaultISR(uint8_t fault_pin, void (*FaultISR)(), uint32_t FaultISR_Heap, UBaseType_t priority)
{
  /// Definisce la funzione di Interrupt Service Routine del pin nFAULT
  this->__FaultISR = FaultISR;
  

  /// collega il pin nFAULT all'Interrupt
  if(fault_pin != 255 && FaultISR != nullptr)
	nFAULT_ISR.Init("nFault_ISR", fault_pin, INPUT, FALLING, 8192, priority, __FaultISR);

  #ifdef LOG_ACTIVE_MOTION
	LogWarning(TAG("MOTION nFault ISR"), "Sto per inizializzare nFaultISR con il pin %d", fault_pin);
	/// collega il pin nFAULT all'Interrupt
	if(fault_pin != 255 && FaultISR != nullptr)
	  LogWarning(TAG("MOTION nFault ISR"), "Sto per Chiamare INTERRUPT.Init");
	else
	  LogWarning(TAG("MOTION nFault ISR"), "Controllare che il pin o la Interrupt Service Routine siano corretti");
  #endif
}

/**
 *  @return la posizione assoluta in gradi
 */
double MOTION::getPosition()
{
  /// @note getAbsPosition appartiene alla @class DRV8825
  return stepsToGradi(Motion.getAbsPosition());
}

/**
 *  @return la posizione assoluta in steps
 */
int64_t MOTION::getPositionInSteps()
{
  /// @note getAbsPosition appartiene alla @class DRV8825
  return Motion.getAbsPosition();
}


/*---------------------------------------------
|                                             |
|            CLASS PRIVATE METHODS            |          
|                                             |
---------------------------------------------*/

/**
 *  @private Element Of The Class
 *
 *  @brief Metodo per ottenere il delay per poter cambiare la velocità del movimento
 */
uint64_t MOTION::getPeriodDelay(const double gradiSecondo)
{
  /// Formula Per ottenere il periodo tra uno step e l'altro tenendo conto del microstepping scelto,
  /// vedi datasheet per ottenere la frequenza di step dato che : Tstep = (1 / Fstep)
  return (uint64_t)(360000000.0 / (gradiSecondo * (double)(this->uStepScelti) * (double)(this->__stepsMotore)));
}


BaseType_t MOTION::MoveSendToQueue(MoveQueue_t StructToSend)
{
  BaseType_t queueValue;

  if(MoveQueueHandler != 0)
  {
	/// Se la coda è piena aspetta 1000 ms = 1s di tempo per inviare
	queueValue = xQueueSend(MoveQueueHandler, &StructToSend, pdMS_TO_TICKS(1000));

	#ifdef LOG_ACTIVE_MOTION
	LogDebug(TAG("MoveSendToQueue"), "Queue value after send = %s", queueValue == pdTRUE ? "pdTRUE" : "pdFALSE");
	#endif
  }

  return queueValue;
}

const char* MOTION::TAG(const char* tag)
{
  this->complete_tag = this->NAME;
  this->complete_tag += this->NAME != "" ? " " : "";
  this->complete_tag += tag;
  return this->complete_tag.c_str();
}