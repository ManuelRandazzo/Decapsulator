#pragma once

/// Include la libreria custom per la gestione delle task
#include "Tasks.hpp"

/// Libreria per la gestione del Debug e la configurazione del WiFi
#include "WiFi_secrets.hpp"

/// Libreria che consente di modificare il codice Over The Air(OTA), tramite WiFi e senza il bisogno di fili
#include "ArduinoOTA.h"

/// flag che permette di far schedulare la task e farla eseguire
SemaphoreHandle_t executeOTA_flag;

/// Crea La task
TaskTypeDef OverTheAir;

void OTA_Setup()
{
  /// ATTENZIONE: NON ELIMINARE MAI DALLO SKETCH LA GESTIONE DELL'OTA O NON SARA' POSSIBILE CARICARE UNO SKETCH DA REMOTO
  /// Se il caricamento OTA fallisce provare a disattivare il firewall o verificare che sia la porta 3232
  /// (ho creato una regola sul mio computer del firewall che non dovrebbe dar più il problema)
  if(WiFi.status() != WL_CONNECTED)
    startWiFi(); 
  ArduinoOTA.setHostname("Decapsulator");
  ArduinoOTA.setPassword(OTA_SOFTWARE_DOWNLOAD_PASSWORD);
  ArduinoOTA.setPort(3232);  //la porta è 3232
  ArduinoOTA.begin();

  /// Crea un semaforo che permette all'OTA di essere chiamato quando serve
  executeOTA_flag = xSemaphoreCreateBinary();
}



/**
 *  @note fare un menù del display TFT a parte che servirà solo per il bootloading dello sketch, così non satura il WiFi inutilmente
 */

void OTA_Loop()
{
  /// Task non schedulata finchè non viene ricevuto il flag
  xSemaphoreTake(executeOTA_flag, portMAX_DELAY);

  ArduinoOTA.handle();  //gestisce l'aggiornamento software OTA tramite il WiFi

  /// Rilascia il semaforo
  xSemaphoreGive(executeOTA_flag);

  //WiFi.mode(WIFI_OFF);  //per evitare conflitti di risorse del Bluetooth
}