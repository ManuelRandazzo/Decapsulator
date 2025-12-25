/**
 *    @brief FILE CONTENENTE LE DEFINIZIONI E LE ENUMERAZIONI DELLE TASKS
 */

#pragma once /// Mi permette di non avere errori di multiple ridefinizioni nei file che le usano che magari vengono inclusi a loro volta in altri file che usano questo file

/// @link https://learn.microsoft.com/it-it/cpp/standard-library/functional?view=msvc-170
#include <functional>

/// Per i Log
#include "Debug.hpp"

/**
 *  @brief enum dell'Heap occupato dalle singole tasks (RAM del runtime)
 *
 *  @note heap occupato realmente --> realHeap = taskHeapSize * 4   (ex. 2048 * 4 = 8196 = 8KB)
 *  @note cercare di non occupare troppo spazio dell'Heap. Utilizzare esp_get_free_heap_size() 
 *        per sapere l'heap rimanente e uxTaskGetStackHighWaterMark(NULL) per sapere quello della task in cui viene chiamato. 
 *        MAX 512KB
 *
 *  ATTENZIONE: @stack_overflow: "Debug exception reason: Stack canary watchpoint triggered (task X) " nella console indica uno stack overflow della task X, aumentare lo stack
 *
 *  
 *  @determinazione_heap:
 *                        1. Aggiungi una funziona che restituisce l'Heap rimanente.
 *
 *                        2. Esegui la task in condizioni realistiche.
 *
 *                        3. Monitora quanta heap rimane con la funzione messa prima.
 *
 *                        4. Aggiungi un margine di sicurezza +20~50%.
 * 
 */
enum taskHeapSize_t
{
  Security_heap   = 8192,
  OTA_heap        = 8192,
  MainPrg_heap    = 8192,
  PesoCont_heap   = 512,
  PesoCaffe_heap  = 512, 
};

/**
 *  @brief enum delle priorità delle singole tasks
 *
 *  @note più alto è il valore più la priorità è alta
 *  @note cercare di non fare troppe task con la stessa priorità (anche se vengono ugualmente eseguite)
 */
enum taskPriority_t
{
  Security_priority   = configMAX_PRIORITIES - 1, //PRIORITA' MASSIMA SEMPRE, il massimo è 24
  OTA_priority        = 20,
  MainPrg_priority    = 4,
  PesoCont_priority   = 3,
  PesoCaffe_priority  = 3, 
};

/**
 *  @brief enum dei delay delle singole tasks
 *
 *  @note cercare di non fare troppi overlap tra le tasks (errore di concorrenza)
 */
enum taskDelays_t
{
  Security_delay   = 5,
  MainPrg_delay    = 10,
  OTA_delay        = 15,
};


class TaskTypeDef
{
  public :

    ///constructor
    TaskTypeDef() {};

    /// destructor
    ~TaskTypeDef();

    /// Inizializza la funzione creandola o loggando gli errori
    BaseType_t Init(const char *name, const uint32_t HeapSize, void *pvParameters, UBaseType_t priority, uint32_t delay_ms = 0);
    
    /// Inizializza la funzione creandola, setta anche il setup e il loop delle tasks o loggando gli errori
    BaseType_t Init(const char *name, const uint32_t HeapSize, void *pvParameters, UBaseType_t priority, uint32_t delay_ms, void (*taskSetupFunction)(), void (*taskLoopFunction)());
    
    /// Inizializza la funzione creandola, setta anche il setup e il loop delle tasks o loggando gli errori
    template <class refClass>
    BaseType_t Init(const char *name, const uint32_t HeapSize, void *pvParameters, UBaseType_t priority, uint32_t delay_ms, refClass* thisPointer, void (refClass::*taskSetupMethod)(), void (refClass::*taskLoopMethod)());

    /// Modifica il tempo della task
    inline void TaskWait(uint32_t delayMillis);

    /// Modifica il tempo della task
    inline void TaskWaitMicros(uint32_t delayMicros);

    /// Funzione che restituisce l'handelr della task, utile per modificare o notificare qualcosa della task (per esempio nelle ISR)
    inline TaskHandle_t getHandler();

