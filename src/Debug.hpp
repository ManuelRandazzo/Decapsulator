/**
 * @file Debug.h
 * @brief ESP32-S3 Debug Logger con MQTT e/o Serial UART
 *
 * @details
 * Questo file implementa un logger per ESP32-S3 che invia messaggi di debug:
 * - tramite porta seriale UART la quale è thread-safe per le ESP32 (non ha bisogno di un'altra task)
 * - tramite MQTT usando la libreria thread-safe ESP32MQTTClient
 *
 * Il sistema è progettato per essere thread-safe, non bloccare il loop principale (o almeno poco overhead),
 * e può essere esteso per supportare anche SD card Log, o altre destinazioni.
 *
 * @authors   Randazzo Manuel, Pisan Alessio
 * @date      2025-05-24
 *
 */

#pragma once


/** ╔═════════════════════════════════════════════╗
    ║             USER: LOGGER OPTIONS            ║
    ╚═════════════════════════════════════════════╝ */

/// @info: Commentando questa riga si disattivano i LOG senza il bisogno di cancellarli nel programma
#define LOG_ACTIVE
/// @info: Commentando questa riga non vi saranno più log da parte del loop del motion. Tutti gli altri log del motion verranno scritti
#define LOG_ACTIVE_MOTION
/// @info: Commentando questa riga si disattivano i LOG MQTT senza il bisogno di cancellarli nel programma
#define LOG_MQTT_ACTIVE
/// @info: Scommentando questa riga si disattiva il restart dell'esp in caso di fail del wifi e/o dell'MQTT
#define NO_ESP_RESTART_ON_CONNECTION_FAILURE

/** ╔═════════════════════════════════════════════╗
    ║           USER: FUNCTION PROTOTIPES         ║
    ╚═════════════════════════════════════════════╝ */
/// @brief Inizializza sia il logger (in preprocessor) che il WiFi (runtime) e l'eventuale MQTT (runtime)
void LogBegin(uint32_t timeout_for_each_initialization_ms);

/// @brief Inizializza il WiFi (viene chiamato dalla funzione LogBegin)
void startWiFi(uint32_t timeout_for_each_initialization_ms);

/// @brief Modifica una variabile in runtime scrivendola in un determinato topic mqtt
/// @warning il topic sarà composto così: Decapsulator_Logger/DEBUG OVERRIDE/override_topic 
template <typename varType>
void DebugOverrideVar(String override_topic, varType* var);










/// File contenente ssid e la password dell'Utente
#include "WiFi_Config.hpp"
#include "ESP32MQTTClient.h"
#include "esp_idf_version.h"
#include "esp32-hal-log.h"
#include "rom/ets_sys.h" /// per ISR logging
#include "semphr.h" // Per usare i semafori
#include <string> /// Stringhe standard del c++

using namespace std;// Usato per le stringhe standard del c++




/*╔═════════════════════════════════════════════╗*/
/*║                 LOGGER CODE                 ║*/
/*╚═════════════════════════════════════════════╝*/
/// Per quanti millisecondi il semaforo blocca la task al MAX se non riceve subito il semaforo
#define __SEMAPHORE_TIMEOUT_MS__ 200
static SemaphoreHandle_t xSemaphoreLogger;
static bool isMqttConnected = false;


