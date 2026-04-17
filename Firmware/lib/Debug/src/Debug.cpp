#include "Debug.hpp"

SemaphoreHandle_t xSemaphoreLogger;
QueueHandle_t LoggerQueueHandler = nullptr;

#if !defined(WiFi_ACTIVE)
  #pragma message ("Warning: OTA DOWNLOAD E MQTT LOG NON DISPONIBILI --> WiFi Disattivato nelle impostazioni")
#endif


/**
 *  @brief Inizializza il WiFi
 *
 *  @note se è già inizializzato non esegue nulla
 */
void startWiFi(uint32_t timeout_for_each_initialization_ms)
{
  /// Debug
  uint32_t startTime = millis();


  #if defined(LOG_ACTIVE) && defined(LOG_ACTIVE_MQTT)
    /// Avvio del broker MQTT sulla porta 1883
    mqttClient.enableDebuggingMessages();

    /// Aggiungere Username e password contenuti in @file WiFi_secrets.hpp
    mqttClient.setURI(MQTT_BROKER); 
    mqttClient.enableLastWillMessage("brokerStatus", "offline");
    mqttClient.setKeepAlive(30);
  #endif


  WiFi.mode(WIFI_STA); //per evitare conflitti con le risorse del bluetooth
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setHostname("Decapsulator");
  LogInfo("WiFi Connect", "Connessione Wi-Fi");

  /// Timeout che se supera un certo tempo riavvia l'ESP
  uint32_t ESP_timeoutRestart = millis();
  while(WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(pdMS_TO_TICKS(100));
    /// Se sfora il timeout allora restarta l'ESP32
    if(millis() - ESP_timeoutRestart > DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS)
    {
      /// Se non è definito il NO_ESP_RESTART_ON_CONNECTION_FAILURE allora rebootta
      #ifndef NO_ESP_RESTART_ON_CONNECTION_FAILURE
        LogError("WiFi Connect", "CONNECTION ERROR TIMEOUT.\nRebooting...");
        ESP.restart();
      #else
        LogError("WiFi Connect", "CONNECTION ERROR TIMEOUT.\nProceeding with program...");
        /// Debug
        uint32_t endTime = millis();
        LogDebug("Tempo StartUp", "Delta Time WiFi Startup : %d", endTime - startTime);
        return;
      #endif
    }
  }    
    
    
  #if defined(LOG_ACTIVE) && defined(LOG_ACTIVE_MQTT)
    /// Timeout che se supera un certo tempo riavvia l'ESP
    mqttClient.loopStart();
    ESP_timeoutRestart = millis();
    while(!mqttClient.isConnected())
    {
      vTaskDelay(pdMS_TO_TICKS(100));
      /// Se sfora il timeout allora restarta l'ESP32
      if(millis() - ESP_timeoutRestart > DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS)
      {
        /// Se non è definito il NO_ESP_RESTART_ON_CONNECTION_FAILURE allora rebootta
        #ifndef NO_ESP_RESTART_ON_CONNECTION_FAILURE
          LogError("MQTT Client Connect", "CONNECTION ERROR TIMEOUT.\nRebooting...");
          ESP.restart();
        #else
          LogError("MQTT Client Connect", "CONNECTION ERROR TIMEOUT.\nProceeding with program...");
          /// Debug
          uint32_t endTime = millis();
          LogDebug("Tempo StartUp", "Delta Time WiFi Startup : %d", endTime - startTime);
          return;
        #endif
      }

    }
    
    isMqttConnected = true;

    /// Printa info sull'IP del dispositivo e sul broker MQTT in cui si è connessi
    LogInfo("WiFi Connect", "\nWiFi connesso: %s\nBroker MQTT: %s", WiFi.localIP().toString().c_str(), MQTT_BROKER);
  #else
    /// Printa info sull'IP del dispositivo
    LogInfo("WiFi Connect", "\nWiFi connesso: %s", WiFi.localIP().toString().c_str());  
  #endif
   
  /// Debug
  uint32_t endTime = millis();
  LogDebug("Tempo StartUp", "Delta Time WiFi Startup : %d", endTime - startTime);
}





/**
 *  @brief Inizializza sia il logger (in preprocessor) che il WiFi (runtime)
 *
 *  @param timeout_for_each_initialization_ms è il timeout che ogni 
 *         inizializzazione (ex. WiFi.begin(), Insights.begin()), deve rispettare.
 *         
 *  @note il timeout totale della funzione è derivato dalla moltiplicazione tra 
 *        @param timeout_for_each_initialization_ms e il numero di inizializzazioni
 *        che devono essere eseguite: ex. n_inizializzazioni = WiFi.begin() = 2, fTimeoutTot = n_inizializzazioni * timeout = 2*timeout
 *
 */
void LogBegin(uint32_t timeout_for_each_initialization_ms)
{
  #ifdef LOG_ACTIVE
    /// Limita il baud al massimo consentito per la Serial port di Arduino IDE
    Serial.begin(115200); 
    
    /// Permette i log dalla WiFi Library
    Serial.setDebugOutput(true);    


    //LoggerQueueHandler = xQueueCreate(LOGGER_QUEUE_LEN, sizeof(log_msg_t));
    xSemaphoreLogger = xSemaphoreCreateMutex();

    if(xSemaphoreLogger == NULL /*|| LoggerQueueHandler == NULL*/)
    {
      Serial.print("Errore nella creazione del Semaforo del logger, riavvio in...3");
      delay(1000);
      Serial.print("...2");
      delay(1000);
      Serial.println("...1\n\n");
      delay(1000);
      ESP.restart();
    }

    #ifdef WiFi_ACTIVE
      /// Inizializza il WiFi @ref @file "WiFi_secrets.hpp"
      startWiFi(timeout_for_each_initialization_ms);
    #else
      /// Delay per aspettare che la seriale sia inizializzata
      vTaskDelay(pdMS_TO_TICKS(2500));
    #endif
  #endif
}




