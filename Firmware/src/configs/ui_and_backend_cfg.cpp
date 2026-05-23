#include "ui_and_backend_cfg.hpp"


/**
 * @brief Inizializza le code di trasmissione tra ui e backend
 * 
 * @return se sono state create con successo tutte le code
 */
BaseType_t hmi_queues_begin(void)
{
    if(QueueHandlerHMI_CMD == nullptr)
        /// @brief Handler della coda che riceve i comandi che arrivano dall'HMI
        QueueHandlerHMI_CMD = xQueueCreate(HMI_CMD_QUEUE_LEN, sizeof(CommandQueueHMI_t));

    if(QueueHandlerHMI_EVT == nullptr)    
        /// @brief Handler della coda che invia gli eventi (errori, avvisi, ecc.) all'HMI
        QueueHandlerHMI_EVT = xQueueCreate(HMI_EVT_QUEUE_LEN, sizeof(EventQueueHMI_t));

    BaseType_t status = QueueHandlerHMI_CMD != nullptr && QueueHandlerHMI_EVT != nullptr ? pdTRUE : pdFALSE;
    /// Se gli handlers non sono nullptr allora sono state create le code
    return status;
}


#pragma region (COMMAND_QUEUE)

/// @brief Handler della coda che riceve i comandi che arrivano dall'HMI
QueueHandle_t QueueHandlerHMI_CMD = nullptr;

constexpr CommandQueueHMI_t defaultCommandQueueHMI =
{
    .StartMachine = false,
    .StopMachine = false,
    .restartAfterContainerEmptied = false,
    
    /// JOG
    .jogPageActive = false,

    .jogStateCMD = NO_CMD,
    
    .jogRallaGradiPerClick = 0.0, /// Quanti gradi fa ad ogni click del pulsante
    .jogRallaSpeed = 0.0,         /// Velocità del jog in gradi al secondo
    
    .jogPunzoneGradiPerClick = 0.0, /// Quanti gradi fa ad ogni click del pulsante
    .jogPunzoneSpeed = 0.0,         /// Velocità del jog in gradi al secondo
    
    .unusedBits = 0,
};
#pragma endregion (COMMAND_QUEUE)




#pragma region (EVENT_QUEUE)

/// @brief Handler della coda che invia gli eventi (errori, avvisi, ecc.) all'HMI
QueueHandle_t QueueHandlerHMI_EVT = nullptr;

constexpr EventQueueHMI_t defaultEventQueueHMI ={
    .xErrorInitPunz = false,
    .xErrorInitRalla = false,
    .xErrorCapsIncastrata = false,


    /// JOG
    .jogRallaIsMoving = false,   /// Indica se il motore si sta muovendo
    .jogRallaCalibrationStatus = false,
    .jogPunzoneIsMoving = false, /// Indica se il motore si sta muovendo
    .jogPunzoneFineCorsaMaxStatus = false,
    .jogPunzoneFineCorsaMinStatus = false,

    .unusedBits = 0,
};
#pragma endregion (EVENT_QUEUE)




/**
 * @brief Cerca e aggiorna in caso trovi dei cambiamenti dall'HMI
 * 
 * @dir Direction: Frontend --> Backend
 * 
 * @param receivingQueue Struct che contiene i dati che vengono inviati dall'HMI
 * 
 * @internal Usa "QueueHandlerHMI_CMD" come handler per la coda
 * 
 * @return pdTRUE se è stato ricevuto un comando con successo
 * 
 * @return pdFALSE se è fallita
 */
BaseType_t checkUpdateHMI(CommandQueueHMI_t* receivingQueue)
{
    #ifdef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
        if(receivingQueue == NULL)
        {
            LogError("checkUpdateHMI HMI-->Backend", "Il parametro \"receivingQueue\" is a null pointer");
            return pdFALSE;
        }
    #endif

    return xQueueReceive(QueueHandlerHMI_CMD, &receivingQueue, TICKS_TO_WAIT_QUEUE_RECEIVE_HMI);
}


