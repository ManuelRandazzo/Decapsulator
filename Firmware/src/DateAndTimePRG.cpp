#include "Arduino.h"
#include "FreeRTOS.h"
#include "WiFi.h"
#include "time.h"
#include "tasks_cfg.hpp"
#include "HMI_UI_EEZ/vars.h"
#include "Debug.hpp"
#include "DateAndTimePRG.hpp"
#include "R_TRIG.hpp"
#include "WiFi_Config.hpp"
#include "HMI_UI_EEZ/vars.h"
#include "filePathsSD.hpp"
#include "decapsulator_io.hpp"

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

        while(xHasBeenNotified == pdPASS && ulNotifiedValue & TRY_TO_CONNECT_WITH_NEW_WIFI_CREDENTIALS)
        {
            LogDebug("Update WiFi Credentials", "Trying to update WiFi Credentials");

            /// Salva in SD il nuovo SSID e la nuova Password
            WiFi.mode(WIFI_STA); //per evitare conflitti con le risorse del bluetooth

            String tmpSSID = get_var_nome_rete_inserita();
            String tmpPASS = get_var_password_rete_inserita();
            if(tmpSSID == "")
            {
                set_var_wi_fi_success(0);
                
                /// Esce dal tentativo di provare a connettersi al WiFi
                break;
            }

            WiFi.disconnect(true);
            vTaskDelay(200);
            WiFi.begin(tmpSSID, tmpPASS);
            if(WiFi.waitForConnectResult(DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS) != WL_CONNECTED)
            {
                if(WIFI_SSID != "")
                {
                    WiFi.disconnect(true);
                    vTaskDelay(200);
                    /// Prova a riconnettersi alla rete di prima
                    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                    if(WiFi.waitForConnectResult(DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS) != WL_CONNECTED)
                        LogInfo("Credenziali WiFi", "Impossibile connettersi con queste credenziali del WiFi e nemmeno a quelle di prima");
                    else
                        LogInfo("Credenziali WiFi", "Impossibile connettersi con queste credenziali del WiFi, uso quelle di prima");
                    
                    set_var_wi_fi_success(0);

                    /// Esce dal tentativo di provare a connettersi al WiFi
                    break;
                }

                LogInfo("Credenziali WiFi", "Impossibile connettersi con queste credenziali del WiFi e nemmeno a quelle di prima");
                
                /// Esce dal tentativo di provare a connettersi al WiFi
                break;
            }
            
            set_var_presenza_errore(false);
            set_var_wi_fi_success(1);

            /// Richiesta di update dell'orologio una volta cambiate le credenziali WiFi
            ulNotifiedValue |= DATE_TIME_FORCE_UPDATE;

            /// Assegna le nuovi credenziali (se sono cambiate)
            if(tmpSSID != WIFI_SSID )
            {
                WIFI_SSID = tmpSSID;
                SD_Card.setValueByKey(WIFI_PATH_SD, "SSID", WIFI_SSID, "Nome della rete dell'utente");
            }

            if(tmpPASS != WIFI_PASSWORD)
            {
                WIFI_PASSWORD = tmpPASS;
                SD_Card.setValueByKey(WIFI_PATH_SD, "WIFI_PASSWORD", WIFI_PASSWORD, "Password della rete dell'utente");
            }

            LogInfo("Credenziali WiFi", "Cambiate le credenziali del WiFi");
        }

        if(WiFiJustConnected.Q() || MILLIS - tmrUpdateRTC >= UPDATE_RTC_MS || (xHasBeenNotified == pdPASS && ulNotifiedValue & DATE_TIME_FORCE_UPDATE))
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