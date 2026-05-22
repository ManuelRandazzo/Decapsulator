// -*- Gruppo 1-FreeRTOS-C++

/**
 * This file is part of {{ Decapsulator }}.
 *
 * Developed for the ITS Meccatronico Montebelluna-Castelfranco Veneto 2024-2026 Gruppo 1.
 * This product includes software developed by the Gruppo 1 Project.
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is NOT free software: you can NOT redistribute it and/or modify it.
 *
 * The Documentation is made following the Doxygen standard documentation style
 *
 *
 * About Code and Libraries stable versions compatibility and Libraries Documentation:
 *
 * @c_plus_plus_reference_documentation_link: https://learn.microsoft.com/it-it/cpp/cpp/?view=msvc-170
 *
 * @freertos_reference_documentation: https://www.freertos.org/Documentation/00-Overview
 */

#include "Arduino.h" // Per il supporto da altri IDE (Integrated Development Environment)
#include "Tasks.hpp"
#include "tasks_cfg.hpp"
#include "decapsulator_io.hpp"
#include "ui_and_backend_cfg.hpp"
#include "Debug.hpp"
#include "OverTheAir_OTA.hpp"
#include "DecapsulatorPRG.hpp"
#include "JogMotoriPRG.hpp"
#include "HMI.hpp"

/**
 *  @brief il setupTask() crea le task e ritorna i log in caso di errore per tutte le task non create con successo 
 *
 *  @param void
 *
 *  @return pdPASS se è andata a buon fine - pdFAIL se c'è stato un errore
 */
BaseType_t setupTasks(void)
{
    BaseType_t status = pdPASS;

    status &= decapsulator_io_begin();
    
    status &= hmi_queues_begin();
    
    //status &= OverTheAir.Init("task OTA", OTA_heap, NULL, OTA_priority, OTA_delay, OTA_Setup, OTA_Loop);

    status &= xTaskCreatePinnedToCore(prgDecapsulatorTask, "task MAIN PROGRAM", MainPrg_heap, NULL, MainPrg_priority, &MainPrgHandler, APP_CPU_NUM);

    status &= xTaskCreatePinnedToCore(prgHMITask, "task HMI PROGRAM", HMI_heap, NULL, HMI_priority, &HMIPrgHandler, APP_CPU_NUM); 

    status &= xTaskCreatePinnedToCore(prgJogMotoriTask, "task JOG MOTORI PROGRAM", JogMotori_heap, NULL, JogMotori_priority, &JogMotoriPrgHandler, APP_CPU_NUM);

    

    /// Restituisce lo stato generale di errore di almeno una delle task, comunque ci sono i log
    return status;
}

/**
 *  @brief Setup per la definizione delle task e inizializzazione dei componenti
 */
void setup()
{
    LogBegin();

    /// crea le task, superato il timeout restarta l'esp32
    const uint32_t tmoSetupTask = millis();
    while(!setupTasks())
    {
        if(millis() - tmoSetupTask >= 1000)
        {
            LogError("setup", "Impossibile creare le task nel tempo specificato");
            ESP.restart();
        }
    }

    LogInfo("setup", "create le tasks - Free Stack Space: %d", uxTaskGetStackHighWaterMark(NULL));
}

/**
 *  @brief loop principale
 *        
 *  @note Non serve perchè le tasks vengono gestite dallo scheduler
 *
 *  @attention Non eliminare la task perchè serve al WiFi
 * 
 */
void loop()
{
    
}
