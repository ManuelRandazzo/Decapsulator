/**
 *    @brief FILE CONTENENTE LE PASSWORD DEL WIFI E LA PASSWORD PER IL BOOT DEL CODICE (OTA - Over The Air)
 */
#pragma once

/// Include la libreria per la gestione del WiFi
#include "WiFi.h"

#define DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS 5000


#define __HIDDEN_PROGRAMMER_INITIAL_WIFI_SSID "*****"    
#define __HIDDEN_PROGRAMMER_INITIAL_WIFI_PASSWORD "*****"

/// Si può modificare dal display (tastiera virtuale usando LVGL, libreria "lvgl" esempio "keyboard"), ma di default sono così, veranno salvati su un file da leggere all'inizio.
/// Bisogna dichiararla inline perchè se no viene fatta una copia in tutti i file che includono WiFi_secrets.h, producendo errori
String WIFI_SSID = __HIDDEN_PROGRAMMER_INITIAL_WIFI_SSID;
String WIFI_PASSWORD = __HIDDEN_PROGRAMMER_INITIAL_WIFI_PASSWORD;
extern String WIFI_SSID;
extern String WIFI_PASSWORD;

#define __HIDDEN_OTA_SOFTWARE_DOWNLOAD_PASSWORD "StartOTADownload"
#define OTA_SOFTWARE_DOWNLOAD_PASSWORD __HIDDEN_OTA_SOFTWARE_DOWNLOAD_PASSWORD

/// Decapsulator ESP MQTT logging "Super User Do" e "AuthKey" 
#define __HIDDEN_PASSWORD_ESP_INSIGHTS_FOR_DETAILED_LOGS "SuperUserDoPassword"






