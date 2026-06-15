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




/*╔═══════════════════════════════════════════════════╗*/
/*║                       STRUCT                      ║*/
/*╚═══════════════════════════════════════════════════╝*/
/// Struct dei dati da salvare
struct DatasToSave
{
    uint32_t var;
};




/*╔════════════════════════════════════════════════════╗*/
/*║                      EXTERNS                       ║*/
/*╚════════════════════════════════════════════════════╝*/
extern void AutoKillTask(void* pvParameters);

