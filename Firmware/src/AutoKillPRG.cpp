#include "AutoKillPRG.hpp"
/// Include la libreria custom per la gestione delle Task
#include "tasks_cfg.hpp"
/// Include la libreria custom per il Debug
#include "Debug.hpp"
/// Include la libreria custom per la gestione dell'SD
#include "decapsulator_io.hpp"
/// Include il file che contiene i percorsi delle directori (paths) della SD
#include "filePathsSD.hpp"

/*╔════════════════════════════════════════════════════╗*/
/*║                 VARIABILI GLOBALI                  ║*/
/*╚════════════════════════════════════════════════════╝*/
/// Handle della coda usata per ricevere i dati dal MainPrg
/// in caso di evento di autokill (spegnimento improvviso)
QueueHandle_t AutokillQueueHandler = nullptr;




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
    AutokillQueueHandler = xQueueCreate(1, sizeof(DatasToSave));
    if(AutokillQueueHandler == nullptr)
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
            const uint32_t startTime = millis();
            Serial.printf("\033[1;36mAutokill.event() Triggerato avvio la Notifica di Autokill\033[0m");
            /// Accende il MOSFET della batteria
            digitalWriteFast(AUTOKILL_SHUTDOWN_PIN, HIGH);

            /// Svuota eventuali dati residui rimasti in coda da un
            /// precedente evento, cosi' da non leggere dati vecchi
            /// nel caso (anomalo) in cui non siano mai stati ritirati.
            xQueueReset(AutokillQueueHandler);

            /// Si assicura di essere nella task MainPrg
            vTaskSuspend(JogMotoriPrgHandler);
            vTaskResume(MainPrgHandler);

            /// Chiede i dati al Programma principale e aspetta che
            /// questo li mandi, con un timeout massimo per non
            /// restare bloccati a tempo indefinito.
            xTaskNotifyGive(MainPrgHandler);

            DatasToSave received;
            if(xQueueReceive(AutokillQueueHandler, &received, portMAX_DELAY) == pdPASS)
            {
                /// Salva i dati ricevuti dal MainPrg sulla SD
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "LastSequenza", (uint8_t)received.lastSequenza, "Ultima sequenza eseguita");
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "nCicliRimanenti", received.nCicliRimanenti, "Cicli rimanenti della sequenza");
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "StepRallaRimanenti", received.stepRimanentiRalla, "Step rimanenti del motore Tamburo");
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "StepPunzoneRimanenti", received.stepRimanentiPunzone, "Step rimanenti del motore Punzone");
                SD_Card.setValueByKey(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "CapsuleTotali", received.capsuleTotali, "Capsule totali prodotte");
            }

            /// Spegne il MOSFET della batteria
            digitalWriteFast(AUTOKILL_SHUTDOWN_PIN, LOW);
            const uint32_t stopTime = millis();
            Serial.printf("Tempo di Autokill : %d\n", stopTime - startTime);
            vTaskDelay(1000);
            ESP.restart(); // Forza il reset perchè non possiamo garantire la posizione dei motori (es. calo in cui lo stepper perde passi)
        }

        xTaskDelayUntil(&getLastTick, Autokill_delay);
    }

    /// Elimina la task qualora uscisse dal while(1)
    vTaskDelete(NULL);
}