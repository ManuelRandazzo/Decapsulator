#pragma once
// -*- Gruppo 1-FreeRTOS-C++

/**
 *  @fileinfo:
 *  In questo file viene gestita la parte di sicurezza dell'intero macchinario.
 *  La task ad altà priorità viene ciclata ogni Security_delay ( @def in @file Tasks.hpp )
 *  e in caso di bisogno blocca i motori staccandone l'alimentazione per evitare incidenti agli operatori.
 *  I motivi principali per cui viene attivata la safety sono :
 *       - Pulsante di emergenza (fungo) premuto
 *       - Macchinario aperto che costituisce pericolo per gli operatori
 *       - nFAULT Motore Tamburo --> Driver è andato in protezione per uno di questi motivi: 
 *                                   Overcurrent, Undervoltage, Overtemperature
 *       - nFAULT Motore Punzone --> Driver è andato in protezione per uno di questi motivi: 
 *                                   Overcurrent, Undervoltage, Overtemperature
 * 
 */

#include "Tamburo.hpp"


SemaphoreHandle_t flagSafety;

/// Handler per la dichiarazione, creazione e distruzione della SafetyTask
TaskHandle_t SafetyHandler = NULL;

/// Timeout dopo il quale il macchinario deve essere riavviato manualmente
#define TIMEOUT_FOR_MANUAL_RESTART_MS 10000


#define MACHINE_OPEN_PIN   255  /** @warning  Ancora da definire*/
#define EMERGENCY_STOP_PIN 255  /** @warning  Ancora da definire*/
#define nFAULT_TAMBURO     255  /** @warning  Ancora da definire*/
#define nFAULT_PUNZONE     255  /** @warning  Ancora da definire*/



void SafetyTask(void *pvParameters)
{
    /**
     *   @safety_task_setup:
     */
    if(Security_delay <= 0)
    {
        LogError("Safety Task Setup", "Security Delay è troppo piccolo,\n Modificare il programma.");
        LogInfo("Safety Task Setup", "Restart dell'ESP32 a causa di un errore nel Security Delay.\n Riavviando ...");
        delay(5000); // Delay per dare il tempo di inviare i logs
        ESP.restart();
    }

    /// Variabile per avviare il tempo di esecuzione corretto
    TickType_t getLastTick = xTaskGetTickCount();

    /// Pulsante di sicurezza a fungo per Emergency Stop
    pinMode(EMERGENCY_STOP_PIN, INPUT); /*!< EMERGENCY PIN : pulsante N.C. */

    /// Segnalatori di macchinario aperto per Emergency Stop
    pinMode(MACHINE_OPEN_PIN, INPUT); /*!< MACHINE OPEN PIN : pulsanti N.O. messi in AND da HW */

    /// nFAULT pin del driver del motore del tamburo (DRV8825) se va in Overcurrent, Undervoltage, Overtemperature
    pinMode(nFAULT_TAMBURO, INPUT);

    /// nFAULT pin del driver del motore del punzone (DRV8825) se va in Overcurrent, Undervoltage, Overtemperature
    pinMode(nFAULT_PUNZONE, INPUT);

    /// Flag che determina se la safety è triggerata
    bool SafetyTriggered = false;
    
    /// Flag che determina se la safety è triggerata
    bool NeedForManualRestart = false;

    /// Timemout dopo il quale il macchinario dovrà essere riavviato manualmente
    uint32_t Timeout = 0;

    /// Flag per bypassare la safety durante lo sviluppo
    bool bypass_safety = false;

    /// Forza tramite messaggio MQTT il flag per bypassare la safety
    DebugOverrideVar<bool>("bypass_safety_topic", &bypass_safety);

    /**
     *   @safety_task_loop:
     */
    while(bypass_safety == false)
    {
        String strReason = "";

        

        /**
         *  Si sblocca in automatico il macchinario quando è finito l'evento
         */
        if(digitalRead(nFAULT_TAMBURO) == LOW)
        {
            if(SafetyTriggered == false)
            {
                Timeout = millis();
                SafetyTriggered = true;
            }

            strReason += "nFAULT TAMBURO, ";
        }
        else if(SafetyTriggered == true)
            SafetyTriggered = false;

        if(digitalRead(nFAULT_PUNZONE) == LOW)
        {
            if(SafetyTriggered == false)
            {
                Timeout = millis();
                SafetyTriggered = true;
            }

            strReason += "nFAULT PUNZONE, ";
        }
        else if(SafetyTriggered == true)
            SafetyTriggered = false;

        if(digitalRead(MACHINE_OPEN_PIN) == LOW)
        {
            if(SafetyTriggered == false)
            {
                Timeout = millis();
                SafetyTriggered = true;
            }

            strReason += "MACCHINARIO APERTO, ";
        }
        else if(SafetyTriggered == true)
            SafetyTriggered = false;


        /**
         *  Serve riavvio manuale del macchinario
         */
        if(digitalRead(EMERGENCY_STOP_PIN) == LOW)
        {
            NeedForManualRestart = true;
            strReason += "PULSANTE D'EMERGENZA PREMUTO.\nE' richiesto il riavvio manuale del macchinario a causa del pulsante premuto";
        }
        
        if(SafetyTriggered)
        {            
            if(millis() - Timeout >= TIMEOUT_FOR_MANUAL_RESTART_MS)
                NeedForManualRestart = true;
            
            MainProgramEmergencyFunction();
            LogError("SAFETY TRIGGERED", "Entrato nello stato di emergenza causa: %s", strReason);
        }

        /// ATTENZIONE: Non ancora finita
        if(NeedForManualRestart == true)
        {   
            DecapsulatorHandleTask.Suspend();
            LogWarning("Safety Task", "Il macchinario deve essere riavviato manualmente perchè\
                                       \nper ragioni di sicurezza non può riavviarsi da solo.\nCausa del riavvio manuale : %s", strReason);
        }

        /// Delay della task
        xTaskDelayUntil(&getLastTick, Security_delay);
    }
}