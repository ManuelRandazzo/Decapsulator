/**
 *
 * @brief FUNZIONI CON SPIEGAZIONI UN PO' PIU' PRECISE
 * 
 * @note Link alla documentazione ufficiale di FreeRTOS: https://aws.github.io/amazon-freertos/202107.00/index.html?
 *
 */

/*
#include <Arduino.h>

// ===================== TASK HANDLES ============================
// I TaskHandle_t servono per controllare (sospendere, riprendere, cancellare) i task
TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;
TaskHandle_t task3Handle = NULL;

// ===================== QUEUE ============================
// Una queue è una struttura FIFO usata per scambiare dati tra task
// Qui creiamo una coda che può contenere 10 elementi di tipo int
QueueHandle_t dataQueue;

// ===================== SEMAPHORE & MUTEX ============================
// Un binary semaphore è un flag (0 o 1), spesso usato per eventi o ISR
// Un mutex è simile ma serve per proteggere risorse condivise
SemaphoreHandle_t binarySemaphore;
SemaphoreHandle_t mutex;

// ===================== TASK 1 – LED BLINK ============================
// Dimostra uso di vTaskDelay per creare un lampeggio LED
void task1(void *pvParameters)
{
  
  //qui il codice passa solo una volta, utile per i setup delle task(è come se fosse un main)
  
  while(true) //loop della task (quasi non bloccante)
  {
    Serial.println("Task 1: LED ON");
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));  // Pausa di 500 ms (usiamo macro pdMS_TO_TICKS per convertire in tick)
    Serial.println("Task 1: LED OFF");
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// ===================== TASK 2 – PRODUCER ============================
// Questo task genera dati (un contatore) e li invia nella queue
void task2(void *pvParameters)
{
  //qui il codice passa solo una volta, utile per i setup delle task(è come se fosse un main)

  int count = 0;

  while(true) //loop della task (quasi non bloccante)
  {
    // xQueueSend(queue, &valore, timeout)
    if (xQueueSend(dataQueue, &count, portMAX_DELAY) == pdPASS)
      Serial.printf("Task 2: Sent %d to queue\n", count++);

    vTaskDelay(pdMS_TO_TICKS(1000));  // 1 secondo di attesa
  }
}

// ===================== TASK 3 – CONSUMER ============================
// Riceve dati dalla queue e li stampa in modo thread-safe grazie al mutex
void task3(void *pvParameters)
{
  //qui il codice passa solo una volta, utile per i setup delle task(è come se fosse un main)

  int value;
  while(true) //loop della task (quasi non bloccante)
  {

    // xQueueReceive(queue, &destinazione, timeout)
    if (xQueueReceive(dataQueue, &value, portMAX_DELAY))
      // xSemaphoreTake(mutex, timeout): blocca finché il mutex è libero
      if (xSemaphoreTake(mutex, portMAX_DELAY))
      {
        Serial.printf("Task 3: Received %d from queue\n", value);
        xSemaphoreGive(mutex);  // Libera il mutex
      }

  }
}

// ===================== ISR SIMULATA ============================
// Simula un interrupt che segnala un evento tramite binary semaphore
// In un vero uso, sostituire con ISR reale
void IRAM_ATTR fakeISR()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // xSemaphoreGiveFromISR() notifica che l’evento è accaduto (da ISR)
  xSemaphoreGiveFromISR(binarySemaphore, &xHigherPriorityTaskWoken);

  // Fa il context switch immediato se serve
  portYIELD_FROM_ISR();
}

// ===================== SETUP ============================
void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // === CREAZIONE QUEUE ===
  // xQueueCreate(numero_elementi, dimensione_elemento_in_byte)
  dataQueue = xQueueCreate(10, sizeof(int));

  // === CREAZIONE SEMAPHORI ===
  binarySemaphore = xSemaphoreCreateBinary(); // "flag" 0/1
  mutex = xSemaphoreCreateMutex();            // protegge risorse condivise

  // === CREAZIONE TASK ===
  // xTaskCreatePinnedToCore(funz, "nome", stackSize, param, priorità, &handle, core) -> "nome" può essere stampato per sapere i thread attualmente in run
  // priorità: da 0 (bassa) a max (configMAX_PRIORITIES)
  // core: 0 = PRO_CPU, 1 = APP_CPU (ESP32 ha 2 core)
  xTaskCreatePinnedToCore(task1, "LED Task", 2048, NULL, 1, &task1Handle, 0);
  xTaskCreatePinnedToCore(task2, "Producer Task", 2048, NULL, 1, &task2Handle, 1);
  xTaskCreatePinnedToCore(task3, "Consumer Task", 2048, NULL, 1, &task3Handle, 0);

  // Simula un interrupt usando un timer
  timerAttachInterrupt(timerBegin(0, 80, true), &fakeISR, true);
}

// ===================== LOOP ============================
// Attende evento dal semaforo (triggerato da ISR)
void loop()
{
  // xSemaphoreTake(semaforo, timeout)
  if (xSemaphoreTake(binarySemaphore, pdMS_TO_TICKS(100)))
    Serial.println("Loop: ISR event received!");

  vTaskDelay(pdMS_TO_TICKS(100));  // piccolo delay per evitare loop serrato
}
//*/