#ifdef LOG_ACTIVE
  // ANSI Code per colorare i logs
  #define RST_COLOR       "\033[0m" // Resetta il colore alla fine del punto che si vuole colorare
  /// Normale
  #define BASE_ERROR      "\033[0;31m" // Rosso
  #define BASE_WARNING    "\033[0;33m" // Giallo
  #define BASE_INFO       "\033[0;32m" // Verde
  #define BASE_DEBUG      "\033[0;36m" // Ciano
  #define BASE_DETAILS    "\033[0;38m" // Grigio
  /// Grassetti
  #define BOLD_ERROR      "\033[1;31m" // Rosso
  #define BOLD_WARNING    "\033[1;33m" // Giallo
  #define BOLD_INFO       "\033[1;32m" // Verde
  #define BOLD_DEBUG      "\033[1;36m" // Ciano
  #define BOLD_DETAILS    "\033[1;38m" // Grigio

  /// topics del logger:
  #define BASE_LOG_TOPIC      "Decapsulator_Logger/"
  #define ERROR_LOG_TOPIC     BASE_LOG_TOPIC "ERROR/"
  #define WARNING_LOG_TOPIC   BASE_LOG_TOPIC "WARNING/"
  #define INFO_LOG_TOPIC      BASE_LOG_TOPIC "INFO/"
  #define DEBUG_LOG_TOPIC     BASE_LOG_TOPIC "DEBUG/"
  #define DETAILS_LOG_TOPIC   BASE_LOG_TOPIC "DETAILS/"


  /// Default Decapsulator Logger Format
  #define _FORMAT_(logType, format, tag) \
      BOLD_ ## logType \
      "\n[### " #logType " ###]"\
      RST_COLOR\
      BASE_ ## logType \
      "\nFile: \"%s\",\
      \nLine:  %u,\
      \nTask: \"%s\",\
      \nFunc: \"%s\",\
      \nTime:  %ums,\
      \n[ Tag: \"%s\" ] = \n"\
      format\
      "\n\n"\
      RST_COLOR,\
      pathToFileName(__FILE__),\
      __LINE__,\
      pcTaskGetName(xTaskGetCurrentTaskHandle()),\
      __FUNCTION__,\
      (uint32_t)(esp_timer_get_time() / 1000ULL),\
      (const char*)tag
      
  /// Aggiunge al topic molte altre informazioni (Neccessario perchè se no vengono sovrascritti i log sempre)
  #define _TOPIC_MQTT_(logType, tag)\
      string topic = logType;\
      topic += "File:" + string(pathToFileName(__FILE__)) +\
              "/Task:" + string(pcTaskGetName(xTaskGetCurrentTaskHandle())) +\
              "/Func:" + string(__FUNCTION__) +\
              "/Line:" + to_string(__LINE__) +\
              "/Time:" + to_string(esp_timer_get_time() / 1000ULL) +\
              " [ Tag : " + string(tag) + " ] "




  #ifdef LOG_MQTT_ACTIVE

    ESP32MQTTClient mqttClient; // Oggetto di tipo Client MQTT

    /// Funzione di formattazione e log MQTT
    static void mqtt_logger_printf(string& topic, const char* format, ...)
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
#endif



#define __SEMAPHORE_TIMEOUT_TICKS__ pdMS_TO_TICKS(__SEMAPHORE_TIMEOUT_MS__)

