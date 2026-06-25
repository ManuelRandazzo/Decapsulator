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

    
/// Definisce la lunghezza del message buffer dei log
#define LOGGER_MESSAGE_BUFF_LEN 25
#define LOGGER_MAX_MESSAGE_SIZE 512
#define LOGGER_MAX_TOPIC_SIZE 40
#define LOGGER_BEGIN_INIT_TIMEOUT_MS 5000

/// @attention Viene perso anche l'OTA e l'MQTT commentando questa riga la quale disattiva l'inizializzazione del WiFi
#define WiFi_ACTIVE
/// @info: Commentando questa riga si disattivano i LOG senza il bisogno di cancellarli nel programma
#define LOG_ACTIVE
/// @info: Commentando questa riga si disattivano i log in seriale senza il bisogno di cancellarli nel programma
#define LOG_ACTIVE_SERIAL
/// @info: Commentando questa riga si disattivano i log in MQTT senza il bisogno di cancellarli nel programma
//#define LOG_ACTIVE_MQTT
/// @info: Commentando questa riga si disattivano i log in SD
#define LOG_COPY_TO_SD
/// @info: Commentando questa riga non vi saranno più log da parte del Programma Principale del decapsulator
#define LOG_ACTIVE_MAIN_PRG
/// @info: Commentando questa riga non vi saranno più log da parte della TaskTypedef class
//#define LOG_ACTIVE_TASK
/// @info: Commentando questa riga non vi saranno più log da parte della motion class
//#define LOG_ACTIVE_MOTION
/// @info: Commentando questa riga si disattivano i LOG delle code di trasferimento tra main prg e HMI
//#define LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
/// @info: Scommentando questa riga si disattiva il restart dell'esp in caso di fail del wifi e/o dell'MQTT
#define NO_ESP_RESTART_ON_CONNECTION_FAILURE


/*╔═════════════════════════════════════════════╗*/
/*║                 NOTIFY CODES                ║*/
/*╚═════════════════════════════════════════════╝*/

#define SD_LOG_REFRESH (1 << 0)
#define SD_LOG_CLEAN (1 << 1)



/*╔═════════════════════════════════════════════╗*/
/*║                 LOGGER CODE                 ║*/
/*╚═════════════════════════════════════════════╝*/
extern MessageBufferHandle_t LoggerMessageHandler;
extern void LoggerTask(void* pvParameters);
extern void startWiFi(uint32_t timeout_for_each_initialization_ms);

struct log_msg_t
{
    char message[LOGGER_MAX_MESSAGE_SIZE];
    char mqttTopic[LOGGER_MAX_TOPIC_SIZE];

    const char* logTypeStr;
    uint8_t logTypeColor; // 1=rosso, 3=giallo, 2=verde, 6=ciano, 8=grigio
    const char* file;
    const char* task;
    const char* func;
    uint32_t line;
    uint32_t time;
    const char* tagStr;
};

#ifndef WiFi_ACTIVE
    #undef LOG_ACTIVE_MQTT
#endif

/// Disattiva tutti i log se LOG_ACTIVE non è definito
#ifndef LOG_ACTIVE
    #undef LOG_ACTIVE_SERIAL
    #undef LOG_ACTIVE_MQTT
    #undef LOG_COPY_TO_SD
    #undef LOG_ACTIVE_MAIN_PRG
    #undef LOG_ACTIVE_TASK
    #undef LOG_ACTIVE_MOTION
    #undef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
#endif

#define _LOG_SNPRINTF_FMT_COLORS_(buffer, size, msg)\
    snprintf(buffer, size, "\033[1;3%dm\n[### %s ###]\033[0m\033[0;3%dm\
                            \nFile: \"%s\",\
                            \nLine:  %u,\
                            \nTask: \"%s\",\
                            \nFunc: \"%s\",\
                            \nTime:  %lums,\
                            \n[ Tag: \"%s\" ] = \n%s\n\n\033[0m",\
                            msg.logTypeColor,\
                            msg.logTypeStr,\
                            msg.logTypeColor,\
                            msg.file,\
                            msg.line,\
                            msg.task,\
                            msg.func,\
                            msg.time,\
                            msg.tagStr,\
                            msg.message)

#define _LOG_SNPRINTF_FMT_NO_COLORS_(buffer, size, msg)\
    snprintf(buffer, size, "\n[### %s ###]\
                            \nFile: \"%s\",\
                            \nLine:  %u,\
                            \nTask: \"%s\",\
                            \nFunc: \"%s\",\
                            \nTime:  %lums,\
                            \n[ Tag: \"%s\" ] = \n%s\n\n",\
                            msg.logTypeStr,\
                            msg.file,\
                            msg.line,\
                            msg.task,\
                            msg.func,\
                            msg.time,\
                            msg.tagStr,\
                            msg.message)
    
    
/// Aggiunge al topic molte altre informazioni (Neccessario perchè se no vengono sovrascritti i log sempre)
#define _LOG_SNPRINTF_TOPIC_MQTT_(msg)\
    snprintf(msg.mqttTopic, LOGGER_MAX_TOPIC_SIZE, "Decapsulator_Logger/%s/File:%s/Task:%s/Func:%s/Line:%d/Time:%lums [ Tag : %s ]",\
                                                    msg.logTypeStr,\
                                                    msg.file,\
                                                    msg.task,\
                                                    msg.func,\
                                                    msg.line,\
                                                    msg.time,\
                                                    msg.tagStr)
    
    
