#include "Tasks.hpp"




/*-----------------------------------------------
  |                                             |
  |             CLASS PUBLIC METHODS            |          
  |                                             |
  -----------------------------------------------*/


TaskTypeDef::TaskTypeDef()
{

}



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
    #ifdef LOG_ACTIVE_TASK
      LogError("Task Creation", "Controllare questi parametri della TASK %s : Heap=%zu    Priorità=%d\n\n", name, HeapSize, priority);
    #endif
    this->status = pdFAIL;
  }
  else
  {
    /// ogni task viene schedulata ed eseguita su core 1 (APP_CPU_NUM), perchè se no va in crash
    this->status = xTaskCreatePinnedToCore(TaskTypeDef::ClassInternalUsage_Task, this->name, this->HeapSize, this, this->priority, &this->__TaskHandler, APP_CPU_NUM); 
  }

  
  #ifdef LOG_ACTIVE_TASK
    if(this->status == pdPASS) 
      LogInfo("TaskCreate", "Riuscita la creazione della task: %s\n", this->name);
    else  
      LogError("TaskCreate", "Errore nella creazione della task: %s\n", this->name);

    LogInfo("TaskCreate Info Heap", "Heap ancora libero : %zu byte su %zu byte   e   Heap totale usato : %zu\n\n\n", ESP.getFreeHeap(), ESP.getHeapSize(), ESP.getHeapSize() - ESP.getFreeHeap());
  #endif

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
  setTask(taskSetupFunction, taskLoopFunction);
  return Init(name, HeapSize, pvParameters, priority, delay_ms);
}



/**
 *  @brief Metodo che permette di cambiare il tempo tra un ciclo loop e un altro della task anche dopo l'inizializzazione
 *
 *  @param delayMillis è il tempo in millisecondi tra un ciclo loop e l'altro, verrà convertito in ticks all'interno
 */
void TaskTypeDef::TaskWait(uint32_t delayMillis)
{
  this->delay_ms = delayMillis;
  delay_tick = pdMS_TO_TICKS(delayMillis);
}

/**
 *  @brief Metodo che permette di ottenere l'Handler della task per usarlo all'esterno della classe
 *
 *  @return dell'Handler della task, utile per creare, distruggere, modificare una task
 */
TaskHandle_t TaskTypeDef::getHandler()
{
  return __TaskHandler;
}

/**
 * @brief Setup e Loop delle funzioni che verranno eseguite dalla task interna @see ClassInternalUsage_Task della @class
 *
 * @warning : @param taskSetupFunction e @param taskLoopFunction NON DEVONO avere parametri, al massimo devono chiamare una funzione a loro volta in cui ci sono dei parametri
 */
void TaskTypeDef::setTask(void (*taskSetupFunction)(), void (*taskLoopFunction)())
{
  __TaskSetupNormale = taskSetupFunction; 
  __TaskLoopNormale = taskLoopFunction; 
  
  #ifdef LOG_ACTIVE_TASK
    if(__TaskSetupNormale == nullptr || __TaskLoopNormale == nullptr)
      LogError("Errore setTask", "Errore nel settaggio della task %s, TaskSetupClass e/o TaskLoopClass sono nullptr");
  #endif
}

/**
 * @brief Sospende momentaneamente la task, può essere riavviata con Reseume()
 */
void TaskTypeDef::Suspend()
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
void TaskTypeDef::Resume()
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
bool TaskTypeDef::isRunning()
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

  #ifdef LOG_ACTIVE_TASK
    LogInfo("ClassInternalUsage_Task", "Calling %s SetupTask", Instance->name);
  #endif

  Instance->TaskStart();
  
  /// Se esiste la funzione di Setup la esegue
  if(Instance->__TaskSetupNormale)
    Instance->__TaskSetupNormale();
  else if(Instance->__TaskSetupClass)  
    Instance->__TaskSetupClass();  /// A quanto pare vuole un parametro *void anche se non lo usa

  #ifdef LOG_ACTIVE_TASK
  else
    LogWarning(Instance->name, "Warning, non existing function or method while Calling %s SetupTask\nAborting further Task calls untill providing a loop function...\n\n\n", Instance->name);
  #endif

  /**
   * 
   *  @task_loop: 
   * 
   */

  #ifdef LOG_ACTIVE_TASK
    LogInfo("ClassInternalUsage_Task", "Calling %s LoopTask\n\n\n", Instance->name);/// se esiste la funzione di loop la esegue
  #endif
  if(Instance->__TaskLoopNormale)
    Instance->__TaskLoopNormale();
  else if(Instance->__TaskLoopClass)
    Instance->__TaskLoopClass(); /// A quanto pare vuole un parametro *void anche se non lo usa
  else
  {
    #ifdef LOG_ACTIVE_TASK
      LogError("ClassInternalUsage_Task", "Warning, non existing function or method while Calling %s SetupTask\nAborting further Task calls until providing a loop function...\n\n\n", Instance->name);
    #endif
    vTaskDelay(portMAX_DELAY);
  }

  #ifdef LOG_ACTIVE_TASK
    LogInfo("setup", "Creata la task \"%s\" - Free Stack Space : %dB", Instance->name, uxTaskGetStackHighWaterMark(NULL));
  #endif

  for(;;)
  {
    /// se esiste la funzione di loop la esegue
    if(Instance->__TaskLoopNormale)
      Instance->__TaskLoopNormale();
    else if(Instance->__TaskLoopClass)
      Instance->__TaskLoopClass(); /// A quanto pare vuole un parametro *void anche se non lo usa
    
    #ifdef LOG_ACTIVE_TASK
    else
      LogError("ClassInternalUsage_Task", "Warning, non existing function or method while Calling %s SetupTask\nAborting further Task calls untill providing a loop function...\n\n\n", Instance->name);
    #endif

    /// attende
    Instance->__TaskWait();
  }
  
  /// Se per qualsiasi ragione dovesse uscire elimina la task
  vTaskDelete(NULL);
}
