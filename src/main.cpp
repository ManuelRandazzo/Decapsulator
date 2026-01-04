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

#include <Arduino.h> // Per il supporto da altri IDE (Integrated Development Environment)
#include "Tasks.hpp"
#include "Debug.hpp"
#include "Safety.hpp"
#include "OverTheAir_OTA.hpp"
#include "Tamburo.hpp"
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
  
  //status &= xTaskCreatePinnedToCore(SafetyTask, "task SAFETY", Security_heap, NULL, Security_priority, &SafetyHandler, APP_CPU_NUM);

  //status &= OverTheAir.Init("task OTA", OTA_heap, NULL, OTA_priority, OTA_delay, OTA_Setup, OTA_Loop);

  status &= xTaskCreatePinnedToCore(prgDecapsulatorTask, "task MAIN PROGRAM", MainPrg_heap, NULL, MainPrg_priority, &MainPrgHandler, APP_CPU_NUM);

  //status &= Contenitori.Init("task PESO CONTENITORI", PesoCont_heap, NULL, PesoCont_priority, BALANCE_DELAY_BETWEEN_READINGS);
  
  

  return status; //restituisce lo stato generale di errore di almeno una delle task, comunque ci sono i log
}

/**
 *  @brief Setup per la definizione delle task e inizializzazione dei componenti
 */
MOTION motoreIsolato;
bool xBusy = false;
void setup()
{
  LogBegin();

  /// crea le task
  //while(!setupTasks());

  LogDebug("setup", "create le tasks");
  drv_err_t err = motoreIsolato.Init(RALLA_MOTOR_STEPS, RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN,
                      RALLA_TASK_PRIORITY, RALLA_MICROSTEP);
  if(DRV_OK != err)
  {
    LogError("Motore test Init", "Resetto l'esp32.\nErrore : %s.", drv_err_to_name(err));
    delay(2000);
    ESP.restart();
  }

  motoreIsolato.attach();
  motoreIsolato.Start();
  
  //while(!motoreIsolato.isStepDone());
  /*motoreIsolato.moveRel(-270.0, 900.0);
  while(!motoreIsolato.isStepDone());*/
  motoreIsolato.moveContinuous(DIR_POSITIVE, 1000.0);
  vTaskDelay(pdMS_TO_TICKS(5000));
  motoreIsolato.Halt();
  //vTaskSuspend(NULL);
}

/**
 *  @brief loop principale
 *        
 *  @note Non serve perchè le tasks vengono gestite dallo scheduler
 *
 *  @attention Non eliminare la task perchè serve al WiFi
 * 
 */
bool toggle = false;
void loop() {
  if(motoreIsolato.isStepDone())
  {
    for(int i=0; i<4; i++)
    {
      if(!toggle)
        motoreIsolato.moveRel(-270.0, 1400.0);
      else
        motoreIsolato.moveRel(+270.0, 1400.0);

      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    toggle ^= 1;
  }
}
