#include "MotionControl.hpp"

/**
 *  @brief Distruttore della classe, perde la memoria delle variabili e dealloca l'oggetto
 */
MOTION::~MOTION()
{
  Stop(RELEASE);
  /// Cancella il buffer di coda dei movimenti
  vQueueDelete(MoveQueueHandler);
}

/*!< Funzione che esegue la task di update */
void MOTION::UpdateMoveHandler(void *pvParameters)
{
  MOTION *THIS = static_cast<MOTION*>(pvParameters);

  while(1)
  {
    /// Aggiorna la classe del DRV8825
    THIS->Motion.update();
    vTaskDelay(5);
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
  /// Crea un'istanza che punta all'oggetto attuale della classe "this" 
  /// il quale è passato come pvParameters, il cast serve per poter usare 
  /// tutti i metodi e tutte le variabili della classe
  /// Questa operazione è necessaria perchè non si possono passare le 
  /// funzioni di una classe quando si crea una task per colpa del puntatore "this->"
  MOTION *THIS = static_cast<MOTION*>(pvParameters);

  TickType_t getLastTick = xTaskGetTickCount();

  while(1)
  {
    bool flagRunOnceCMD = false;

    /// Se il motore non sta eseguendo nessun comando (selettore = STAND_STILL) e non
    /// è stato fermato (perchè lo Stop forza STAND_STILL) allora...
    if(THIS->selettore == STAND_STILL)
    {
      BaseType_t queueErr = xQueueReceive(THIS->MoveQueueHandler, &THIS->receiverQueue, 0);
      
      /// Se è arrivato qualcosa in coda allora invia il comando al driver
      if(queueErr == pdTRUE)
      {
        THIS->selettore = THIS->receiverQueue.__SwitchMove;

        /// Alza il flag di comando
        flagRunOnceCMD = true;
      }
      #ifdef LOG_ACTIVE_MOTION
        LogInfo("Handler Motion", "Command %sReceived", (queueErr == pdTRUE ? "" : "Not "));
      #endif
    }
    else if(THIS->Motion.isStepDone() == DRV_TRUE)
    {
      /// Se era in corso il backoff dell'homing, segnala il completamento
      if(THIS->selettore == HOMING)
      {
        if(THIS->__homing_state == HOMING_BACKOFF)
        {
          /// Ha finito l'homing ed è arretrato, qui c'è il punto zero (0)
          THIS->Motion.setAbsPosition(0);
          THIS->__isHomeFinished = true;
          THIS->__homing_state = HOMING_IDLE;
          THIS->selettore = STAND_STILL;
          #ifdef LOG_ACTIVE_MOTION
            LogInfo("Handler Motion", "Homing completato con successo");
          #endif
        }
      }
      else
        THIS->selettore = STAND_STILL;
    }
    
    if(THIS->HardMax != nullptr)
    {
      THIS->HardMax->IsInterrupt() ? THIS->HardMax->intrUpdate()
                                  : THIS->HardMax->pollUpdate();
      //LogWarning("HardMax", "RawRead HardMax : %s", THIS->HardMax->rawRead()  == HIGH ? "HIGH" : "LOW");
      if(THIS->HardMax->event())
      {
        if(THIS->HardMax->rawRead() == THIS->__calib_signal)
        {
          #ifdef LOG_ACTIVE_MOTION
            LogInfo("Handler Motion", "Hard Max Limit Pressed, blocking DIR_POSITIVE");
          #endif
          THIS->__limit_direction = DIR_NEGATIVE;
        }
        else if(THIS->__limit_direction == DIR_NEGATIVE)
        {
          #ifdef LOG_ACTIVE_MOTION
            LogInfo("Handler Motion", "Hard Max Limit Released, clearing limit direction");
          #endif
          THIS->__limit_direction = NO_DIR;
        }
      }
    }

    if(THIS->HardMin != nullptr)
    {
      THIS->HardMin->IsInterrupt() ? THIS->HardMin->intrUpdate()
                                  : THIS->HardMin->pollUpdate();
      //LogWarning("HardMin", "RawRead HardMin : %s", THIS->HardMin->rawRead()  == HIGH ? "HIGH" : "LOW");

      if(THIS->HardMin->event())
      {
        if(THIS->HardMin->rawRead() == THIS->__calib_signal)
        {
          #ifdef LOG_ACTIVE_MOTION
            LogInfo("Handler Motion", "Hard Min Limit Pressed, blocking DIR_NEGATIVE");
          #endif
          THIS->__limit_direction = DIR_POSITIVE;
        }
        else if(THIS->__limit_direction == DIR_POSITIVE)
        {
          #ifdef LOG_ACTIVE_MOTION
            LogInfo("Handler Motion", "Hard Min Limit Released, clearing limit direction");
          #endif
          THIS->__limit_direction = NO_DIR;
        }
      }
    }

    if(THIS->__limit_direction != NO_DIR)
    {
      THIS->abortCurrentCommand();
      /// Durante l'homing in SEARCH il finecorsa è atteso: avvia il backoff
      if(THIS->selettore == HOMING && THIS->__homing_state == HOMING_SEARCH)
      {
        #ifdef LOG_ACTIVE_MOTION
          LogInfo("Handler Motion", "Homing: finecorsa trovato, avvio backoff di %lu steps",
            receiverQueue.__PostHomeVal);
        #endif

        /// Percorre i passi post-home ad una velocità dimezzata
        THIS->Motion.setDirection(THIS->receiverQueue.__backDir);
        THIS->Motion.step(THIS->receiverQueue.__PostHomeVal, uint64_t(THIS->receiverQueue.__home_steps_us*2), THIS->receiverQueue.__home_acc_steps_s2, THIS->receiverQueue.__home_dec_steps_s2);

        THIS->__homing_state = HOMING_BACKOFF;
        THIS->__limit_direction = NO_DIR;
        flagRunOnceCMD = false;
      }
      else if(THIS->receiverQueue.__dir == THIS->__limit_direction)
      {
        #ifdef LOG_ACTIVE_MOTION
          LogInfo("Handler Motion", "Hard Limit active, aborting command in dir: %s",
            THIS->receiverQueue.__dir == DIR_NEGATIVE ? "DIR_NEGATIVE" : "DIR_POSITIVE");
        #endif
        flagRunOnceCMD  = false;
      }
    }
    
    /// Se è stato dato un comando di halt allora blocca la coda
    if(THIS->__isHalted)
    {
      #ifdef LOG_ACTIVE_MOTION
        LogInfo("Handler Motion", "Command Halted.\nErasing Queue");
      #endif

      /// Toglie qualsiasi movimento successivo contenuto nella coda se non è già vuota
      if(uxQueueMessagesWaiting(THIS->MoveQueueHandler) > 0)
        xQueueReset(THIS->MoveQueueHandler);

      /// Forza il motore a stare in Halt, ovvero interrompe il comando attuale
      THIS->selettore = STAND_STILL;

      /// Sblocca il motore dall'Halt (Halt rimane attivo per un ciclo e basta)
      THIS->__isHalted = false;

      /// Resetta l'homing state interrompendolo se serve
      THIS->__homing_state = HOMING_IDLE;

      /// Abortisce il movimento attuale
      THIS->Motion.abortCurrentMovement();
      
      /// Abbassa il flag di comando
      flagRunOnceCMD = false;
    }
    
    if(flagRunOnceCMD)
    {
      /// Abbassa il flag di comando
      flagRunOnceCMD = false;

      /// Imposta la direzione
      THIS->Motion.setDirection(THIS->receiverQueue.__dir);

      /// Invio dei comandi
      switch(THIS->selettore)
      {
        case STAND_STILL :
          // Do nothing
        break;
        /// Avvia il movimento continuo verso il finecorsa di calibrazione
        case HOMING :
          THIS->__homing_state = HOMING_SEARCH;
          THIS->Motion.stepContinuous(THIS->receiverQueue.__home_steps_us);
        break;
        /// Invia il comando di fare un movimento di tot steps in una direzione specificata
        case MOVE_REL :
        case MOVE_ABS :
          THIS->Motion.step(THIS->receiverQueue.__move_steps, THIS->receiverQueue.__speed_steps_us, THIS->receiverQueue.__acc_steps_s2, THIS->receiverQueue.__dec_steps_s2);
        break;
        /// Invia il comando che fa un passo finché non viene ricevuto un altro dato dalla queue
        case CONTINUOUS :
          THIS->Motion.stepContinuous(THIS->receiverQueue.__speed_steps_us);
        break;
      }
      
      /// Gestione dei log
      #ifdef LOG_ACTIVE_MOTION
        LogInfo("Handler Motion", "SwitchMove(MC state selector) attuale = %s (stato = %d)", SwitchMoveStr[selettore], selettore);
      #endif
    }
  

    #ifdef LOG_ACTIVE_MOTION
      static uint32_t time = 0;
      uint32_t actualTime = millis();
      if(actualTime - time >= 8000)
      {
        time = actualTime;
        if(err != DRV_OK && err != DRV_NO_NOTIFY && err != DRV_WAITING_RMT_TX_TO_FINISH)
          LogError("Errore Update DRV8825", "Driver Error : %s", drv_err_to_name(err));
        else
          LogInfo("Update DRV8825", "%s", drv_err_to_name(err));
      }
    #endif



    
    //vTaskDelayUntil(&getLastTick, pdMS_TO_TICKS(10)); /// Permette di fare lo switch tra le task
  }
  
  /// Se per qualsiasi ragione dovesse uscire elimina la task
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
  uStepScelti = microSteps;

  /// Passi totali per ogni giro di motore
  __stepsMotore = numberOfSteps;

  /// Crea il buffer di coda per i movimenti del motore
  /// @link_ref: https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/01-xQueueCreate
  MoveQueueHandler = xQueueCreate(10, sizeof(MoveQueue_t));
  
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
      LogError("Motion Init", "Inizializzazione della task \"Motion Command\" fallita");
      return DRV_FAIL;
    }
  #endif
  
  /// Inizializza la task per il metodo move
  errTaskInit = xTaskCreatePinnedToCore(MOTION::UpdateMoveHandler, "Motion Update", 2048, this, 20, &this->__UpdateMoveHandlerTask, PRO_CPU_NUM); 

  #ifdef LOG_ACTIVE_MOTION
    if(errTaskInit != pdTRUE)
    {
      LogError("Motion Init", "Inizializzazione della task di \"Motion Update\" fallita");
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
 * 
 *  @param levelActive è il segnale per il quale il sensore viene considerato come triggerato.
 *                     Valori accetati ACTIVE_LOW, ACTIVE_HIGH
 */
void MOTION::setHardLimits(uint8_t pinLimMax, uint8_t pinLimMin, bool IntrOrPoll, uint32_t debounce_ms, uint8_t input_mode, CalibSignal_t levelActive)
{

  if(pinLimMax == 255 && pinLimMin == 255)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError("setHardLimits", "Impossibile settare hard limits senza avere almeno settato uno dei due pin diverso da 255");
    #endif
    return;
  }

  if(levelActive == UNKNOWN)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError("setHardLimits", "Impossibile dedurre il livello di quando gli hard limit sono attivi o no del Motion");
    #endif
    return;
  }
  
  this->__calib_signal = levelActive;
  
  if(pinLimMax != 255)
  {
    DebPinHandler* ptrHardMax = new DebPinHandler(IntrOrPoll, pinLimMax, "MotionHardPinMax", debounce_ms, CHANGE, input_mode);
    
    if(ptrHardMax == 0)
    {
      #ifdef LOG_ACTIVE_MOTION
        LogError("Hard Max Pin", "Impossibile allocare memoria per il pin Max del Motion");
      #endif
      return;
    }

    this->HardMax = ptrHardMax;
  }
  
  if(pinLimMin != 255)
  {
    DebPinHandler* ptrHardMin = new DebPinHandler(IntrOrPoll, pinLimMin, "MotionHardPinMin", debounce_ms, CHANGE, input_mode);

    if(ptrHardMin == 0)
    {
      #ifdef LOG_ACTIVE_MOTION
        LogError("Hard Min Pin", "Impossibile allocare memoria per il pin Min del Motion");
      #endif
      return;
    }

    this->HardMin = ptrHardMin;
  }

}

