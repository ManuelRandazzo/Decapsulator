#pragma once

/// Include della definizione di Sequence_t
#include "DecapsulatorPRG.hpp"


/*╔═══════════════════════════════════════════════════╗*/
/*║                       NOTIFY                      ║*/
/*╚═══════════════════════════════════════════════════╝*/
#define AUTOKILL_NOTIFY (1 << 31) // Ultimo bit della notifica

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
extern QueueHandle_t AutokillQueueHandler;
extern TaskHandle_t  MainPrgHandler;