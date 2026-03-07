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

/**
 *  @private Element Of The Class
 *
 *  @brief Questo metodo ha il compito di gestire in modo ottimale per sistema FreeRTOS il movimento e la velocità del motore
 * 
 *  ATTENZIONE: La task viene riattivata da Start e sospesa da Stop quindi è sicuro che non venga mai chiamato MoveHandler per ragioni di sicurezza
 */
void MOTION::MoveHandler()
{
  bool flagRunOnceCMD = false;

  /// Se il motore non sta eseguendo nessun comando (selettore = STAND_STILL) e non
  /// è stato fermato (perchè lo Stop forza STAND_STILL) allora...
  if(this->selettore == STAND_STILL)
  {
    BaseType_t queueErr = xQueueReceive(MoveQueueHandler, &receiverQueue, 0);
    
    /// Se è arrivato qualcosa in coda allora invia il comando al driver
    if(queueErr == pdTRUE)
    {
      this->selettore = receiverQueue.__SwitchMove;

      /// Alza il flag il flag di comando
      flagRunOnceCMD = true;
    }
    #ifdef LOG_ACTIVE_MOTION
      LogInfo("Handler Motion", "Command %sReceived", (queueErr == pdTRUE ? "" : "Not "));
    #endif
  }
  else if(Motion.isStepDone() == DRV_TRUE)
    this->selettore = STAND_STILL;

  /// Se è stato dato un comando di halt allora blocca la coda
  if(this->__isHalted)
  {
    #ifdef LOG_ACTIVE_MOTION
      LogInfo("Handler Motion", "Command Halted.\nErasing Queue");
    #endif

    /// Toglie qualsiasi movimento successivo contenuto nella coda se non è già vuota
    if(uxQueueMessagesWaiting(MoveQueueHandler) > 0)
      xQueueReset(MoveQueueHandler);

    /// Forza il motore a stare in Halt, ovvero interrompe il comando attuale
    this->selettore = STAND_STILL;

    /// Sblocca il motore dall'Halt (Halt rimane attivo per un ciclo e basta)
    this->__isHalted = false;

    /// Abortisce il movimento attuale
    Motion.abortCurrentMovement();
    
    /// Abbassa il flag di comando
    flagRunOnceCMD = false;
  }
  
  if(flagRunOnceCMD)
  {
    /// Abbassa il flag di comando
    flagRunOnceCMD = false;

    /// Imposta la direzione
    Motion.setDirection(receiverQueue.__dir);

    /// Invio dei comandi
    switch(this->selettore)
    {
      /// Invia il comando di fare un movimento di tot steps in una direzione specificata
      case MOVE_REL :
      case MOVE_ABS :
        Motion.step(receiverQueue.__move_steps, receiverQueue.__speed_steps_us);
      break;
      /// Invia il comando che fa un passo finché non viene ricevuto un altro dato dalla queue
      case CONTINUOUS :
        Motion.stepContinuous(receiverQueue.__speed_steps_us);
      break;
    }
    
    /// Gestione dei log
    #ifdef LOG_ACTIVE_MOTION
      LogInfo("Handler Motion", "SwitchMove(MC state selector) attuale = %s (stato = %d)", SwitchMoveStr[selettore], selettore);
    #endif
  }

  
  /// Aggiorna la classe del DRV8825
  drv_err_t err = Motion.update();
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

  
  //vTaskDelay(pdMS_TO_TICKS(0)); /// Permette di fare lo switch tra le task
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
  MoveQueueHandler = xQueueCreate(5, sizeof(MoveQueue_t));
  
  /// Inizializza l'Interrupt Service Routine per il pin nFAULT
  if(fault_pin != 255 && FaultISR != nullptr)
    setFaultISR(fault_pin, FaultISR, FaultISR_Heap, FaultISR_priority);

  /// Inizializza il driver e i pin
  drv_err_t errDrv = Motion.begin(dir_pin, step_pin, en_pin, rst_pin, sleep_pin, numberOfSteps);
  if(errDrv != DRV_OK)
    return errDrv;

  /// Resetta i valori delle variabili della classe, fatto principalmente per portare a valori default "receiverQueue"
  reset();
                          
  /// Inizializza la task per il metodo move
  BaseType_t errTaskInit = MoveHandlerTask.Init<MOTION>("Move Handler", 8192, NULL, taskPriority, 0, this, nullptr, &MOTION::MoveHandler);
  
  #ifdef LOG_ACTIVE_MOTION
    if(errTaskInit != pdTRUE)
    {
      LogError("Motion Init", "Inizializzazione della task fallita");
      return DRV_FAIL;
    }
  #endif
  
  
  /// Setta la task in cui verrà chiamato l'update
  errDrv = Motion.setUpdateTask(MoveHandlerTask.getHandler());
  if(errDrv != DRV_OK)
    return errDrv;

  /// Il motore inizialmente non è in coppia e aspetta un segnale di Start
  Stop(RELEASE);

  return DRV_OK;
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
 *  @param calibrationPin                 : Pin su cui viene segnalato il finecorsa
 *  @param inputModePin                   : Modalità di input del pin che può essere : INPUT, INPUT_PULLUP, INPUT_PULLDOWN
 *  @param triggerMode                    : Modalità con cui viene triggerato l'interrupt : FALLING, RISING, CHANGE, HIGH, LOW
 *  @param HomeVelocity_gradi_sec         : Velocità con cui verrà eseguito l'homing
 *  @param searchDirection                : Direzione in cui il motore cerca il finecorsa --> DIR_NEGATIVE (default) = clockWise DIR_POSITIVE = counterClockWise
 *  @param gradiDopoHome                  : Valore di posizione dopo aver fatto l'homing
 *  @param quanteVolteToccaIlSensore      : Indica quante volte passa sul sensore per essere effettivamente calibrato
 *  @param calibCamSignal                 : Segnale del finecorsa quando attivo --> ACTIVE_LOW (default) = attivo basso, ACTIVE_HIGH = attivo alto
 *
 *  @note Se il @param calibCamSignal è ACTIVE_LOW allora il segnale viene invertito internamente nella definizione del pin
 *
 *  @warning QUESTA FUNZIONE ESCE SUBITO ED ESEGUE L'HOMING IN MODO ASINCRONO CON TASK INTERNA.
 *           Solo quando la funzione @see isHomeDone() restituisce true allora sarà effettivamente finito l'home 
 */
void MOTION::home(uint8_t calibrationPin, uint8_t inputModePin, uint8_t triggerMode, double HomeVelocity_gradi_sec, Direction_t searchDirection,
                         double gradiDopoHome, uint8_t quanteVolteToccaIlSensore, CalibSignal_t calibCamSignal)
{
  /// Struct per inviare il buffer dati
  MoveQueue_t HomeQueueDatas = { .__home_steps_us = 10, .__nCalibTouch = 1, .__speed_steps_us = 10 };

  /// Viene salvato il pin del sensore
  HomeQueueDatas.__calibPin = calibrationPin;

  /// Viene salvato il segnale che assume il sensore al passaggio
  HomeQueueDatas.__calibSig = calibCamSignal;

  /// Imposta la direzione
  HomeQueueDatas.__dir = searchDirection;

  /// Imposta la velocità di Home
  HomeQueueDatas.__home_steps_us = getPeriodDelay(HomeVelocity_gradi_sec);
  
  /// Quante volte deve toccare il sensore per finire l'homing
  HomeQueueDatas.__nCalibTouch = quanteVolteToccaIlSensore - uint8_t(getCamSignal()); //rimuove uno se è già attivo il finecorsa

  /// Abbassa il Flag di Home finito
  __isHomeFinished = false;

  /// Post Home position
  HomeQueueDatas.__absPostHomeVal = gradiToSteps(gradiDopoHome);
  
  /// Setta il selettore dello switch case 
  HomeQueueDatas.__SwitchMove = HOMING;

  /// Invia i dati alla coda
  MoveSendToQueue(HomeQueueDatas);

  /// Nel caso in cui non sia definito l'handler dell'interrupt allora lo inizializza
  if(HOME_IT.getHandlerIT() == NULL)
    /// Task per l'handling dell'interrupt dell'Homing
    HOME_IT.Init<MOTION>("Motion Homing Interrupt", calibrationPin, inputModePin, triggerMode, 2048, 20, this, &MOTION::HomingReachedISR);
  else
    HOME_IT.resumeISR();
}



/**
 *  @brief Ritorna se l'Homing è finito o no
 *
 *  @return Restituisce true se l'homing è finito, restituisce false se non è finito
 */
bool MOTION::isHomeDone()
{
  return __isHomeFinished;
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
  MoveHandlerTask.Suspend();
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
  MoveHandlerTask.Resume();

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
 *  @brief Muove il motore in una direzione specificata alla velocità specificata o alla velocità precedentemente impostata in modo Relativo
 *
 *  @param gradi il segno determina la direzione e sono i gradi di cui si sposta
 *  @param speed_gradi_al_secondo è la velocità a cui si muove il motore
 */
void MOTION::moveRel(double gradi, double speed_gradi_al_secondo)
{
  /// Struttura temporanea da inviare in coda
  MoveQueue_t QueueDatasToSend = { .__home_steps_us = 10, .__nCalibTouch = 1, .__speed_steps_us = 10 };

  /// Setta la direzione
  QueueDatasToSend.__dir = gradi < 0.0 ? DIR_NEGATIVE : DIR_POSITIVE; //isola il segno per riconoscere la direzione
  
  /// Conta quanti step deve fare (non tiene conto del segno perchè è già impostata la direzione) a __move_steps
  QueueDatasToSend.__move_steps = gradiToSteps(abs(gradi)); //rimuove il segno se c'è e lo associa direttamente a __move_steps

  /// Mantiene la velocità precedentemente data se la velocità è <= 0.0
  if(speed_gradi_al_secondo > 0.0)
    QueueDatasToSend.__speed_steps_us = getPeriodDelay(speed_gradi_al_secondo);

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
  MoveQueue_t QueueDatasToSend = { .__home_steps_us = 10, .__nCalibTouch = 1, .__speed_steps_us = 10 };

  /// Salva e setta la direzione, va bene qualsiasi siano i segni degli step e dell'absoluteStepCounter
  QueueDatasToSend.__dir = tmpSteps < absoluteStepCounter ? DIR_NEGATIVE : DIR_POSITIVE; 

  /// Conta quanti step deve fare (non tiene conto del segno perchè è già impostata la direzione) a __move_steps
  QueueDatasToSend.__move_steps = absoluteStepCounter - gradiToSteps(abs(gradi)); 

  /// Mantiene la velocità precedentemente data se la velocità è <= 0.0
  if(speed_gradi_al_secondo > 0.0)
    QueueDatasToSend.__speed_steps_us = getPeriodDelay(speed_gradi_al_secondo);

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
  MoveQueue_t QueueDatasToSend = { .__home_steps_us = 10, .__nCalibTouch = 1, .__speed_steps_us = 10 };

  QueueDatasToSend.__dir = direzione;
  
  /// Mantiene la velocità precedentemente data se la velocità è <= 0.0
  if(speed_gradi_al_secondo > 0.0)
    QueueDatasToSend.__speed_steps_us = getPeriodDelay(speed_gradi_al_secondo);

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

  receiverQueue =                      /*!< Struct che contiene i dati fa il reset (default values) degli attuali dati ricevuti  */
  {
    .__SwitchMove = STAND_STILL,       /*!< Variabile switch per il movimento del motore nella task  */

    /// Dati Homing
    .__home_steps_us = 10,             /*!< Velocità dell'homing in step/secondo  */
    .__calibPin = 63,                  /*!< indica il pin di calibrazione (finecorsa), 6 bits = 64pin max  */
    .__calibSig = false,               /*!< è il valore che assume il sensore quando viene attivato  */
    .__nCalibTouch = 1,                /*!< indica quante volte viene toccato il sensore per far sì che sia calibrato  */
    .__absPostHomeVal = 0,             /*!< è il valore assoluto che viene associato dopo l'homing  */

    /// Altri Dati
    .__speed_steps_us = 10,            /*!< Velocità step/secondo  */
    .__move_steps = 0,                 /*!< Passi da eseguire scelti in runtime  */
    .__dir = DIR_NEGATIVE,             /*!< Direzione che verrà impostata all'invio del comando  */
  };        

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
  __FaultISR = FaultISR;
  

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
  return gradiToSteps(getAbsPosition());
}

/**
 *  @return la posizione assoluta in steps
 */
int64_t MOTION::getPositionInSteps()
{
  /// @note getAbsPosition appartiene alla @class DRV8825
  return getAbsPosition();
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



/**
 *  @private Element Of The Class
 *
 *  @brief Interrupt Service Routine per il segnale del finecorsa che viene eseguito su una task dedicata, @see @ref @file Interrupts.hpp --> @class INTERRUPTS
 */
void MOTION::HomingReachedISR()
{
  if(getCamSignal()) /// Se non è stato fermato e il segnale è valido allora...
  {
    if(receiverQueue.__nCalibTouch > 0)
    {
      /// si assicura che NON abbia finito l'homing
      __isHomeFinished = false;

      /// ha toccato una volta il sensore quindi decrementa i tocchi
      receiverQueue.__nCalibTouch--;    

      /// fa un passo finché viene chiamata la task e non viene fatto l'interrupt sul __calibPin
      /// trova la frequenza necessaria per il delay in microsecondi
      Motion.step(1, receiverQueue.__home_steps_us);    

    }
    else if(!__isHomeFinished && receiverQueue.__nCalibTouch == 0)
    {
      /// ha finito l'homing
      __isHomeFinished = true;

      /// Disattiva l'interrupt per saturare meno la CPU
      HOME_IT.suspendISR();

      /// Setta la nuova posizione assoluta e anche quella relativa
      absoluteStepCounter = receiverQueue.__absPostHomeVal;
    }
  }    
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


/**
 *  @private Element Of The Class
 *
 *  @brief Funzione che restituisce 1 quando il finecorsa è attivo
 *
 *  @return bool OUTPUT della truth table:
 *
 *  @note Casi possibili con la XNOR tra __calibPin e __calibSig TRUTH-TABLE :
 *        ╔══════════╦══════════╦════════╗
 *        ║ calibPin ║ calibSig ║ OUTPUT ║
 *        ╠══════════╬══════════╬════════╣
 *        ║     0    ║     0    ║    1   ║
 *        ╠══════════╬══════════╬════════╣
 *        ║     0    ║     1    ║    0   ║
 *        ╠══════════╬══════════╬════════╣
 *        ║     1    ║     0    ║    0   ║
 *        ╠══════════╬══════════╬════════╣
 *        ║     1    ║     1    ║    1   ║
 *        ╚══════════╩══════════╩════════╝
 */
bool MOTION::getCamSignal()
{
  return !(digitalReadFast(receiverQueue.__calibPin) ^ receiverQueue.__calibSig);
}