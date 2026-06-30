/**
 * @file Debug.h
 * @brief ESP32-S3 Debug Logger con MQTT e/o Serial UART
 *
 * @details
 * Questo file implementa un logger per ESP32-S3 che invia messaggi di debug:
 * - tramite porta seriale UART la quale è thread-safe per le ESP32 (non ha bisogno di un'altra task)
 * - tramite MQTT usando la libreria thread-safe ESP32MQTTClient @version 1.0.0
 *
 * Il sistema è progettato per essere thread-safe, non bloccare il loop principale,
 * e può essere esteso per supportare anche SD card Log, o altre destinazioni.
 *
 * @authors   Randazzo Manuel, Pisan Alessio
 * @date      2025-05-24
 *
 *
 * ATTENZIONE: @todo BISOGNA ASSOLUTAMENTE USARE (O PROVARE) ESP INSIGHTS NELLA @class "DEBUG" USANDO ANCHE L'ESEMPIO BASE FORNITO.
 *              L'IDEA DI ESP INSIGHTS E' QUELLA DI REPORTARE BUG E PROBLEMI GENERALI CIRCA TUTTO QUELLO CHE RIGUARDA L'ESP32
 *              AD ESEMPIO ANCHE CAUSE DI CRASH E REBOOT.
 *              PRATICAMENTE BISOGNA SETTARE IL WIFI E FORSE ANCHE COME VENGONO INVIATI I LOGS (es tramite MQTT o HTTPS) E IN
 *              const char*MATICO DOVREBBERO ESSERE INVIATI I LOG SIA SU SERIALE CHE DA REMOTO TRAMITE
 *              LOGV (Log Verbouse), LOGI (Log Info), LOGE (Log Error), LOGD (Log Debug), LOGW (Log Warning)
 *              @link alla dashboard : https://dashboard.insights.espressif.com
 *              E' DA FARE UN @test ANCHE DI LOGx NORMALE DOPO AVERE SETTATO TUTTO CON IL GIA' DICHIARATO OGGETTO "Insights"
 *              PER VEDERE SE AVVIENE IN const char*MATICO IL LOG ANCHE SULLA DASHBOARD ONLINE 
 *
 *              bool Insights.begin(const char *auth_key, const char *node_id = NULL,
 *              uint32_t log_type = 0xFFFFFFFF, bool alloc_ext_ram = false,
 *              bool use_default_transport = true)
 */

#pragma once

/// File contenente ssid e la password dell'Utente
#include "WiFi_secrets.hpp"
#include "ESP32MQTTClient.h"
#include "rom/ets_sys.h" /// per ISR logging
#include "esp32-hal-log.h"

/** ╔═════════════════════════════════════════════╗
    ║             USER: LOGGER OPTIONS            ║
    ╚═════════════════════════════════════════════╝ */

/// Commentando questa riga si disattivano i LOG senza il bisogno di cancellarli nel programma
#define LOG_ACTIVE
/// Commentando questa riga si disattivano i LOG MQTT senza il bisogno di cancellarli nel programma
#define LOG_MQTT_ACTIVE


/// @brief Inizializza sia il logger (in preprocessor) che il WiFi (runtime)
void LogBegin(uint32_t timeout_for_each_initialization_ms);

/// @brief Modifica una variabile in runtime scrivendola in un determinato topic mqtt
//void OverrideVar(String override_topic, auto* var );











/*╔═════════════════════════════════════════════╗*/
/*║                 LOGGER CODE                 ║*/
/*╚═════════════════════════════════════════════╝*/


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
    "\nTime:  %ums,\
    \nFile: \"%s\",\
    \nLine:  %u,\
    \nTask: \"%s\",\
    \nFunc: \"%s\",\
    \nTag : \"%s\"\
    \n" format\
    "\n\n"\
    RST_COLOR,\
    (uint32_t)(esp_timer_get_time() / 1000ULL),\
    pathToFileName(__FILE__),\
    __LINE__,\
    pcTaskGetName(xTaskGetCurrentTaskHandle()),\
    __FUNCTION__,\
    (const char*)tag