    /// Setup e Loop delle funzioni che verranno eseguite dalla task interna @see ClassInternalUsage_Task della @class
    inline void setTask(void (*taskSetupFunction)(), void (*taskLoopFunction)());

    /// Setup e Loop dei metodi che verrnno eseguiti dalla task interna @see ClassInternalUsage_Task della @class
    template <class refClass>
    inline void setTask(refClass* thisPointer, void (refClass::*taskSetupMethod)(), void (refClass::*taskLoopMethod)());

    /// Sospende momentaneamente la task, può essere riavviata con Reseume()
    inline void Suspend();

    /// Riprende la task, può essere sospesa con Suspend()
    inline void Resume();

    /// Restituisce se la task può essere schedulata, può essere modificata da Resume() o da Suspend()
    inline bool isRunning();

  private :
    TaskHandle_t __TaskHandler = NULL;                  /*!<  Handler per la dichiarazione, creazione e distruzione di una task che all'inizio non punta a nulla (NULL)     */

    const char *name;                                   /*!<  Nome string della funzione che può essere loggato nella console in USART                                      */  

    uint32_t HeapSize;                                  /*!<  Quantità di stack riservata alla task. ABSOLUTE TOTAL MAX = 520KB                                             */

    void *pvParameters;                                 /*!<  Parametri destinati alla task passati come argomento                                                          */

    UBaseType_t priority;                               /*!<  Priorità che ha una task rispetto ad un'altra. MAX = configMAX_PRIORITIES                                     */

    BaseType_t status;                                  /*!<  Status dell'inizializzazione ---> pdPASS / pdFAIL                                                             */

    uint32_t delay_ms = 10;                             /*!<  Delay in millisecondi per la task, default sono 10ms                                                          */

    TickType_t delay_tick = pdMS_TO_TICKS(delay_ms);    /*!<  Delay in ticks per la task                                                                                    */

    TickType_t getLastTick;                             /*!<  Acquisisce il tick di partenza, questo serve per avere un tempo iniziale di riferimento                       */

    void (*__TaskSetupNormale)();                       /*!<  Questa funzione di callback esegue il codice aggiuntivo dell'utente per il setup della task per le funzioni   */

    void (*__TaskLoopNormale)();                        /*!<  Questa funzione di callback esegue il codice aggiuntivo dell'utente per il setup della task per le funzioni   */

    std::function<void()> __TaskSetupClass = nullptr;   /*!<  Questa funzione di callback esegue il codice aggiuntivo dell'utente per il setup della task per le class      */

    std::function<void()> __TaskLoopClass = nullptr;    /*!<  Questa funzione di callback esegue il codice aggiuntivo dell'utente per il loop della task per le class       */

    unsigned __isRunning : 1;                           /*!<  Questa variabile indica se la task è sospesa (__isRunning = false) o sta attualmente andando (__isRunning = true)  */


    /// funzione che deve essere messa prima del for(;;) o del while(1)
    /// per fare il setup della task e avviare il tempo di esecuzione corretto
    inline void TaskStart() { getLastTick = xTaskGetTickCount(); };

    /// funzione che determina il tempo che avrà la task, normalmente il delay è acquisito dall'inizializzazione
    /// altrimenti, cambiando il delay, verrà aggiornato il delay relativo della task
    inline void __TaskWait() { if(this->delay_tick > 0) xTaskDelayUntil(&this->getLastTick, this->delay_tick); } //ogni Xms esatti (no drift temporali) viene eseguita questa task

    /// Class Internal Usage Task
    static void ClassInternalUsage_Task(void *pvParameters);
};



/*-----------------------------------------------
  |                                             |
  |             CLASS PUBLIC METHODS            |          
  |                                             |
  -----------------------------------------------*/



/// destructor
TaskTypeDef::~TaskTypeDef()
{
  if(__TaskHandler != NULL)
    vTaskDelete(__TaskHandler);
}