/**
 *
 *    OVERRIDE SECTION
 *
 */


/// @link alle funzioni std:: usate --> https://en.cppreference.com/w/cpp/string/basic_string.html @sezione: numeric conversion
template<typename varType>
static varType FromStringToVarType(const std::string &s)
{    
  if constexpr (is_same_v<varType, string>)
    return s;
  if constexpr (is_same_v<varType, String>)
    return String(s.c_str());
  else if constexpr (is_same_v<varType, bool>)
  {
    string boolVal = s;
    /// No case sensitive ---> @link soluzione di tolower: https://stackoverflow.com/questions/313970/how-to-convert-an-instance-of-stdstring-to-lower-case
    transform(boolVal.begin(), boolVal.end(), boolVal.begin(), [](unsigned char c){ return tolower(c); });
    if(boolVal == "1" || boolVal == "true")  return true;
    if(boolVal == "0" || boolVal == "false") return false;
    // se la stringa non rappresenta un bool...
    return stoll(boolVal) != 0;
  } 
  else if constexpr (is_integral_v<varType> && is_signed_v<varType>)
  {
    /// Usa stoll per qualsiasi signed int
    long long v = stoll(s);        
    return static_cast<varType>(v);
  }
  else if constexpr (is_integral_v<varType> && is_unsigned_v<varType>)
  {
    /// Usa stoll per qualsiasi unsigned int
    unsigned long long v = stoull(s); // usa stoull per unsigned
    return static_cast<varType>(v);
  }
  else if constexpr (is_floating_point_v<varType>)
  {
    /// Usa stoll per qualsiasi floating point (float o double)
    long double v = stold(s);     // usa stold per float/double
    return static_cast<varType>(v);
  }

  return varType{};
}


/**
 *    MQTT Library Custom Functions
 */


#if defined(LOG_ACTIVE) && defined(LOG_ACTIVE_MQTT)

  ESP32MQTTClient mqttClient; // Oggetto di tipo Client MQTT

  /// Funzioni aggiuntive dell'MQTT Client
  void onMqttConnect(esp_mqtt_client_handle_t client){}

  void printError(esp_mqtt_error_codes_t *error_handle)
  {
    LogError("MQTT error handler", "Errore MQTT Handler : %d", error_handle);
  }

  /// In base alla versione installata di ESP IDF sceglie tra una o l'altra
  #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    esp_err_t handleMQTT(esp_mqtt_event_handle_t event)
    {
        mqttClient.onEventCallback(event);
        return ESP_OK;
    }
  #else  // IDF CHECK
    void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
    {
        auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
        mqttClient.onEventCallback(event);
    }
  #endif

  /// Funzione di formattazione e log MQTT
  void mqtt_logger_printf(string& topic, const char* format, ...)
  {
    static char loc_buf[5];
    char* temp = loc_buf;
    va_list args;
    va_start(args, format);
    
    /// Otttiene la lunghezza
    va_list copy;
    va_copy(copy, args);
    uint32_t len = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    if(len >= sizeof(loc_buf))
    {
      /// Alloca della memoria in runtime se il buffer è troppo piccolo
      temp = (char*)malloc(len+1);
      /// Se l'allocazione non è andata a buon fine ritorna
      if(temp == NULL)
      {
        va_end(args);
        return;
      }
    }

    /// Formatta la stringa passata e gli argomenti
    vsnprintf(temp, len+1, format, args);
    va_end(args);
    
    /// Pubblica il log sul topic
    mqttClient.publish(topic, temp, 0, false);
    
    /// Libera la memoria eventualmente allocata
    if(len >= sizeof(loc_buf))
      free(temp);
  }



#else /// Definisce altrimenti una funzione vuota
  #define mqtt_logger_printf(topic, format, ...) (void)0
#endif




/**
 * 
 * @example di come dovrà essere __DECAPSULATOR_LOG
 * 
 * #define __DECAPSULATOR_LOG(logType, tag, format, ...)\
 *   do\
 *   {\
 *     log_msg_t msg;\
 *     snprintf(msg.message, sizeof(msg.message), _FORMAT_(logType, format, tag), ##__VA_ARGS__);\
 *     _TOPIC_MQTT_(logType ## _LOG_TOPIC, tag);\
 *     msg.topic = topic;\
 *     xQueueSend(LoggerQueueHandler, &msg, 0);\
 *   } while(0)
 * 
 * 
 * #define __ISR_DECAPSULATOR_LOG(logType, tag, format, ...)\
 *    do\
 *    {\
 *       ets_printf(ARDUHAL_LOG_FORMAT(## logType, format), ##__VA_ARGS__);\
 *    } while(0)
 */

void LoggerTask(void* pvParameters)
{  
  while(1)
  {
    log_msg_t to_log;

    /// Attende all'infinito che qualcuno invii un log
    xQueueReceive(LoggerQueueHandler, &to_log, portMAX_DELAY);
    
    /// Printa il messaggio
    log_printf(to_log.message);

    #ifdef LOG_MQTT_ACTIVE
      if(isMqttConnected)
        mqtt_logger_printf(to_log.mqttTopic, to_log.message);
    #endif

    /**
     * 
     *  @todo Capire con Pisan per la parte di Diagnostica nel decapsulator per salvare i dati in SD
     *        sin da quando viene ricevuto il log o solo una volta quando viene spento oppure quando 
     *        xQueueReceice=false sfrutta il momento per inviare i log nel file
     * 
     */
  }
  vTaskDelete(NULL);
}




