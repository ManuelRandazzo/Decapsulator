#include "HMI_UI_EEZ/actions.h"
#include "HMI_UI_EEZ/vars.h"
#include "decapsulator_io.hpp"
#include "HMI_UI_EEZ/screens.h"
#include "tasks_cfg.hpp"
#include "filePathsSD.hpp"
#include "WiFi_Config.hpp"
#include "Debug.hpp"

void action_calibrazione_touch(lv_event_t *e)
{
    /// Refresh forzato
    lv_obj_invalidate(lv_screen_active());
    lv_refr_now(lv_obj_get_display(lv_screen_active()));

    /// Comunica alla UI che faccia la calibrazione del touch
    set_var_calibrazione_touch_finita(false);

    /// Background color
    constexpr uint32_t BG_COLOR = TFT_BLACK;

    /// Foreground color
    constexpr uint32_t FG_COLOR = TFT_RED;

    uint16_t calib_datas[5] = { 0, 0, 0, 0, 0 };    

    tft.calibrateTouch(calib_datas, FG_COLOR, BG_COLOR, 40);

    /// Comunica alla UI che la calibrazione del touch è terminata
    set_var_calibrazione_touch_finita(true);

    /// Imposta i dati di calibrazione ottenuti
    tft.setTouch(calib_datas);

    /// Scrive i dati di calibrazione nella SD
    for(uint8_t i=0; i < 5; i++)
        SD_Card.setValueByKey(HMI_TOUCH_CALIB_PATH_SD, "data[" + String(i) + "]", calib_datas[i]);
}




void action_logger_page_init(lv_event_t * e)
{
    /// Legge i log dalla microSD e li carica nella variabile della UI
    set_var_str_logger_txt(SD_Card.readFile(LOG_PATH_SD).c_str());
}




void action_logger_page_deinit(lv_event_t * e)
{

}




void action_jogger_page_init(lv_event_t * e)
{
    /// Sospende la task del programma principale e attiva quella del jogger
    vTaskSuspend(MainPrgHandler);
    vTaskResume(JogMotoriPrgHandler);

    /// Inizializza lo spazio percorso e la velocità dei motori di default
    set_var_gradi_per_click_ralla("360.0");
    set_var_speed_motore_ralla("90.0");
    set_var_gradi_per_click_punz("360.0");
    set_var_speed_motore_punz("90.0");

    /// Si assicura che sia accesa la ventola
    Ventola.on();
}




void action_jogger_page_deinit(lv_event_t * e)
{
    /// Sospende la task del jogger e attiva quella del programma principale
    vTaskSuspend(JogMotoriPrgHandler);
    vTaskResume(MainPrgHandler);

    /// Si assicura che sia spenta la ventola
    Ventola.off();
}




void action_conf_wi_fi_page_init(lv_event_t * e)
{
}




void action_conf_wi_fi_page_deinit(lv_event_t * e)
{

}




void action_refresh_logs(lv_event_t * e)
{
    /// Legge i log dalla microSD e li carica nella variabile della UI
    set_var_str_logger_txt(SD_Card.readFile(LOG_PATH_SD).c_str());
}




void action_verify_wi_fi(lv_event_t * e)
{
    /// Salva in SD il nuovo SSID e la nuova Password
    /** @todo : 
     *          1. Controllare se è possibile connettersi
     *             con le nuove credenziali.
     *          2. Restituire un bool alla UI -> true=connesso, false=credenziali errate.
     *          3. se è true salva le credenziali in microSD
     */

    WiFi.mode(WIFI_STA); //per evitare conflitti con le risorse del bluetooth

    String tmpSSID = ""; // Mettere get_var_nome_rete_inserito();
    String tmpPASS = get_var_password_rete_inserita();

    WiFi.begin(tmpSSID, tmpPASS);
    if(WiFi.waitForConnectResult(DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS) != WL_CONNECTED)
    {
        if(WIFI_SSID != "")
        {
            /// Prova a riconnettersi alla rete di prima
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            if(WiFi.waitForConnectResult(DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS) != WL_CONNECTED)
                return;
        }
    }

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