/**
 *  @brief funzione per cercare di creare una task restituendo un log in caso di errore
 *
 *  @param name permette all'utente di decidere un nome specifico con cui verrà fatto il log al momento dell'inizializzazione
 *  @param HeapSize definisce quanto heap può essere utilizzato dalla task
 *  @param pvParameters parametro che è passato alla task, questo parametro deve continuare ad esistere al momento della creazione della task
 *  @param priority priorità della task, più alta è e prima verrà eseguita in caso di task che concorrono per quel tempo
 *  @param delay_ms E' quanto tempo passa tra una chiamata e un'altra della task
 *  
 *  @return BaseType_t pdTRUE se andato a buon fine, pdFALSE se c'è stato un problema
 */  
BaseType_t TaskTypeDef::Init(const char *name, const uint32_t HeapSize, void *pvParameters, UBaseType_t priority, uint32_t delay_ms)
{
  this->name = name;
  this->HeapSize = HeapSize;
  this->pvParameters = pvParameters;
  this->priority = priority;
  this->delay_ms = delay_ms;
  this->delay_tick = pdMS_TO_TICKS(delay_ms);

  if(priority > configMAX_PRIORITIES || HeapSize == 0x00 || HeapSize == 0)
  {
    LogError("Task Creation", "Controllare questi parametri della TASK %s : Heap=%zu    Priorità=%d\n\n", name, HeapSize, priority);
    this->status = pdFAIL;
  }
  else
  {
    /// ogni task viene schedulata ed eseguita su core 1 (APP_CPU_NUM), perchè se no va in crash
    this->status = xTaskCreatePinnedToCore(TaskTypeDef::ClassInternalUsage_Task, this->name, this->HeapSize, this, this->priority, &this->__TaskHandler, APP_CPU_NUM); 
  }

  
  if(this->status == pdPASS) 
    LogInfo("TaskCreate", "Riuscita la creazione della task: %s\n", this->name);
  else  
    LogError("TaskCreate", "Errore nella creazione della task: %s\n", this->name);

  LogInfo("TaskCreate Info Heap", "Heap ancora libero : %zu byte su %zu byte   e   Heap totale usato : %zu\n\n\n", ESP.getFreeHeap(), ESP.getHeapSize(), ESP.getHeapSize() - ESP.getFreeHeap());
    

  if(status == pdPASS)
    __isRunning = 1;

  return status;
}


/**
 *  @brief funzione per cercare di creare una task restituendo un log in caso di errore
 *
 *  @param name permette all'utente di decidere un nome specifico con cui verrà fatto il log al momento dell'inizializzazione
 *  @param HeapSize definisce quanto heap può essere utilizzato dalla task
 *  @param pvParameters parametro che è passato alla task, questo parametro deve continuare ad esistere al momento della creazione della task
 *  @param priority priorità della task, più alta è e prima verrà eseguita in caso di task che concorrono per quel tempo
 *  @param delay_ms E' quanto tempo passa tra una chiamata e un'altra della task
 *  @param taskSetupFunction Questa è la funzione che viene eseguita (puntata) dalla task al momento del setup()
 *  @param taskLoopFunction Questa è la funzione che viene eseguita (puntata) dalla task al momento del loop()
 *  
 *  @return BaseType_t pdTRUE se andato a buon fine, pdFALSE se c'è stato un problema
 */ 
BaseType_t TaskTypeDef::Init(const char *name, const uint32_t HeapSize, void *pvParameters, UBaseType_t priority, uint32_t delay_ms, void (*taskSetupFunction)(), void (*taskLoopFunction)())
{
  LogInfo("Loop&Setup", "trying to setup the loop and setup tasks");
  setTask(taskSetupFunction, taskLoopFunction);
  LogInfo("Loop&Setup", "finished to setup the loop and setup tasks");
  return Init(name, HeapSize, pvParameters, priority, delay_ms);
}

