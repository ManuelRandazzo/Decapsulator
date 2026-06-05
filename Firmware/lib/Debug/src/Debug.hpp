/**
 * @file Debug.hpp
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


#include "Arduino.h"
#include <string>              /// Stringhe standard del c++
#include "esp_task_wdt.h"      /// Per disattivare momentaneamente il INT_WDT che causa reset involontari

using namespace std;// Usato per le stringhe standard del c++

/** ╔═════════════════════════════════════════════╗
    ║             USER: LOGGER OPTIONS            ║
    ╚═════════════════════════════════════════════╝ */

    
/// Definisce la lunghezza della coda dei log
#define LOGGER_QUEUE_LEN 20
#define LOGGER_MAX_MESSAGE_SIZE 512
#define LOGGER_BEGIN_INIT_TIMEOUT_MS 5000

/// @attention Viene perso anche l'OTA e l'MQTT commentando questa riga la quale disattiva l'inizializzazione del WiFi
#define WiFi_ACTIVE
/// @info: Commentando questa riga si disattivano i LOG senza il bisogno di cancellarli nel programma
#define LOG_ACTIVE
/// @info: Commentando questa riga si disattivano i LOG MQTT senza il bisogno di cancellarli nel programma
#define LOG_ACTIVE_MQTT
/// @info: Commentando questa riga si disattivano i log in SD
//#define LOG_COPY_TO_SD
/// @info: Commentando questa riga non vi saranno più log da parte del Programma Principale del decapsulator
#define LOG_ACTIVE_MAIN_PRG
/// @info: Commentando questa riga non vi saranno più log da parte della TaskTypedef class
//#define LOG_ACTIVE_TASK
/// @info: Commentando questa riga non vi saranno più log da parte della motion class
//#define LOG_ACTIVE_MOTION
/// @info: Commentando questa riga si disattivano i LOG delle code di trasferimento tra main prg e HMI
//#define LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
/// @info: Scommentando questa riga si disattiva il restart dell'esp in caso di fail del wifi e/o dell'MQTT
//#define NO_ESP_RESTART_ON_CONNECTION_FAILURE




/*╔═════════════════════════════════════════════╗*/
/*║                 LOGGER CODE                 ║*/
/*╚═════════════════════════════════════════════╝*/
extern QueueHandle_t LoggerQueueHandler;
extern void LoggerTask(void* pvParameters);
extern void startWiFi(uint32_t timeout_for_each_initialization_ms);

struct log_msg_t
{
    string mqttTopic;
    string message;

    /// Costruttore base
    log_msg_t() : message(LOGGER_MAX_MESSAGE_SIZE, ' ') {}
};

#if !defined(WiFi_ACTIVE)
    #undef LOG_ACTIVE_MQTT
#endif


/// @precompilazione: Se non è predisposto il log o non è specificata la sua attivazione definisce delle macro vuote
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
    
    #ifdef LOG_ACTIVE_MQTT
        /// topics del logger:
        #define BASE_LOG_TOPIC      "Decapsulator_Logger/"
        #define ERROR_LOG_TOPIC     BASE_LOG_TOPIC "ERROR/"
        #define WARNING_LOG_TOPIC   BASE_LOG_TOPIC "WARNING/"
        #define INFO_LOG_TOPIC      BASE_LOG_TOPIC "INFO/"
        #define DEBUG_LOG_TOPIC     BASE_LOG_TOPIC "DEBUG/"
        #define DETAILS_LOG_TOPIC   BASE_LOG_TOPIC "DETAILS/"
        
        /// Aggiunge al topic molte altre informazioni (Neccessario perchè se no vengono sovrascritti i log sempre)
        #define _TOPIC_MQTT_(logType, tag)\
            msg.mqttTopic = logType;\
            msg.mqttTopic += "File:" + string(pathToFileName(__FILE__)) +\
                             "/Task:" + string(pcTaskGetName(xTaskGetCurrentTaskHandle())) +\
                             "/Func:" + string(__FUNCTION__) +\
                             "/Line:" + to_string(__LINE__) +\
                             "/Time:" + to_string(esp_timer_get_time() / 1000ULL) +\
                             " [ Tag : " + string(tag) + " ] "
    #endif






    #ifdef LOG_ACTIVE_MQTT
        /// Default Decapsulator Logger
        #define __DECAPSULATOR_LOG(logType, tag, format, ...)\
            do\
            {\
                if(LoggerQueueHandler == NULL)\
                    break;\
                log_msg_t msg;\
                snprintf(msg.message.data(), LOGGER_MAX_MESSAGE_SIZE, _FORMAT_(logType, format, tag), ##__VA_ARGS__);\
                _TOPIC_MQTT_(logType ## _LOG_TOPIC, tag);\
                xQueueSend(LoggerQueueHandler, &msg, 0);\
            } while(0)
    #else
        /// Default Decapsulator Logger
        #define __DECAPSULATOR_LOG(logType, tag, format, ...)\
            do\
            {\
                if(LoggerQueueHandler == NULL)\
                    break;\
                log_msg_t msg;\
                snprintf(msg.message.data(), LOGGER_MAX_MESSAGE_SIZE, _FORMAT_(logType, format, tag), ##__VA_ARGS__);\
                xQueueSend(LoggerQueueHandler, &msg, 0);\
            } while(0)
    #endif

    /// Default Decapsulator Logger For Interrupt Service Routines (ISR)
    #define __ISR_DECAPSULATOR_LOG(logType, tag, format, ...)\
        do\
        {\
            if(LoggerQueueHandler == NULL)\
                break;\
            BaseType_t __LoggerQueueHigherPriorityTaskWoken__ = pdFALSE;\
            log_msg_t msg;\
            snprintf(msg.message.data(), LOGGER_MAX_MESSAGE_SIZE, _FORMAT_(logType, format, tag), ##__VA_ARGS__);\
            xQueueSendFromISR(LoggerQueueHandler, &msg, &__LoggerQueueHigherPriorityTaskWoken__);\
            if(__LoggerQueueHigherPriorityTaskWoken__)\
                portYIELD_FROM_ISR();\
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