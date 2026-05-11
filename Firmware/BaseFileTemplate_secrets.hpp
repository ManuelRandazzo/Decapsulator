/**
 * 
 *   @brief Questo file permette di poter avere impostare le credenziali
 *          del proprio WiFi con la relativa password.
 * 
 *   ATTENZIONE: Questo determinato file VIENE SALVATO SU GITHUB perchè è il modello base.
 *               Per poter usare nel modo corretto copiare questo file e rinominarlo 
 *               "local_secrets.hpp", metterlo nella directory @dir Decapsulator_PRG/src/ 
 *               e definire un SSID e una Password validi.
 *               È anche scegliere se usare l'hotspot come connessione
 *               
 */

#define __HIDDEN_PROGRAMMER_INITIAL_WIFI_SSID "DefinireUnSSID"    
#define __HIDDEN_PROGRAMMER_INITIAL_WIFI_PASSWORD "DefinireUnaPassword"


//#define HOTSPOT_CONNECTION

#ifndef HOTSPOT_CONNECTION
    #define __HIDDEN_PROGRAMMER_INITIAL_WIFI_SSID "DefinireUnSSID"    
    #define __HIDDEN_PROGRAMMER_INITIAL_WIFI_PASSWORD "DefinireUnaPassword"
#else
    #define __HIDDEN_PROGRAMMER_INITIAL_WIFI_SSID "DefinireUnSSID HOTSPOT"    
    #define __HIDDEN_PROGRAMMER_INITIAL_WIFI_PASSWORD "DefinireUnaPassword HOTSPOT"
#endif