/**
 *  @brief funzione per cercare di creare una task restituendo un log in caso di errore
 *
 *  @tparam refClass è il modello della @class che chiama il metodo Init templato
 *
 *  @param name permette all'utente di decidere un nome specifico con cui verrà fatto il log al momento dell'inizializzazione
 *  @param HeapSize definisce quanto heap può essere utilizzato dalla task
 *  @param pvParameters parametro che è passato alla task, questo parametro deve continuare ad esistere al momento della creazione della task
 *  @param priority priorità della task, più alta è e prima verrà eseguita in caso di task che concorrono per quel tempo
 *  @param delay_ms E' quanto tempo passa tra una chiamata e un'altra della task
 *
 *  @param thisPointer è il puntatore alla @class del modello refClass
 *  @param taskSetupMethod Questa è il metodo che viene eseguito (puntato) dalla task al momento del setup()
 *  @param taskLoopMethod Questa è il metodo che viene eseguito (puntato) dalla task al momento del loop()
 *  
 *  @return BaseType_t pdTRUE se andato a buon fine, pdFALSE se c'è stato un problema
 */
template <class refClass>
BaseType_t TaskTypeDef::Init(const char *name, const uint32_t HeapSize, void *pvParameters, UBaseType_t priority, uint32_t delay_ms, refClass* thisPointer, void (refClass::*taskSetupMethod)(), void (refClass::*taskLoopMethod)())
{
  setTask<refClass>(thisPointer, taskSetupMethod, taskLoopMethod);
  return Init(name, HeapSize, pvParameters, priority, delay_ms);
}

/**
 *  @brief Metodo che permette di cambiare il tempo tra un ciclo loop e un altro della task anche dopo l'inizializzazione
 *
 *  @param delayMillis è il tempo in millisecondi tra un ciclo loop e l'altro, verrà convertito in ticks all'interno
 */
inline void TaskTypeDef::TaskWait(uint32_t delayMillis)
{
  this->delay_ms = delayMillis;
  delay_tick = pdMS_TO_TICKS(delayMillis);
}

/**
 *  @brief Metodo che permette di ottenere l'Handler della task per usarlo all'esterno della classe
 *
 *  @return dell'Handler della task, utile per creare, distruggere, modificare una task
 */
inline TaskHandle_t TaskTypeDef::getHandler()
{
  return __TaskHandler;
}

/**
 * @brief Setup e Loop delle funzioni che verranno eseguite dalla task interna @see ClassInternalUsage_Task della @class
 *
 * @warning : @param taskSetupFunction e @param taskLoopFunction NON DEVONO avere parametri, al massimo devono chiamare una funzione a loro volta in cui ci sono dei parametri
 */
inline void TaskTypeDef::setTask(void (*taskSetupFunction)(), void (*taskLoopFunction)())
{
  __TaskSetupNormale = taskSetupFunction; 
  __TaskLoopNormale = taskLoopFunction; 
  
  if(__TaskSetupNormale == nullptr || __TaskLoopNormale == nullptr)
    LogError("Errore setTask", "Errore nel settaggio della task %s, TaskSetupClass e/o TaskLoopClass sono nullptr");
}

/**
 *  @brief Setup e Loop dei metodi che verrnno eseguiti dalla task interna @see ClassInternalUsage_Task della @class
 *
 *  @warning taskLoopMethod NON DEVE avere parametri, al massimo deve chiamare una funzione a sua volta in cui ci sono dei parametri
 *
 *  @tparam refClass è il template, ovvero il modello, che dovrebbe essere la classe in cui viene utilizzata
 *
 *  @param thisPointer è l'istanza della classe. E' in generale il pointer-to-object
 *  @param taskLoopMethod è il metodo che passa la classe figlia che viene eseguita in loop nella task
 *  @param taskSetupMethod è il metodo che passa la classe figlia che viene eseguita nel setup della task
 */
template <class refClass>
inline void TaskTypeDef::setTask(refClass* thisPointer, void (refClass::*taskSetupMethod)(), void (refClass::*taskLoopMethod)()) 
{ 
  /// Viene creata una wrapper function, ovvvero una funzione che ne racchiude un'altra.
  /// struttura:    __FunzioneWrap = [cattura della copia delle variabili] (tipo di dato dei parametri della funzione) { if(EsisteLaFunzione == true) PuntatoreAllaFunzione; };
  __TaskSetupClass = [thisPointer, taskSetupMethod] (void) { if(taskSetupMethod) (thisPointer->*taskSetupMethod)(); };
  __TaskLoopClass = [thisPointer, taskLoopMethod] (void) { if(taskLoopMethod) (thisPointer->*taskLoopMethod)(); }; 

  if(__TaskSetupClass == nullptr || __TaskLoopClass == nullptr)
    LogError("Errore setTask", "Errore nel settaggio della task %s, TaskSetupClass e/o TaskLoopClass sono nullptr");
}

