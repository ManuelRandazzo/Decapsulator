#include "Debug.hpp"

void runningTask(void *pvParameters)
{
  TickType_t getLastTick = xTaskGetTickCount();
  LogInfoISR("Init task", "Inizializzando...");
  for(;;)
  {
    LogWarning("Loop task", "Sto saturando per nulla la CPU");
    xTaskDelayUntil(&getLastTick, 3000);
  }
}

void setup()
{
    rmt_channel_t _rmtChannel;
  LogBegin();
 /* LogInfo("Balance Calibration", "Calibrazione HX711: \
                                  \nRimuovere tutto il peso dalla bilancia \
                                  \nDopo l'inizio delle letture, posizionare un peso noto sulla bilancia \
                                  \nPremere + o a per aumentare il fattore di calibrazione \
                                  \nPremere - o z per diminuire il fattore di calibrazione \
         ");
*/
  //esp_log_level_set("*", ESP_LOG_MAX);
  //InitSD_Card("MB", 1000, false);

  //TaskHandle_t task;
  //xTaskCreatePinnedToCore(runningTask, "name", 4096, nullptr, 1, &task, APP_CPU_NUM);

}

bool runOnce = true;
void loop()
{
  static uint32_t time;

  static int intero = 14;
  static float conVirgola = 3.14;
  static String stringaArduino = "Stringa arduino";
  static std::string stringaStdCPP = "std string c++";

  if(runOnce)
  { /// Setta tutti gli override
    DebugOverrideVar("int", &intero);
    DebugOverrideVar("float", &conVirgola);
    DebugOverrideVar("String", &stringaArduino);
    DebugOverrideVar("std::string", &stringaStdCPP);
  }

  if(millis() - time >= 1000)
  {
    time = millis();
    Serial.printf("Intero = %d\n", intero);
    Serial.printf("Float = %.2f\n", conVirgola);
    Serial.printf("String = %s\n", stringaArduino.c_str());
    Serial.printf("std::string = %s\n\n\n", stringaStdCPP.c_str());
  }

}
