#include "AutoKillPRG.hpp"

QueueHandle_t autokillQueueHandler = nullptr;




void AutoKillTask(void* pvParameters)
{
    /// Crea la coda
    //autokillQueueHandler = xQueueCreate(1, )
    if(autokillQueueHandler == nullptr)
    {
        LogError("AUTOKILL QUEUE", "Impossibile creare la coda per l'autokill");
        //vTaskSuspend(NULL);
    }

    while(1)
    {
        /// Aggiorna lo stato del pin dell'autokill
        autoKill.intrUpdate();

        /// Se ha ricevuto l'evento allora invia le notifiche 
        /// alle task per riceve i loro dati
        if(autoKill.event())
        {
            /// Accende il MOSFET della batteria
            digitalWriteFast(AUTOKILL_SHUTDOWN_PIN, HIGH);

            /// Credenziali WiFi utente
            SD_Card.setValueByKey(pathWiFi, "SSID", WIFI_SSID, "Nome della rete dell'utente");
            SD_Card.setValueByKey(pathWiFi, "WIFI_PASSWORD", WIFI_PASSWORD, "Password della rete dell'utente");
            
            /*
            /// Chiede i dati al Programma principale e aspetta che questo li mandi
            xTaskNotifyGive(MainPrgHandler);
            xQueueReceive(autokillQueueHandler, &, portMAX_DELAY);

            SD_Card.setValueByKey("key1", received.value1);
            SD_Card.setValueByKey("key2", received.value2);
            SD_Card.setValueByKey("key3", received.value3);*/

            /// Spegne il MOSFET della batteria
            digitalWriteFast(AUTOKILL_SHUTDOWN_PIN, LOW);
        }
    }
}