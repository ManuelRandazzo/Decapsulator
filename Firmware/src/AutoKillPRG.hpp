#pragma once

/// Include la libreria custom per la gestione delle Task
#include "tasks_cfg.hpp"
/// Include la libreria custom per il Debug
#include "Debug.hpp"
/// Include la libreria custom per la gestione dell'SD
#include "decapsulator_io.hpp"
/// Include il file contenente SSID e la PASSWORD dell'Utente 
#include "WiFi_Config.hpp"
/// Include il file che contiene i percorsi delle directori (paths) della SD
#include "filePathsSD.hpp"
/// Include della definizione di Sequence_t
#include "DecapsulatorPRG.hpp"




/*╔═══════════════════════════════════════════════════╗*/
/*║                       STRUCT                      ║*/
/*╚═══════════════════════════════════════════════════╝*/
/// Struct dei dati da salvare
struct DatasToSave
{
    Sequence_t lastSequenza;
    uint8_t    nCicliRimanenti;
    uint64_t   stepRimanentiRalla;
    uint64_t   stepRimanentiPunzone;
    uint16_t   capsuleTotali;
};




/*╔════════════════════════════════════════════════════╗*/
/*║                      EXTERNS                       ║*/
/*╚════════════════════════════════════════════════════╝*/
extern void AutoKillTask(void* pvParameters);
extern QueueHandle_t autokillQueueHandler;
extern TaskHandle_t  MainPrgHandler;