/**
 *  @brief Rimuove i limiti massimi e minimi oltre ai quali il motore non può arrivare
 */
void MOTION::removeHardLimits()
{
  if(this->HardMax != nullptr)
  {
    /// Copia l'istanza
    this->HardMaxCpy = this->HardMax;

    /// Rimuove il valore nel puntatore
    this->HardMax = nullptr;
  }

  if(this->HardMin != nullptr)
  {
    /// Copia l'istanza
    this->HardMinCpy = this->HardMin;

    /// Rimuove il valore nel puntatore
    this->HardMin = nullptr;
  }
}

/**
 *  @brief Ricollega i limiti massimi e minimi oltre ai quali il motore non può arrivare
 */
void MOTION::reattachHardLimits()
{
  if(this->HardMaxCpy != nullptr)
  {
    this->HardMax = this->HardMaxCpy;

    /// Rimuove il valore nel puntatore di copia
    this->HardMaxCpy = nullptr;
  }

  if(this->HardMinCpy != nullptr)
  {
    this->HardMin = this->HardMinCpy;

    /// Rimuove il valore nel puntatore di copia
    this->HardMinCpy = nullptr;
  }
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
 *  @param HomeVelocity_gradi_sec         : Velocità con cui verrà eseguito l'homing
 *  @param searchDirection                : Direzione in cui il motore cerca il finecorsa --> DIR_NEGATIVE (default) = clockWise DIR_POSITIVE = counterClockWise
 *  @param gradiDopoHome                  : Valore di posizione dopo aver fatto l'homing
 *
 *  @warning QUESTA FUNZIONE ESCE SUBITO ED ESEGUE L'HOMING IN MODO ASINCRONO CON TASK INTERNA.
 *           Solo quando la funzione @see isHomeDone() restituisce true allora sarà effettivamente finito l'home 
 */
void MOTION::home(double HomeVelocity_gradi_sec, double acc_gradi_al_secondo_quadro, double dec_gradi_al_secondo_quadro, Direction_t searchDirection, double gradiDopoHome)
{
  if(HomeVelocity_gradi_sec <= 0)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError("Homing", "Impossibile eseguire l'homing con una velocità di homing di : %f", HomeVelocity_gradi_sec);
    #endif
    return;
  }

  if(searchDirection == NO_DIR)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogError("Homing", "Impossibile eseguire l'homing senza avere una direzione definita (searchDirection = NO_DIR)");
    #endif
    return;
  }

  Direction_t backDir;
  if(gradiDopoHome < 0) /// Mantiene la stessa direzione
  {
    backDir = searchDirection;
    gradiDopoHome = abs(gradiDopoHome);
  }
  else /// Inverte la direzione
    backDir = (searchDirection == DIR_POSITIVE ? DIR_NEGATIVE : DIR_POSITIVE);
  

  /// Struct per inviare il buffer dati
  
  MoveQueue_t HomeQueueDatas = defaultReceiverQueue;
  HomeQueueDatas =
  {
    .__SwitchMove = HOMING,
    .__home_steps_us = getPeriodDelay(HomeVelocity_gradi_sec),
    .__home_acc_steps_s2 = gradiToSteps(abs(acc_gradi_al_secondo_quadro)),
    .__home_dec_steps_s2 = gradiToSteps(abs(dec_gradi_al_secondo_quadro)),
    .__backDir = backDir,
    .__PostHomeVal = gradiToSteps(gradiDopoHome),
    .__speed_steps_us = 10,
    .__dir = searchDirection,
  };

  /// Abbassa il Flag di Home finito
  this->__isHomeFinished = false;

  /// Invia i dati alla coda
  MoveSendToQueue(HomeQueueDatas);
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

  /// Sospende la task per ragioni di siurezza in modo che non riparta finchè non riviene dato lo Start()
  vTaskSuspend(this->__MoveHandlerTask);
  vTaskSuspend(this->__UpdateMoveHandlerTask);
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
  /// Riprende la task che era stata precedentemente fermata per ragioni di siurezza con Stop()
  vTaskResume(this->__MoveHandlerTask);
  vTaskResume(this->__UpdateMoveHandlerTask);

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
void MOTION::moveRel(double gradi, double speed_gradi_al_secondo, double acc_gradi_al_secondo_quadro, double dec_gradi_al_secondo_quadro)
{
  /// Struttura temporanea da inviare in coda
  MoveQueue_t QueueDatasToSend = defaultReceiverQueue;

  /// Setta la direzione
  QueueDatasToSend.__dir = gradi < 0.0 ? DIR_NEGATIVE : DIR_POSITIVE; //isola il segno per riconoscere la direzione
  
  /// Conta quanti step deve fare (non tiene conto del segno perchè è già impostata la direzione) a __move_steps
  QueueDatasToSend.__move_steps = gradiToSteps(abs(gradi)); //rimuove il segno se c'è e lo associa direttamente a __move_steps

  QueueDatasToSend.__speed_steps_us = getPeriodDelay(abs(speed_gradi_al_secondo));
  QueueDatasToSend.__acc_steps_s2 = gradiToSteps(abs(acc_gradi_al_secondo_quadro));
  QueueDatasToSend.__dec_steps_s2 = gradiToSteps(abs(dec_gradi_al_secondo_quadro));

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
void MOTION::moveAbs(double gradi, double speed_gradi_al_secondo, double acc_gradi_al_secondo_quadro, double dec_gradi_al_secondo_quadro)
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
  QueueDatasToSend.__acc_steps_s2 = gradiToSteps(abs(acc_gradi_al_secondo_quadro));
  QueueDatasToSend.__dec_steps_s2 = gradiToSteps(abs(dec_gradi_al_secondo_quadro));

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
  return (int64_t)((gradi / 360.0) * double(uStepScelti * __stepsMotore));
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
  return (double)((steps * 360) / __stepsMotore); 
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
    LogInfo("MOTION nFault ISR", "Sto per inizializzare nFaultISR con il pin %d", fault_pin);
    /// collega il pin nFAULT all'Interrupt
    if(fault_pin != 255 && FaultISR != nullptr)
      LogInfo("MOTION nFault ISR", "Sto per Chiamare INTERRUPT.Init");
    else
      LogWarning("MOTION nFault ISR", "Controllare che il pin o la Interrupt Service Routine siano corretti");
  #endif
}

