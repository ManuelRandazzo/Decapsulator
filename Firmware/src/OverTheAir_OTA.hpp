#pragma once

/// Include la libreria custom per la gestione delle task
#include "Debug.hpp"

/// Libreria per la gestione del Debug e la configurazione del WiFi
#include "WiFi_Config.hpp"

/// Libreria che consente di modificare il codice Over The Air(OTA), tramite WiFi e senza il bisogno di fili
#include "ArduinoOTA.h"

/// Crea La task
TaskTypeDef OverTheAir;

void OTA_Setup()
{
    /// ATTENZIONE: NON ELIMINARE MAI DALLO SKETCH LA GESTIONE DELL'OTA O NON SARA' POSSIBILE CARICARE UNO SKETCH DA REMOTO
    /// Se il caricamento OTA fallisce provare a disattivare il firewall o verificare che sia la porta 3232
    /// (ho creato una regola sul mio computer del firewall che non dovrebbe dar più il problema)
    if(WiFi.status() != WL_CONNECTED)
	    startWiFi(5000); 
    ArduinoOTA.setHostname("Decapsulator");
    ArduinoOTA.setPassword(OTA_SOFTWARE_DOWNLOAD_PASSWORD);
    ArduinoOTA.setPort(3232);  //la porta è 3232
    ArduinoOTA.begin();
}



/**
 *  @note fare un menù del display TFT a parte che servirà solo per il bootloading dello sketch, così non satura il WiFi inutilmente
 */
void OTA_Loop()
{
	ArduinoOTA.handle();  //gestisce l'aggiornamento software OTA tramite il WiFi
}