/**
 * @brief Sospende momentaneamente la task, può essere riavviata con Reseume()
 */
inline void TaskTypeDef::Suspend()
{
  if(__isRunning)
  {
    vTaskSuspend(__TaskHandler);
    __isRunning = 0;
  }
}

/**
 * @brief Riprende la task, può essere sospesa con Suspend()
 */
inline void TaskTypeDef::Resume()
{
  if(!__isRunning)
  {
    vTaskResume(__TaskHandler);
    __isRunning = 1;
  }
}

/**
 * @brief Restituisce se la task è stata modificata da Resume() o da Suspend() e se
 *        restituisce @true: la task è libera di essere schedulata @false: la task NON può essere schedulata
 */
inline bool TaskTypeDef::isRunning()
{
  return __isRunning;
}


/*-----------------------------------------------
  |                                             |
  |             CLASS PRIVATE METHODS           |          
  |                                             |
  -----------------------------------------------*/

/**
 *  @brief Class Internal Usage Task
 *
 *  @details sarebbe la task che viene schedulata ed eseguita, la quale è all'interno della
 *           classe per aiutare nella stesura del codice ed evitare troppe ripetizioni
 *
 *  @param pvParameters sarebbe da passare il puntatore della classe "this" per poter usare le stesse variabili e metodi della classe figlia
 */
void TaskTypeDef::ClassInternalUsage_Task(void *pvParameters)
{


  /// Crea un'istanza che punta all'oggetto attuale della classe "this" il quale è passato come pvParameters, il cast serve per poter usare tutti i metodi e tutte le variabili della classe
  /// Questa operazione è necessaria perchè non si possono passare le funzioni di una classe quando si crea una task per colpa del puntatore "this->"
  TaskTypeDef *Instance = static_cast<TaskTypeDef *>(pvParameters);
  
  /**
   * 
   *  @task_setup: 
   * 
   */

  LogInfo("ClassInternalUsage_Task", "Calling %s SetupTask", Instance->name);

  Instance->TaskStart();
  
  /// Se esiste la funzione di Setup la esegue
  if(Instance->__TaskSetupNormale)
    Instance->__TaskSetupNormale();
  else if(Instance->__TaskSetupClass)  
    Instance->__TaskSetupClass();  /// A quanto pare vuole un parametro *void anche se non lo usa
  else
    LogWarning(Instance->name, "Warning, non existing function or method while Calling %s SetupTask\nAborting further Task calls untill providing a loop function...\n\n\n", Instance->name);

  /**
   * 
   *  @task_loop: 
   * 
   */

  LogInfo("ClassInternalUsage_Task", "Calling %s LoopTask\n\n\n", Instance->name);/// se esiste la funzione di loop la esegue
  if(Instance->__TaskLoopNormale)
    Instance->__TaskLoopNormale();
  else if(Instance->__TaskLoopClass)
    Instance->__TaskLoopClass(); /// A quanto pare vuole un parametro *void anche se non lo usa
  else
  {
    LogError("ClassInternalUsage_Task", "Warning, non existing function or method while Calling %s SetupTask\nAborting further Task calls untill providing a loop function...\n\n\n", Instance->name);
    return;
  }

  for(;;)
  {
    /// se esiste la funzione di loop la esegue
    if(Instance->__TaskLoopNormale)
      Instance->__TaskLoopNormale();
    else if(Instance->__TaskLoopClass)
      Instance->__TaskLoopClass(); /// A quanto pare vuole un parametro *void anche se non lo usa
    else
      LogError("ClassInternalUsage_Task", "Warning, non existing function or method while Calling %s SetupTask\nAborting further Task calls untill providing a loop function...\n\n\n", Instance->name);
    
    /// attende
    Instance->__TaskWait();
  }
}
