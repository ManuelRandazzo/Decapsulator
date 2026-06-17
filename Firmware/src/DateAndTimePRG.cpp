#include "Arduino.h"
#include "FreeRTOS.h"
#include "WiFi.h"
#include "time.h"
#include "tasks_cfg.hpp"
#include "HMI_UI_EEZ/vars.h"
#include "Debug.hpp"
#include "DateAndTimePRG.hpp"

void DateAndTimePRG(void* pvParameters)
{
    TickType_t getLastTick = xTaskGetTickCount();
 
    constexpr uint32_t UPDATE_RTC_MS = 30/* min */ * 60 * 1000;

    /// Forza il caricamento iniziale della data e ora
    uint32_t tmrUpdateRTC = millis() - UPDATE_RTC_MS;

    while(1)
    {
        set_var_presenza_wi_fi(WiFi.isConnected());
        
        const uint32_t MILLIS = millis();
        if(MILLIS - tmrUpdateRTC >= UPDATE_RTC_MS)
        {
            if(WiFi.isConnected())
            {
                configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "time.inrim.it", "pool.ntp.org");
                tmrUpdateRTC = MILLIS;
            }
        }

        tm time_info;
        getLocalTime(&time_info);
        std::string time_and_date;
        strftime(time_and_date.data(), sizeof(time_and_date), "%d/%m/%Y       %H:%M:%S", &time_info);
        set_var_date_time_string(time_and_date.c_str());
        
        LogDebug("RAM", "Free Stack Space: %d", DateAndTime_heap - uxTaskGetStackHighWaterMark(NULL));

        vTaskDelayUntil(&getLastTick, DateAndTime_delay);
    }
    vTaskDelete(NULL);
}