#if defined(LOG_ACTIVE) && defined(LOG_MQTT_ACTIVE)

  /// Test Mosquitto broker, @see https://test.mosquitto.org
  #define broker "mqtt://test.mosquitto.org:1883"
  ESP32MQTTClient mqttClient; // all params are set later


  static void mqtt_logger_printf(const char* topic, const char* tag, const char* format, ...)
  {
    static char loc_buf[64];
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
    
    String tempTopic = topic;

    /// Aggiunge al topic molte altre informazioni (Neccessario perchè se no vengono sovrascritti i log sempre)
    tempTopic += "File: " + String(pathToFileName(__FILE__)) + "/Task: " + String(pcTaskGetName(xTaskGetCurrentTaskHandle())) + "/Func: " + __FUNCTION__ + "/Line: " + __LINE__ + "/Time:" + String(esp_timer_get_time() / 1000ULL) + "[ " + tag + " ]";
    mqttClient.publish(tempTopic.c_str(), temp, 0, false);
    
    /// Libera la memoria eventualmente allocata
    if(len >= sizeof(loc_buf))
      free(temp);
  }

  
  static void mqtt_logger_printf(String topic, String tag, const char* format, ...)
  {
    va_list(args);
    va_start(args, format);
    mqtt_logger_printf(topic.c_str(), tag.c_str(), format, args);
    va_end(args);
  }
#else /// Definisce altrimenti una funzione vuota
  #define mqtt_logger_printf(topic, tag, format, ...) (void)0
#endif




/// @precompilazione: Se non è predisposto il log o non è specificata la sua attivazione definisce delle macro vuote
#ifdef LOG_ACTIVE

  /// Default Decapsulator Logger
  #define __DECAPSULATOR_LOG(logType, tag, format, ...)\
    do\
    {\
      log_printf(_FORMAT_(logType, format, tag), ##__VA_ARGS__);\
      mqtt_logger_printf(logType ## _LOG_TOPIC, tag, format, ##__VA_ARGS__);\
      delayMicroseconds(10);\
    } while(0)
  /// Default Decapsulator Logger For Interrupt Service Routines (ISR)
  #define __ISR_DECAPSULATOR_LOG(logType, tag, format, ...)\
    do\
    {\
      log_printf(_FORMAT_(logType, format, "ISR " tag), ##__VA_ARGS__);\
      mqtt_logger_printf(logType ## _LOG_TOPIC, "ISR" tag, format, ##__VA_ARGS__);\
      delayMicroseconds(10);\
    } while(0)
#else

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



void onMqttConnect(esp_mqtt_client_handle_t client)
{
  if (mqttClient.isMyTurn(client)) // can be omitted if only one client
  {
    mqttClient.subscribe("bar/#", [](const std::string &topic, const std::string &payload)
                          { log_i("%s: %s", topic.c_str(), payload.c_str()); });
  }
}

esp_err_t handleMQTT(esp_mqtt_event_handle_t event)
{
    mqttClient.onEventCallback(event);
    return ESP_OK;
}



/**
 *  @brief Inizializza il WiFi, in primo luogo logger remoto
 *
 *  @note se è già inizializzato non esegue nulla
 */
extern void startWiFi(uint32_t timeout_for_each_initialization_ms = DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS)
{
  #if defined(LOG_ACTIVE) && defined(LOG_MQTT_ACTIVE)
    /// Avvio del broker MQTT sulla porta 1883
    mqttClient.enableDebuggingMessages();

    mqttClient.setURI(broker);
    mqttClient.enableLastWillMessage("brokerStatus", "offline");
    mqttClient.setKeepAlive(30);
    mqttClient.setOnMessageCallback([](const std::string &topic, const std::string &payload) {
      LogInfo("MQTT Client onMessageCallback Init", "Global callback: %s: %s", topic.c_str(), payload.c_str());
    });
  #endif


    WiFi.mode(WIFI_STA); //per evitare conflitti con le risorse del bluetooth
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setHostname("Decapsulator");
    LogInfo("WiFi Connect", "Connessione Wi-Fi");

    /// Timeout che se supera un certo tempo riavvia l'ESP
    uint32_t ESP_timeoutRestart = millis();
    while(WiFi.status() != WL_CONNECTED)
    {
      LogWarning("YOOOOO", "ATTENZIONENENNENENE");
      /// Se sfora il timeout allora restarta l'ESP32
      if(millis() - ESP_timeoutRestart > DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS)
      {
        LogError("WiFi Connect", "CONNECTION ERROR TIMEOUT.\nRebooting...");
        ESP.restart();
        break;
      }
    }    
    mqttClient.loopStart();

    LogInfo("WiFi Connect", "\nWiFi connesso: %s\nBroker MQTT: %s", WiFi.localIP().toString().c_str(), broker);

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
  
  #ifdef LOG_ACTIVE
    /// Permette i log dalla WiFi Library
    Serial.setDebugOutput(true);    
  #endif

  
  /// Inizializza il WiFi @ref @file "WiFi_secrets.hpp"
  startWiFi(timeout_for_each_initialization_ms);
}