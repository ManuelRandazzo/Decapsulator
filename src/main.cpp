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
 *
 * ESP32 Library --> @version 3.3.0 --> @link: https://github.com/espressif/arduino-esp32
 *
 * DRV8825 Stepper Driver --> @version 0.2.1 --> @link: https://github.com/RobTillaart/DRV8825
 *
 * HX711 ADC 24 bit per celle di carico --> @version 0.7.5 --> @link: https://github.com/bogde/HX711/blob/master
 */

#include "Arduino.h" // Per il supporto da altri IDE (Integrated Development Environment)
#include "Tasks.hpp"
#include "Debug.hpp"
#include "Safety.hpp"
#include "OverTheAir_OTA.hpp"
#include "DecapsulatorPRG.hpp"
#include "Contenitori.hpp"
#include "TFT_Display.hpp"

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
  
  //status &= xTaskCreatePinnedToCore(SafetyTask, "task SAFETY", Security_heap, NULL, Security_priority, &SafetyHandler, PRO_CPU_NUM);

  //status &= OverTheAir.Init("task OTA", OTA_heap, NULL, OTA_priority, OTA_delay, OTA_Setup, OTA_Loop);

  status &= xTaskCreatePinnedToCore(prgDecapsulatorTask, "task MAIN PROGRAM", MainPrg_heap, NULL, MainPrg_priority, &MainPrgHandler, APP_CPU_NUM);

  //status &= Contenitori.Init("task PESO CONTENITORI", PesoCont_heap, NULL, PesoCont_priority, BALANCE_DELAY_BETWEEN_READINGS);
  
  

  return status; //restituisce lo stato generale di errore di almeno una delle task, comunque ci sono i log
}

/**
 *  @brief Setup per la definizione delle task e inizializzazione dei componenti
 */
void setup()
{  
  LogBegin();

  /// crea le task, superato il timeout riaccende l'esp
  const uint32_t tmoSetupTask = millis();
  while(!setupTasks())
  {
    if(millis() - tmoSetupTask >= 1000)
      ESP.restart();
      
    vTaskDelay(100);
  }

  LogDebug("setup", "create le tasks");
}

/**
 *  @brief loop principale
 *        
 *  @note Non serve perchè le tasks vengono gestite dallo scheduler
 *
 *  @attention Non eliminare la task perchè serve al WiFi
 * 
 */
void loop() {}