/// @precompilazione: Se non è predisposto il log o non è specificata la sua attivazione definisce delle macro vuote
#ifdef LOG_ACTIVE

  #ifdef LOG_MQTT_ACTIVE
    /// Default Decapsulator Logger
    #define __DECAPSULATOR_LOG(logType, tag, format, ...)\
      do\
      {\
	      xSemaphoreTake(xSemaphoreLogger, __SEMAPHORE_TIMEOUT_TICKS__);\
        log_printf(_FORMAT_(logType, format, tag), ##__VA_ARGS__);\
        if(isMqttConnected)\
        {\
          _TOPIC_MQTT_(logType ## _LOG_TOPIC, tag);\
          mqtt_logger_printf(topic, format, ##__VA_ARGS__);\
        }\
	      xSemaphoreGive(xSemaphoreLogger);\
      } while(0)
  #else
    /// Default Decapsulator Logger
    #define __DECAPSULATOR_LOG(logType, tag, format, ...)\
      do\
      {\
	      xSemaphoreTake(xSemaphoreLogger, __SEMAPHORE_TIMEOUT_TICKS__);\
        log_printf(_FORMAT_(logType, format, tag), ##__VA_ARGS__);\
	      xSemaphoreGive(xSemaphoreLogger);\
      } while(0)  
  #endif

    /// Default Decapsulator Logger For Interrupt Service Routines (ISR)
    #define __ISR_DECAPSULATOR_LOG(logType, tag, format, ...)\
      do\
      {\
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;\
	      xSemaphoreTakeFromISR(xSemaphoreLogger, &xHigherPriorityTaskWoken);\
        ets_printf(ARDUHAL_LOG_FORMAT(E, format), ##__VA_ARGS__);\
        xSemaphoreGiveFromISR(xSemaphoreLogger, &xHigherPriorityTaskWoken);\
      } while(0)

#else // NO LOGGER DEFINES

  /// NO LOGGER Default Decapsulator Logger
  #define __DECAPSULATOR_LOG(logType, tag, format, ...)
  /// NO LOGGER Default Decapsulator Logger For Interrupt Service Routines (ISR)
  #define __ISR_DECAPSULATOR_LOG(logType, tag, format, ...)
  
#endif


/*╔══════════════════════════════════════════════╗*/
/*║             Per Logging Normale              ║*/
/*╚══════════════════════════════════════════════╝*/
/// @logging: degli @errori:
#define LogError(tag, format, ...)         __DECAPSULATOR_LOG(ERROR, tag, format, ##__VA_ARGS__)

/// @logging: degli @avvertimenti:
#define LogWarning(tag, format, ...)       __DECAPSULATOR_LOG(WARNING, tag, format, ##__VA_ARGS__)

/// @logging: delle @informazioni:
#define LogInfo(tag, format, ...)          __DECAPSULATOR_LOG(INFO, tag, format, ##__VA_ARGS__)

/// @logging: di @debug:
#define LogDebug(tag, format, ...)         __DECAPSULATOR_LOG(DEBUG, tag, format, ##__VA_ARGS__)

/// @logging: dei @dettagli:
#define LogDetails(tag, format, ...)       __DECAPSULATOR_LOG(DETAILS, tag, format, ##__VA_ARGS__)


/*╔══════════════════════════════════════════════╗*/
/*║ Per Logging Nelle Interrupt Service Routine  ║*/
/*╚══════════════════════════════════════════════╝*/
/// @logging: degli @errori: nelle Interrupt Service Routine:
#define LogErrorISR(tag, format, ...)      __ISR_DECAPSULATOR_LOG(ERROR, tag, format, ##__VA_ARGS__)

/// @logging: degli @avvertimenti: nelle Interrupt Service Routine:
#define LogWarningISR(tag, format, ...)    __ISR_DECAPSULATOR_LOG(WARNING, tag, format, ##__VA_ARGS__)

/// @logging: delle @informazioni: nelle Interrupt Service Routine:
#define LogInfoISR(tag, format, ...)       __ISR_DECAPSULATOR_LOG(INFO, tag, format, ##__VA_ARGS__)

/// @logging: di @debug: nelle Interrupt Service Routine:
#define LogDebugISR(tag, format, ...)      __ISR_DECAPSULATOR_LOG(DEBUG, tag, format, ##__VA_ARGS__)

/// @logging: dei @dettagli: nelle Interrupt Service Routine:
#define LogDetailsISR(tag, format, ...)    __ISR_DECAPSULATOR_LOG(DETAILS, tag, format, ##__VA_ARGS__)







/**
 *  @brief Inizializza il WiFi, in primo luogo logger remoto
 *
 *  @note se è già inizializzato non esegue nulla
 */
extern void startWiFi(uint32_t timeout_for_each_initialization_ms = DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS)
{
  /// Debug
  uint32_t startTime = millis();


  #if defined(LOG_ACTIVE) && defined(LOG_MQTT_ACTIVE)
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
    
    
  #if defined(LOG_ACTIVE) && defined(LOG_MQTT_ACTIVE)
    /// Timeout che se supera un certo tempo riavvia l'ESP
    mqttClient.loopStart();
    ESP_timeoutRestart = millis();
    while(!mqttClient.isConnected())
    {
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
    LogInfo("WiFi Connect", "\nWiFi connesso: %s\nBroker MQTT: %s\n\n\n\n", WiFi.localIP().toString().c_str(), MQTT_BROKER);
  #else
    /// Printa info sull'IP del dispositivo
    LogInfo("WiFi Connect", "\nWiFi connesso: %s\n\n\n\n", WiFi.localIP().toString().c_str());  
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
 *        che devono essere eseguite: ex. n_inizializzazioni = WiFi.begin() + Insights.begin() = 2, fTimeoutTot = n_inizializzazioni * timeout = 2*timeout
 *
 */
void LogBegin(uint32_t timeout_for_each_initialization_ms = DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS)
{
  /// Limita il baud al massimo consentito per la Serial port di Arduino IDE
  /// @warning Viene Inizializzata comunque la seriale per fare dei test rapidi
  ///          anche se è disabilita la funione di Log con dei classici metodi dell'oggetto "Serial"
  Serial.begin(115200); 
  
  /// Permette i log dalla WiFi Library
  Serial.setDebugOutput(true);    


  xSemaphoreLogger = xSemaphoreCreateMutex();

  if(xSemaphoreLogger == NULL)
  {
    Serial.print("Errore nella creazione del Semaforo del logger, riavvio in...3");
    delay(1000);
    Serial.print("...2");
    delay(1000);
    Serial.println("...1\n\n");
    delay(1000);
    ESP.restart();
  }

  /// Inizializza il WiFi @ref @file "WiFi_secrets.hpp"
  startWiFi(timeout_for_each_initialization_ms);
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





/// @brief Modifica una variabile in runtime scrivendola in un determinato topic mqtt
/// @warning il topic sarà composto così: Decapsulator_Logger/DEBUG OVERRIDE/override_topic 
template<typename varType>
void DebugOverrideVar(const String override_topic, varType* var)
{
  if(!var)
    return;

  /// Scrive il topic utente
  String topic = BASE_LOG_TOPIC "DEBUG OVERRIDE/" + override_topic;

  /// ATTENZIONE: il puntatore deve essere valido per tutta la durata della sottoscrizione.
  mqttClient.subscribe(topic.c_str(), [var](const string &topic, const string &payload)
  {
      *var = FromStringToVarType<varType>(payload);
  });
}






/**
 *    MQTT Library Custom Functions
 */


#if defined(LOG_ACTIVE) && defined(LOG_MQTT_ACTIVE)
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
#endif