// ANSI Code per colorare i logs
#define ERROR_COLOR   ( (uint8_t)(1) ) // "\033[x;31m" Rosso
#define WARNING_COLOR ( (uint8_t)(3) ) // "\033[x;33m" Giallo
#define INFO_COLOR    ( (uint8_t)(2) ) // "\033[x;32m" Verde
#define DEBUG_COLOR   ( (uint8_t)(6) ) // "\033[x;36m" Ciano
#define DETAILS_COLOR ( (uint8_t)(8) ) // "\033[x;38m" Grigio


/// @precompilazione: Se non è predisposto il log o non è specificata la sua attivazione definisce delle macro vuote
#ifdef LOG_ACTIVE

    #define __BASE_DEACAPSULATOR_LOG(logType, color, tag, format, ...)\
        if(LoggerMessageHandler == NULL)\
            break;\
        log_msg_t msg;\
        msg.logTypeStr = logType,\
        msg.logTypeColor = color,\
        msg.tagStr     = tag,\
        msg.file = pathToFileName(__FILE__),\
        msg.line = __LINE__,\
        msg.task = pcTaskGetName(xTaskGetCurrentTaskHandle()),\
        msg.func = __FUNCTION__,\
        msg.time = (uint32_t)(esp_timer_get_time() / 1000ULL),\
        /* Fa un "piccolo" snprintf per settare i __VA_ARGS__ */\
        snprintf(msg.message, LOGGER_MAX_MESSAGE_SIZE, format, ##__VA_ARGS__)


    /// Default Decapsulator Logger
    #define __DECAPSULATOR_LOG(logType, color, tag, format, ...)\
        do\
        {\
            __BASE_DEACAPSULATOR_LOG(logType, color, tag, format, ##__VA_ARGS__);\
            xMessageBufferSend(LoggerMessageHandler, &msg, sizeof(msg), 0);\
        } while(0)


    /// Default Decapsulator Logger per Interrupt Service Routines (ISR)
    #define __ISR_DECAPSULATOR_LOG(logType, color, tag, format, ...)\
        do\
        {\
            __BASE_DEACAPSULATOR_LOG(logType, color, tag, format, ##__VA_ARGS__);\
            BaseType_t __LoggerMessageBuffHigherPriorityTaskWoken__ = pdFALSE;\
            xMessageBufferSendFromISR(LoggerMessageHandler, &msg, sizeof(msg), &__LoggerMessageBuffHigherPriorityTaskWoken__);\
            if(__LoggerMessageBuffHigherPriorityTaskWoken__)\
                portYIELD_FROM_ISR();\
        } while(0)
        
#else // NO LOGGER DEFINES

    /// NO LOGGER Default Decapsulator Logger
    #define __DECAPSULATOR_LOG(logType, color, tag, format, ...)
    /// NO LOGGER Default Decapsulator Logger For Interrupt Service Routines (ISR)
    #define __ISR_DECAPSULATOR_LOG(logType, color, tag, format, ...)
  
#endif


/*╔══════════════════════════════════════════════╗*/
/*║             Per Logging Normale              ║*/
/*╚══════════════════════════════════════════════╝*/
/// @logging: degli @errori:
#define LogError(tag, format, ...)         __DECAPSULATOR_LOG("ERROR", ERROR_COLOR, tag, format, ##__VA_ARGS__)

/// @logging: degli @avvertimenti:
#define LogWarning(tag, format, ...)       __DECAPSULATOR_LOG("WARNING", WARNING_COLOR, tag, format, ##__VA_ARGS__)

/// @logging: delle @informazioni:
#define LogInfo(tag, format, ...)          __DECAPSULATOR_LOG("INFO", INFO_COLOR, tag, format, ##__VA_ARGS__)

/// @logging: di @debug:
#define LogDebug(tag, format, ...)         __DECAPSULATOR_LOG("DEBUG", DEBUG_COLOR, tag, format, ##__VA_ARGS__)

/// @logging: dei @dettagli:
#define LogDetails(tag, format, ...)       __DECAPSULATOR_LOG("DETAILS", DETAILS_COLOR, tag, format, ##__VA_ARGS__)


/*╔══════════════════════════════════════════════╗*/
/*║ Per Logging Nelle Interrupt Service Routine  ║*/
/*╚══════════════════════════════════════════════╝*/
/// @logging: degli @errori: nelle Interrupt Service Routine:
#define LogErrorISR(tag, format, ...)      __ISR_DECAPSULATOR_LOG("ERROR", ERROR_COLOR, tag, format, ##__VA_ARGS__)

/// @logging: degli @avvertimenti: nelle Interrupt Service Routine:
#define LogWarningISR(tag, format, ...)    __ISR_DECAPSULATOR_LOG("WARNING", WARNING_COLOR, tag, format, ##__VA_ARGS__)

/// @logging: delle @informazioni: nelle Interrupt Service Routine:
#define LogInfoISR(tag, format, ...)       __ISR_DECAPSULATOR_LOG("INFO", INFO_COLOR, tag, format, ##__VA_ARGS__)

/// @logging: di @debug: nelle Interrupt Service Routine:
#define LogDebugISR(tag, format, ...)      __ISR_DECAPSULATOR_LOG("DEBUG", DEBUG_COLOR, tag, format, ##__VA_ARGS__)

/// @logging: dei @dettagli: nelle Interrupt Service Routine:
#define LogDetailsISR(tag, format, ...)    __ISR_DECAPSULATOR_LOG("DETAILS", DETAILS_COLOR, tag, format, ##__VA_ARGS__)