/**
 *
 * @brief STESSO CODICE, MA CON FUNZIONI FATTE COME DOVREMMO FARE NOI (non servono i brief per le variabili)
 * 
 * @note Link alla documentazione ufficiale di FreeRTOS: https://aws.github.io/amazon-freertos/202107.00/index.html?
 *
 */


#include <Arduino.h>

// ===================== TASK HANDLES ============================

/**
 * @brief Handle per il controllo del Task 1 (LED)
 */
TaskHandle_t task1Handle = NULL;

/**
 * @brief Handle per il Task 2 (Producer)
 */
TaskHandle_t task2Handle = NULL;

/**
 * @brief Handle per il Task 3 (Consumer)
 */
TaskHandle_t task3Handle = NULL;

// ===================== QUEUE ============================

/**
 * @brief Coda condivisa tra Producer e Consumer
 * Contiene elementi di tipo int (max 10)
 */
QueueHandle_t dataQueue;

// ===================== SEMAPHORE & MUTEX ============================

/**
 * @brief Binary Semaphore usato per notifiche asincrone (es. ISR)
 */
SemaphoreHandle_t binarySemaphore;

/**
 * @brief Mutex per accesso sicuro alla Serial (thread-safe)
 */
SemaphoreHandle_t mutex;

// ===================== TASK 1 – LED BLINK ============================

/**
 * @brief Task che gestisce il lampeggio del LED di bordo
 * 
 * @param pvParameters Parametri passati alla task (non usati)
 */
///*
void task1(void *pvParameters)
{
  while (true)
  {
    Serial.println("Task 1: LED ON");
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));

    Serial.println("Task 1: LED OFF");
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
//*/

// ===================== TASK 2 – PRODUCER ============================

/**
 * @brief Task che genera numeri e li invia alla queue
 * 
 * @param pvParameters Parametri passati alla task (non usati)
 */
///*
void task2(void *pvParameters)
{
  int count = 0;

  while (true)
  {
    // Invia count nella queue, attende se piena (portMAX_DELAY)
    if (xQueueSend(dataQueue, &count, portMAX_DELAY) == pdPASS)
      Serial.printf("Task 2: Sent %d to queue\n", count++);

    vTaskDelay(pdMS_TO_TICKS(1000));  // Attende 1 secondo
  }
}
//*/

// ===================== TASK 3 – CONSUMER ============================

/**
 * @brief Task che riceve valori dalla queue e li stampa usando mutex
 * 
 * @param pvParameters Parametri passati alla task (non usati)
 */
///* 
void task3(void *pvParameters)
{
  int value;

  while (true)
  {
    if (xQueueReceive(dataQueue, &value, portMAX_DELAY))
    {
      if (xSemaphoreTake(mutex, portMAX_DELAY))
      {
        Serial.printf("Task 3: Received %d from queue\n", value);
        xSemaphoreGive(mutex);
      }
    }
  }
}
//*/

// ===================== ISR SIMULATA ============================

/**
 * @brief Simula un interrupt e rilascia il binary semaphore
 * 
 * @note Questa funzione può essere collegata a un vero ISR
 */
///*
void IRAM_ATTR fakeISR()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xSemaphoreGiveFromISR(binarySemaphore, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(); // Permette context switch se necessario
}
//*/

// ===================== SETUP ============================

/**
 * @brief Setup iniziale: crea task, semafori, queue e configura interrupt
 */
///*
void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Creazione della queue (10 elementi int)
  dataQueue = xQueueCreate(10, sizeof(int));

  // Creazione di un binary semaphore e di un mutex
  binarySemaphore = xSemaphoreCreateBinary();
  mutex = xSemaphoreCreateMutex();

  // Creazione dei task (funzione, nome, stack, param, priorità, handle, core)
  xTaskCreatePinnedToCore(task1, "LED Task", 2048, NULL, 1, &task1Handle, 0);
  xTaskCreatePinnedToCore(task2, "Producer Task", 2048, NULL, 1, &task2Handle, 1);
  xTaskCreatePinnedToCore(task3, "Consumer Task", 2048, NULL, 1, &task3Handle, 0);

  // Collegamento fittizio di ISR a un timer (simulazione)
  timerAttachInterrupt(timerBegin(0, 80, true), &fakeISR, true);
}
//*/

// ===================== LOOP ============================

/**
 * @brief Loop principale: controlla se c'è una notifica dal semaforo
 */
///*
void loop()
{
  if (xSemaphoreTake(binarySemaphore, pdMS_TO_TICKS(100)))
    Serial.println("Loop: ISR event received!");

  vTaskDelay(pdMS_TO_TICKS(100));  // Delay anti-loop serrato
}
//*/
