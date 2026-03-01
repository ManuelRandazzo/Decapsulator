/**
 *    @brief FILE CONTENENTE LE PASSWORD DEL WIFI E LA PASSWORD PER IL BOOT DEL CODICE (OTA - Over The Air)
 */
#pragma once

/// Include la libreria per la gestione del WiFi
#include "Arduino.h"
#include <WiFi.h>

/// Include il file in cui ci sono i dati del WiFi e la relativa password 
/// del programmatore (salvato solo nel PC in locale e non su github, escluso da .gitignore)
#include "local_secrets.hpp"

#define DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS 4000


/// Test Mosquitto MQTT broker, @see https://test.mosquitto.org
#define MQTT_BROKER "mqtt://test.mosquitto.org:1883"

/// Si può modificare dal display (tastiera virtuale usando LVGL, libreria "lvgl" esempio "keyboard"), ma di default sono così, veranno salvati su un file da leggere all'inizio.
/// Bisogna dichiararla inline perchè se no viene fatta una copia in tutti i file che includono WiFi_secrets.h, producendo errori
String WIFI_SSID = __HIDDEN_PROGRAMMER_INITIAL_WIFI_SSID;
String WIFI_PASSWORD = __HIDDEN_PROGRAMMER_INITIAL_WIFI_PASSWORD;
extern String WIFI_SSID;
extern String WIFI_PASSWORD;

#define __HIDDEN_OTA_SOFTWARE_DOWNLOAD_PASSWORD "StartOTADownload"
#define OTA_SOFTWARE_DOWNLOAD_PASSWORD __HIDDEN_OTA_SOFTWARE_DOWNLOAD_PASSWORD

/// Decapsulator ESP MQTT logging "Super User Do" e "AuthKey" 
#define __HIDDEN_PASSWORD_ESP_MQTT_FOR_DETAILED_LOGS "DecapsulatorSudo2426mb"