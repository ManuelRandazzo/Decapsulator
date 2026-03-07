#include "Interrupts.hpp"



/*---------------------------------------------
|                                             |
|            CLASS PUBLIC METHODS             |          
|                                             |
---------------------------------------------*/




/**
 *  @brief Costruttore per le operazioni che richiedono un Interrupt Pin @for Normali Funzioni
 *
 *  @param name                     : Questo è il nome unico che verrà visto nella console quando inizializza la task di Interrupt
 *  @param pin                      : Hardware Pin in cui verrà eseguito l'Input Interrupt 
 *  @param modePin                  : INPUT, INPUT_PULLUP, INPUT_PULLDOWN  sono le possibili modalità di input del pin
 *  @param IT_TriggerMode           : RISING, FALLING, CHANGE, HIGH, LOW   sono le modalità di Interrupt
 *  @param HeapSize                 : Quantità di stack riservata alla task
 *  @param priority                 : Priorità che ha la task @warning: SI CONSIGLIA CHE SIA IL PIU' ALTO POSSIBILE
 *  @param functionToExecute        : E' la funzione dell'Utente che verrà chiamata dall'ISR e gestita da una task interna
 */

void INTERRUPTS::Init(const char *name, uint8_t pin, uint8_t modePin, uint8_t IT_TriggerMode, uint32_t HeapSize,
                      UBaseType_t priority, void (*functionToExecute)())
{
  LogInfo("ISR Init Task Creation", "sono all'Init di %s e TaskHandler è %d", name, TaskIT.getHandler());
  /// Nel caso in cui non sia definito l'handler della task allora lo inizializza
  if(TaskIT.getHandler() == NULL)
  {
    /// Inizializza l'oggetto per una task che viene triggerata dall'Interrupt
    /// e copia la funzione ISR per poterla eseguire
    TaskIT.Init<INTERRUPTS>(name, HeapSize, NULL, priority, 0, this, nullptr, &INTERRUPTS::__wrapperTaskLoopIT);
  }

  /// Copia la funzione ISR per poterla eseguire
  setISR(functionToExecute);

  /// Si salva l'handler della task
  TaskHandler = TaskIT.getHandler();
  
  /// Imposta il codice che verrà eseguito quando arriverà l'Interrupt
  TaskIT.setTask<INTERRUPTS>(this, nullptr, &INTERRUPTS::__wrapperTaskLoopIT);

  /// Attach dell'interrupt
  changePinIT(pin, modePin, IT_TriggerMode);
}



/**
 *  @brief Costruttore per le operazioni che richiedono un Interrupt Pin per le Classi
 *
 *  @tparam Sarebbe il modello della classe chiamante 
 *
 *  @param name                     : Questo è il nome unico che verrà visto nella console quando inizializza la task di Interrupt
 *  @param pin                      : Hardware Pin in cui verrà eseguito l'Input Interrupt 
 *  @param modePin                  : INPUT, INPUT_PULLUP, INPUT_PULLDOWN  sono le possibili modalità di input del pin
 *  @param IT_TriggerMode           : RISING, FALLING, CHANGE, HIGH, LOW   sono le modalità di Interrupt
 *  @param HeapSize                 : Quantità di stack riservata alla task
 *  @param priority                 : Priorità che ha la task @warning SI CONSIGLIA CHE SIA IL PIU' ALTO POSSIBILE
 *  @param thisPointer              : E' il puntatore della classe chiamante passato come argomento
 *  @param methodToExecute          : E' la funzione dell'Utente che verrà chiamata dall'ISR e gestita da una task interna
 */
template <class refClass>
void INTERRUPTS::Init(const char *name, uint8_t pin, uint8_t modePin, uint8_t IT_TriggerMode, uint32_t HeapSize,
                      UBaseType_t priority, refClass* thisPointer, void (refClass::*methodToExecute)())
{
  LogInfo("ISR Init Task Creation", "sono all'Init di %s e TaskHandler è %d", name, TaskIT.getHandler());
  /// Nel caso in cui non sia definito l'handler della task allora lo inizializza
  if(TaskIT.getHandler() == NULL)
    /// Inizializza l'oggetto per una task che viene triggerata dall'Interrupt
    TaskIT.Init<INTERRUPTS>(name, HeapSize, NULL, priority, 0, this, nullptr, &INTERRUPTS::__wrapperTaskLoopIT);

  /// Copia il metodo ISR per poterlo eseguire
  setISR<refClass>(thisPointer, methodToExecute);
  
  /// Si salva l'handler della task
  TaskHandler = TaskIT.getHandler();

  /// Imposta il codice che verrà eseguito quando arriverà l'Interrupt
  TaskIT.setTask<INTERRUPTS>(this, nullptr, &INTERRUPTS::__wrapperTaskLoopIT);

  /// Attach dell'interrupt
  changePinIT(pin, modePin, IT_TriggerMode);
}

/**
 *  @brief distruttore della classe che elimina la Task e disaccoppia il pin dall'Interrupt
 */
