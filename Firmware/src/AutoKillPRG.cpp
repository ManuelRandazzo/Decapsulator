#include "filePathsSD.hpp"
#include "AutoKillPRG.hpp"

/*╔════════════════════════════════════════════════════╗*/
/*║                 VARIABILI GLOBALI                  ║*/
/*╚════════════════════════════════════════════════════╝*/
/// Handle della coda usata per ricevere i dati dal MainPrg
/// in caso di evento di autokill (spegnimento improvviso)
QueueHandle_t autokillQueueHandler = nullptr;


static const TickType_t AUTOKILL_RESPONSE_TIMEOUT = pdMS_TO_TICKS(200);




/*╔════════════════════════════════════════════════════╗*/
/*║                     AUTOKILL TASK                  ║*/
/*╚════════════════════════════════════════════════════╝*/
/// Task che monitora il pin di autokill e, quando rileva
/// un'interruzione di alimentazione, richiede gli ultimi
/// dati utili al MainPrg e li salva su SD prima che la
/// batteria venga effettivamente scollegata.
void AutoKillTask(void* pvParameters)
{
    TickType_t getLastTick = xTaskGetTickCount();

    /// Crea la coda per ricevere i dati dal MainPrg.
    /// Lunghezza 1: serve solo l'ultimo (e unico) set di dati
    /// richiesto al momento dell'evento di autokill.
    autokillQueueHandler = xQueueCreate(1, sizeof(DatasToSave));
    if(autokillQueueHandler == nullptr)
    {
        LogError("AUTOKILL QUEUE", "Impossibile creare la coda per l'autokill");

        /// Senza la coda la task non puo' funzionare correttamente:
        /// la sospendo per evitare comportamenti indefiniti piu'
        /// avanti (es. xQueueReceive su handle nullo).
        vTaskSuspend(NULL);
    }

    while(1)
    {
        /// Aggiorna lo stato del pin dell'autokill
        autoKill.intrUpdate();

        /// Se ha ricevuto l'evento allora invia le notifiche
        /// alle task per ricevere i loro dati
        if(autoKill.event())
        {
            /// Accende il MOSFET della batteria
            digitalWriteFast(AUTOKILL_SHUTDOWN_PIN, HIGH);

            /// Svuota eventuali dati residui rimasti in coda da un
            /// precedente evento, cosi' da non leggere dati vecchi
            /// nel caso (anomalo) in cui non siano mai stati ritirati.
            xQueueReset(autokillQueueHandler);

            /// Chiede i dati al Programma principale e aspetta che
            /// questo li mandi, con un timeout massimo per non
            /// restare bloccati a tempo indefinito.
            xTaskNotifyGive(MainPrgHandler);

            DatasToSave received;
            if(xQueueReceive(autokillQueueHandler, &received, AUTOKILL_RESPONSE_TIMEOUT) == pdPASS)
            {
                /// Salva i dati ricevuti dal MainPrg sulla SD
                /// reale definito in filePathsSD.hpp
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "LastSequenza", (uint8_t)received.lastSequenza, "Ultima sequenza eseguita");
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "nCicliRimanenti", received.nCicliRimanenti, "Cicli rimanenti della sequenza");
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "StepRallaRimanenti", received.stepRimanentiRalla, "Step rimanenti del motore Ralla");
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "StepPunzoneRimanenti", received.stepRimanentiPunzone, "Step rimanenti del motore Punzone");
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "CapsuleTotali", received.capsuleTotali, "Capsule totali prodotte");
            }
            else
            {
                /// Il MainPrg non ha risposto in tempo: si procede
                /// comunque allo spegnimento, ma viene loggato
                /// l'errore per poterlo diagnosticare in seguito.
                LogError("AUTOKILL", "Timeout: il MainPrg non ha risposto in tempo, dati non salvati");
            }

            /// Spegne il MOSFET della batteria
            digitalWriteFast(AUTOKILL_SHUTDOWN_PIN, LOW);
        }

        xTaskDelayUntil(&getLastTick, Autokill_delay);
    }

    /// Elimina la task qualora uscisse dal while(1)
    vTaskDelete(NULL);
}