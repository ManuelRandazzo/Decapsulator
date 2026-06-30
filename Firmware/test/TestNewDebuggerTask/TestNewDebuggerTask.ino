#include "Debug.hpp"

TaskHandle_t LoggerHandler;

void setup()
{
    pinMode(35, OUTPUT);
    vTaskDelay(3000);

    /// Inizializza la task del logger
    BaseType_t xLogTaskOK = xTaskCreatePinnedToCore(LoggerTask, "task LOGGER", 12*1024, NULL, 1, &LoggerHandler, APP_CPU_NUM);
    
}

uint32_t checkLostPackets = 1;
uint32_t tmrBlink = 0;
void loop()
{
    const uint32_t MILLIS = millis();
    if(MILLIS - tmrBlink >= 500)
    {
        tmrBlink = MILLIS;
        digitalWrite(35, !digitalRead(35));
    }

    LogInfo("Info", "Act packet : %d\n", checkLostPackets++);
}
