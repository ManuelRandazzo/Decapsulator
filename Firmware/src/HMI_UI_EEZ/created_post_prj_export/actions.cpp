#include "HMI_UI_EEZ/actions.h"
#include "HMI_UI_EEZ/vars.h"
#include "decapsulator_io.hpp"
#include "HMI_UI_EEZ/screens.h"
#include "tasks_cfg.hpp"
#include "filePathsSD.hpp"
#include "WiFi_Config.hpp"
#include "DateAndTimePRG.hpp"
#include "Debug.hpp"
#include "DecapsulatorPRG.hpp"

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
    String strCalibDatas;
    for(uint8_t i = 0; i < 5; i++)
    {
        strCalibDatas += SD_Card.setValueByKey(HMI_TOUCH_CALIB_PATH_SD, "data[" + String(i) + "]", calib_datas[i]);
        vTaskDelay(100);
    }

    LogInfo("New Touch Calib Datas", "Content of file %s :\n%s", HMI_TOUCH_CALIB_PATH_SD, strCalibDatas.c_str());
}




void action_logger_page_init(lv_event_t * e)
{
    xTaskNotify(LoggerHandler, SD_LOG_REFRESH, eSetBits);
}




void action_logger_page_deinit(lv_event_t * e)
{

}




void action_jogger_page_init(lv_event_t * e)
{
    /// Sospende la task del programma principale
    vTaskSuspend(MainPrgHandler);

    MotRalla.abortCurrentCommand();
    MotPunzone.abortCurrentCommand();
    MotRalla.detach();
    MotPunzone.detach();
    ServoParatia.write(SERVO_CLOSED_POS); /// Chiude il servo

    /// Attiva la task del Jogger
    vTaskResume(JogMotoriPrgHandler);

    /// Inizializza lo spazio percorso e la velocità dei motori di default
    set_var_gradi_per_click_ralla("270.0");
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

    MotRalla.attach();
    ServoParatia.attach(SERVO_PIN); // Riattiva il servomotore

    /// Deve rifare l'homing quando esce dal Jogger
    sequenza = MACHINE_STARTUP_STATE;

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
    xTaskNotify(LoggerHandler, SD_LOG_REFRESH, eSetBits);
}




void action_clean_logs(lv_event_t * e)
{
    xTaskNotify(LoggerHandler, SD_LOG_CLEAN, eSetBits);
}




void action_verify_wi_fi(lv_event_t * e)
{
    /// Salva in SD il nuovo SSID e la nuova Password

    /// Richiesta di update delle credenziali WiFi
    xTaskNotify(DateAndTimeHandler, TRY_TO_CONNECT_WITH_NEW_WIFI_CREDENTIALS, eSetBits);
}