INTERRUPTS::~INTERRUPTS()
{
  if(__isInterruptAttached)
    detachInterrupt(this->pin);
}

/**
 *  @brief Metodo per attivare / riattivare un interrupt (esegue l'attach)
 */
void INTERRUPTS::resumeISR()
{
  if(!__isInterruptAttached)
  {
    TaskIT.Resume();
    attachInterruptArg(digitalPinToInterrupt(this->pin), &INTERRUPTS::__CallbackIT, this, triggerIT);
    
    /// Flag di inizializzazione IT basso
    __isInterruptAttached = 1;
  }
}

/**
 *  @brief Sospende momentaneamente l'Interrupt Service Routine
 */
void INTERRUPTS::suspendISR()
{
  if(__isInterruptAttached)
  {
    TaskIT.Suspend();
    detachInterrupt(this->pin);

    /// Flag di inizializzazione IT basso
    __isInterruptAttached = 0;
  }
}

/**
 *  @brief cambia il pin di interrupt e fa l'attach di quello nuovo e il detach di quello vecchio
 * 
 *  @param pin                      : Hardware Pin in cui verrà eseguito l'Input Interrupt 
 *  @param modePin                  : INPUT, INPUT_PULLUP, INPUT_PULLDOWN  sono le possibili modalità di input del pin
 *  @param IT_TriggerMode           : RISING, FALLING, CHANGE, HIGH, LOW   sono le modalità di Interrupt

 */
void INTERRUPTS::changePinIT(uint8_t pin, uint8_t modePin, uint8_t IT_TriggerMode)
{
  if(pin != this->pin || modePin != this->inputMode || IT_TriggerMode != this->triggerIT)
  {
    if(__isInterruptAttached)
      detachInterrupt(this->pin);

    this->pin = pin;
    this->inputMode = modePin;
    this->triggerIT = IT_TriggerMode;

    /// Modalità di ingresso del pin di Interrupt
    pinMode(this->pin, this->inputMode);
    attachInterruptArg(digitalPinToInterrupt(this->pin), &INTERRUPTS::__CallbackIT, this, triggerIT);

    /// Flag di inizializzazione IT alto
    __isInterruptAttached = 1;
  }
}

/**
 *  @brief Restituisce l'Handler della task che gestisce l'Interrupt Service Routine
 * 
 *  @return dell'Handler della task dell'ISR, utile per creare, distruggere, modificare una task
 */
TaskHandle_t INTERRUPTS::getHandlerIT()
{
  return TaskIT.getHandler();
}

/// Metodo che cambia dinamicamente la funzione gestita nell'ISR
void INTERRUPTS::setISR(void (*UserISR)())
{
  /// Si assicura che non ci sia il metodo di loop per i metodi esterni
  __MethodLoopIT = nullptr;

  /// Salva dinamicamente la funzione
  __LoopIT = UserISR;
}

/// Metodo che cambia dinamicamente il metodo gestito nell'ISR
template <class refClass>
void INTERRUPTS::setISR(refClass* thisPointer, void (refClass::*methodToExecute)())
{
  /// Si assicura che non ci sia la funzione di loop per le normali funzioni
  __LoopIT = nullptr;
  
  /// struttura:  __FunzioneWrap = [cattura della copia delle variabili] (tipo di dato dei parametri della funzione) { if(EsisteIlMetodoUtente == true) PuntatoreAlMetodoUtente; };
  __MethodLoopIT = [thisPointer, methodToExecute] (void) { if(methodToExecute) (thisPointer->*methodToExecute)(); }; 
}


/*---------------------------------------------
|                                             |
|            CLASS PRIVATE METHODS            |          
|                                             |
---------------------------------------------*/

/**
 *  @brief Callback del generico Interrupt che è stato invocato
 */
void IRAM_ATTR INTERRUPTS::__CallbackIT(void *thisPointer)
{
  INTERRUPTS *Instance = static_cast<INTERRUPTS *>(thisPointer);

  /// Se diventa pdTRUE passa subito alla task se non ci sono task più prioritarie di questa
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  //LogInfoISR("ISR", "Sending A Notification of Interrupt");

  /// Crea la notifica dell'interrupt
  vTaskNotifyGiveFromISR(Instance->TaskHandler, &xHigherPriorityTaskWoken);

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 *  @brief Wrappa il programma dell'Utente e ciò che attende la notifica.
 *         Questo metodo viene passato nel loop della task
 */
void INTERRUPTS::__wrapperTaskLoopIT()
{
  //LogInfoISR("ISR", "Waiting To Receive A Notification from Interrupt");

  /// Aspetta la notifica all' "infinito" in questa task finchè non arriva
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

  /// Esegue il codice dell'Utente

  //LogInfoISR("ISR", "Received A Notification from Interrupt");

  /// Esegue la funzione se esiste
  if(__LoopIT)
    __LoopIT();
  // Oppure chiama ed esegue il metodo, se c'è
  else if(__MethodLoopIT)
    __MethodLoopIT();
}