/**
 *  @return la posizione assoluta in gradi
 */
double MOTION::getPosition()
{
  /// @note getAbsPosition appartiene alla @class DRV8825
  return gradiToSteps(Motion.getAbsPosition());
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
 *  @brief Funzione per ottenere il delay per poter cambiare la velocità del movimento
 */
uint64_t MOTION::getPeriodDelay(const double gradiSecondo)
{
  /// Formula Per ottenere il periodo tra uno step e l'altro tenendo conto del microstepping scelto,
  /// vedi datasheet per ottenere la frequenza di step dato che : Tstep = (1 / Fstep)
  return (uint64_t)(360000000.0 / (gradiSecondo * double(this->uStepScelti * this->__stepsMotore)));
}


BaseType_t MOTION::MoveSendToQueue(MoveQueue_t StructToSend)
{
  BaseType_t queueValue;

  if(MoveQueueHandler != 0)
  {
    /// Se la coda è piena aspetta 1000 ms = 1s di tempo per inviare
    queueValue = xQueueSend(MoveQueueHandler, &StructToSend, pdMS_TO_TICKS(1000));

    #ifdef LOG_ACTIVE_MOTION
    LogDebug("MoveSendToQueue", "Queue value after send = %s", queueValue == pdTRUE ? "pdTRUE" : "pdFALSE");
    #endif
  }

  return queueValue;
}