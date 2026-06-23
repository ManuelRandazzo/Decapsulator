#include "Arduino.h"
#include "FreeRTOS.h"
#include "WiFi.h"
#include "time.h"
#include "tasks_cfg.hpp"
#include "HMI_UI_EEZ/vars.h"
#include "Debug.hpp"
#include "DateAndTimePRG.hpp"
#include "R_TRIG.hpp"

void DateAndTimePRG(void* pvParameters)
{
    TickType_t getLastTick = xTaskGetTickCount();
 
    constexpr uint32_t UPDATE_RTC_MS = 30/* min */ * 60 * 1000;

    /// Forza il caricamento iniziale della data e ora
    uint32_t tmrUpdateRTC = millis() - UPDATE_RTC_MS;

    R_TRIG WiFiJustConnected;

    while(1)
    {
        bool WiFiConnected = WiFi.isConnected();
        WiFiJustConnected.CLK(WiFiConnected);
        set_var_presenza_wi_fi(WiFiConnected);
        
        const uint32_t MILLIS = millis();
        uint32_t ulNotifiedValue = 0;
        BaseType_t xHasBeenNotified = xTaskNotifyWait(0, ULONG_MAX, &ulNotifiedValue, 0);
        if(WiFiJustConnected.Q() || MILLIS - tmrUpdateRTC >= UPDATE_RTC_MS || (xHasBeenNotified == pdPASS && ulNotifiedValue == DATE_TIME_FORCE_UPDATE))
        {
            LogDebug("UPDATE RTC from NTP Server", "Trying to update RTC");
            if(WiFi.isConnected())
            {
                configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "time.inrim.it", "pool.ntp.org");
                tmrUpdateRTC = MILLIS;
            }
        }

        tm time_info;
        getLocalTime(&time_info);
        char time_and_date[30];
        strftime(time_and_date, sizeof(time_and_date), "%d/%m/%Y       %H:%M:%S", &time_info);
        set_var_date_time_string(time_and_date);
        
        vTaskDelayUntil(&getLastTick, DateAndTime_delay);
    }
    vTaskDelete(NULL);
}