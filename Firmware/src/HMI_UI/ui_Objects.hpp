/**
 * @brief File che contiene i define di tutti gli oggetti come bottoni, labels, ecc. 
 *        di tipo lv_obj_t dell'HMI e la funzione per vedere se è accaduto l'evento
 */

#include "ui.h"
#include "DecapsulatorPRG.hpp"

/**
 * @brief restituisce quando un oggetto dell'HMI genera un evento
 * 
 * @param cmd è la coda dell'HMI in quel ciclo
 * 
 * @param obj è l'oggetto che l'utente vuole controllare, ad esempio un bottone
 * 
 * @param evt_expected è l'evento che l'utente si aspetta per quello 
 *        specifico oggetto (obj) per eseguire una certa azione
 * 
 * @example
 *   @code
 *   if(evt_expc(FromHMI, START_BTN, LV_EVENT_PRESSED)
 *   {
 *      Decapsulator_start();
 *   }
 * 
 *   if(evt_expc(FromHMI, STOP_BTN, LV_EVENT_PRESSED)
 *   {
 *      Decapsulator_stop();
 *   }
 *   @endcode
 * 
 * @todo Probabilmente ha più senso gestire in una @class questa funzione (evt_expc)
 *       (così da non dover mettere ogni volta il cmd) e anche :
 *       BaseType_t checkUpdateHMI(CommandQueueHMI_t* receivingQueue); 
 *       BaseType_t checkUpdateHMI(EventQueueHMI_t* receivingQueue);
 *       BaseType_t sendUpdateHMI(CommandQueueHMI_t* sendingQueue, bool* sendChanges);
 *       BaseType_t sendUpdateHMI(EventQueueHMI_t* sendingQueue, bool* sendChanges);  
 */
bool evt_expc(CommandQueueHMI_t cmd, lv_obj_t obj, lv_event_code_t evt_expected)
{
    /// Se l'oggetto che è stato ricevuto dalla coda è 
    /// uguale a quello richiesto e è l'eento che 
    /// l'utente si aspettava allora ritorna true
    if(cmd.obj == obj)
    {
        if(cmd.evt == evt_expected)
            return true;
    }

    return false;
}


#pragma region (SCHERMATA PRINCIPALE)

#define START_BTN ui_Schermata_Principale_PULS_START
#define STOP_BTN ui_Schermata_Principale_PULS_STOP

#pragma endregion (SCHERMATA PRINCIPALE)


/**
 * @example Concept della classe per gestire la comunicazione HMI
 */
/*
class CommunicationHMI
{
    public :
        /// Si aspetta che le due code esistano già
        CommunicationHMI(QueueHandle_t QueueHandlerCMD = nullptr, QueueHandle_t QueueHandlerEVT = nullptr, TickType_t TicksToWaitRxTx = 0)
            : QueueHandlerCMD(QueueHandlerCMD), QueueHandlerEVT(QueueHandlerEVT), TicksToWaitRxTx(TicksToWaitRxTx)
        {}

        BaseType_t checkUpdateHMI()
        {
             #ifdef LOG_ACTIVE_QUEUE_TRANSFER_MAIN_PRG_AND_HMI
                 if(receivingQueue == NULL)
                 {
                     LogError("checkUpdateHMI HMI-->Backend", "Il parametro \"receivingQueue\" is a null pointer");
                     return pdFALSE;
                 }
             #endif
 
             return xQueueReceive(this->QueueHandler, &this->CmdQueue, this->TicksToWaitRxTx);
        }

        BaseType_t checkUpdateHMI(EventQueueHMI_t* receivingQueue);

        BaseType_t sendUpdateHMI(CommandQueueHMI_t* sendingQueue)
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

        BaseType_t sendUpdateHMI(EventQueueHMI_t* sendingQueue);
        
        
    private :
        CommandQueueHMI_t CmdQueue = defaultCommandQueueHMI;

        EventQueueHMI_t EvtQueue = defaultEventQueueHMI;

        /// @brief Handler della coda che riceve i comandi che arrivano dall'HMI
        QueueHandle_t QueueHandlerCMD = nullptr;

        /// @brief Handler della coda che riceve i comandi che partono dall'HMI
        QueueHandle_t QueueHandlerEVT = nullptr;

        TickType_t TicksToWaitRxTx = 0;
        
        CommandQueueHMI_t* lastSendedCMD = defaultCommandQueueHMI;
        EventQueueHMI_t* lastSendedEVT = defaultEventQueueHMI;
}*/