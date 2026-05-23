#pragma once

#include "FreeRTOS.h"
/// Include il file per la gestione della coda
#include "queue.h"



#define TICKS_TO_WAIT_QUEUE_RECEIVE_HMI (pdMS_TO_TICKS(0))
#define TICKS_TO_WAIT_QUEUE_SEND_HMI    (pdMS_TO_TICKS(0))
#define HMI_CMD_QUEUE_LEN 5
#define HMI_EVT_QUEUE_LEN 5


/**
 * @brief Inizializza le code di trasmissione tra ui e backend
 * 
 * @return se sono state create con successo tutte le code
 */
extern BaseType_t hmi_queues_begin(void);




#pragma region (COMMAND_QUEUE)

typedef enum : int8_t
{
    NO_CMD = -1,
    TAMBURO_ENABLE,
    TAMBURO_DISABLE,
    TAMBURO_JOG_POSITIVE,
    TAMBURO_JOG_NEGATIVE,

    PUNZONE_ENABLE,
    PUNZONE_DISABLE,
    PUNZONE_JOG_POSITIVE,
    PUNZONE_JOG_NEGATIVE,
    
    HOMING,
} JogState;


/// @brief Handler della coda che riceve i comandi che arrivano dall'HMI
extern QueueHandle_t QueueHandlerHMI_CMD;

/// @brief Direction: Frontend --> Backend 
struct CommandQueueHMI_t
{
    unsigned StartMachine : 1;
    unsigned StopMachine : 1;
    unsigned restartAfterContainerEmptied : 1;

    /// JOG
    unsigned jogPageActive : 1;

    JogState jogStateCMD;

    float    jogRallaGradiPerClick;   /// Quanti gradi fa ad ogni click del pulsante
    float    jogRallaSpeed;           /// Velocità del jog in gradi al secondo

    float    jogPunzoneGradiPerClick;   /// Quanti gradi fa ad ogni click del pulsante
    float    jogPunzoneSpeed;           /// Velocità del jog in gradi al secondo

    unsigned unusedBits : 20; // = 32 - n° bit occupati dagli altri membri
};

/// Struct di inizializzazione (evita errori nell'utilizzo di membri non inizializzati)
extern const CommandQueueHMI_t defaultCommandQueueHMI;

#pragma endregion (COMMAND_QUEUE)




#pragma region (EVENT_QUEUE)

/// @brief Handler della coda che invia gli eventi (errori, avvisi, ecc.) all'HMI
extern QueueHandle_t QueueHandlerHMI_EVT; 

/// @brief Direction: Backend --> Frontend 
struct EventQueueHMI_t
{
    unsigned xShutDown : 1;
    
    /// ERRORI:
    unsigned xErrorInitPunz : 1;
    unsigned xErrorInitRalla : 1;
    unsigned xErrorCapsIncastrata : 1;
    //unsigned  xError : 1;
    
    /// JOG
    unsigned jogRallaIsMoving : 1;    /// Indica se il motore si sta muovendo
    unsigned jogRallaCalibrationStatus : 1;
    unsigned jogPunzoneIsMoving : 1;    /// Indica se il motore si sta muovendo
    unsigned jogPunzoneFineCorsaMaxStatus : 1;
    unsigned jogPunzoneFineCorsaMinStatus : 1;

    unsigned unusedBits : 23; // = 32 - n° bit occupati dagli altri membri
};

/// Struct di inizializzazione (evita errori nell'utilizzo di membri non inizializzati)
extern const EventQueueHMI_t defaultEventQueueHMI;

#pragma endregion (EVENT_QUEUE)


extern BaseType_t checkUpdateHMI(CommandQueueHMI_t* receivingQueue); /// Direction: Frontend --> Backend
extern BaseType_t checkUpdateHMI(EventQueueHMI_t* receivingQueue);   /// Direction: Backend --> Frontend
extern BaseType_t sendUpdateHMI(CommandQueueHMI_t* sendingQueue, bool* sendChanges); /// Direction: Frontend --> Backend
extern BaseType_t sendUpdateHMI(EventQueueHMI_t* sendingQueue, bool* sendChanges);   /// Direction: Backend --> Frontend 