/**
 * @brief Cerca e aggiorna in caso trovi dei cambiamenti dall'HMI
 * 
 * @dir Direction: Backend --> Frontend
 * 
 * @param receivingQueue Struct che contiene i dati che vengono inviati dall'HMI
 * 
 * @internal Usa "QueueHandlerHMI_EVT" come handler per la coda
 * 
 * @return BaseType_t Queue Error
 */
BaseType_t checkUpdateHMI(EventQueueHMI_t* receivingQueue)
{
    #ifdef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
        if(receivingQueue == NULL)
        {
            LogError("checkUpdateHMI Backend-->HMI", "Il parametro \"receivingQueue\" is a null pointer");
            return pdFALSE;
        }
    #endif

    return xQueueReceive(QueueHandlerHMI_EVT, receivingQueue, TICKS_TO_WAIT_QUEUE_RECEIVE_HMI);
}


/**
 * @brief Aggiorna in caso vengano richiesti dei cambiamenti da segnalare al Backend
 * 
 * @dir Direction: Frontend --> Backend
 * 
 * @param sendingQueue Struct che contiene i dati che vengono inviati al Backend
 * 
 * @param sendChanges Flag che determina se deve essere inviata la queue.
 *                    Il flag viene abbassato se è stato inviato il messaggio.
 * 
 * @internal Usa "QueueHandlerHMI_CMD" come handler per la coda
 * 
 * @return BaseType_t Queue Error
 */
BaseType_t sendUpdateHMI(CommandQueueHMI_t* sendingQueue, bool* sendChanges)
{
    #ifdef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
        if(sendingQueue == NULL)
        {
            LogError("sendUpdateHMI HMI-->Backend", "Il parametro \"sendingQueue\" is a null pointer");
            return pdFALSE;
        }

        if(sendChanges == NULL)
        {
            LogError("sendUpdateHMI HMI-->Backend", "Il parametro \"sendChanges\" is a null pointer");
            return pdFALSE;
        }
    #endif


    BaseType_t err = pdTRUE;

    if(*sendChanges == true)
    {
        err = xQueueSend(QueueHandlerHMI_CMD, sendingQueue, TICKS_TO_WAIT_QUEUE_SEND_HMI);
        if(err == pdTRUE)
            *sendChanges = false; // abbassa il flag se l'invio è riuscito
    }

    return err;
}


/**
 * @brief Aggiorna in caso vengano richiesti dei cambiamenti da segnalare all'HMI
 * 
 * @dir Direction: Backend --> Frontend
 * 
 * @param sendingQueue Struct che contiene i dati che vengono inviati all'HMI
 * 
 * @param sendChanges Flag che determina se deve essere inviata la queue.
 *                    Il flag viene abbassato se è stato inviato il messaggio.
 * 
 * @internal Usa "QueueHandlerHMI_EVT" come handler per la coda
 * 
 * @return BaseType_t Queue Error
 */
BaseType_t sendUpdateHMI(EventQueueHMI_t* sendingQueue, bool* sendChanges)
{
    #ifdef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
        if(sendingQueue == NULL)
        {
            LogError("sendUpdateHMI Backend-->HMI", "Il parametro \"sendingQueue\" is a null pointer");
            return pdFALSE;
        }
        
        if(sendChanges == NULL)
        {
            LogError("sendUpdateHMI Backend-->HMI", "Il parametro \"sendChanges\" is a null pointer");
            return pdFALSE;
        }
    #endif

    BaseType_t err = pdTRUE;

    if(*sendChanges == true)
    {
        err = xQueueSend(QueueHandlerHMI_EVT, sendingQueue, TICKS_TO_WAIT_QUEUE_SEND_HMI);
        if(err == pdTRUE)
            *sendChanges = false; // abbassa il flag se l'invio è riuscito
    }

    return err;
}


