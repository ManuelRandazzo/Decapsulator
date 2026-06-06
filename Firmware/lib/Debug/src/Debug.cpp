#include "ESP32MQTTClient.h"   /// Comunicazione MQTT 
#include "esp_idf_version.h"   /// Serve per il client MQTT per gli handle 
#include "SavingFilesSD.hpp"   /// Include la libreria per la gestione della SD Card
#include "WiFi_Config.hpp"     /// File contenente ssid e la password dell'Utente 
#include "Debug.hpp"

QueueHandle_t LoggerQueueHandler = nullptr;

#if defined(LOG_ACTIVE) && defined(LOG_ACTIVE_MQTT)
    ESP32MQTTClient mqttClient; // Oggetto di tipo Client MQTT
#endif

#if !defined(WiFi_ACTIVE)
    #pragma message ("Warning: OTA DOWNLOAD E MQTT LOG NON DISPONIBILI --> WiFi Disattivato nelle impostazioni")
#endif

/**
 * @brief Permette di cambiare l' "Interrupt Watchdog Timer" timeout in ms
 *
 * @param wdt_ms determina il timeout in millisecondi del INT_WDT
 */
static void changeWDT(uint32_t wdt_ms)
{
    esp_task_wdt_config_t tmr_cfg { .timeout_ms = wdt_ms };
    esp_task_wdt_reconfigure(&tmr_cfg);
}

/**
 * @brief Permette di restartare l'ESP32 con dei Log
 *
 * @param reason_of_restart_string Stampa in seriale anche una stringa che spiega
 *                                 la ragione del restart
 */
static void restartESP32(const char* reason_of_restart_string)
{
    String rst_rsn(reason_of_restart_string);

    /// Colore dell'errore nel logger
    Serial.print(BASE_ERROR);

    if(!rst_rsn.equals(""))
        Serial.print(rst_rsn.c_str());
    
    if(!rst_rsn.endsWith("\n"))
        Serial.print(", ");

    Serial.print("riavvio in...3");
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.print("...2");
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.println("...1\n\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    /// Toglie il colore dell'errore nel logger
    Serial.print(RST_COLOR);

    ESP.restart();
}


/**
 *  @brief Inizializza il WiFi
 *
 *  @note se è già inizializzato non esegue nulla
 */
void startWiFi(uint32_t timeout_for_each_initialization_ms)
{
    /// Debug
    uint32_t startTime = millis();


    #if defined(LOG_ACTIVE) && defined(LOG_ACTIVE_MQTT)
        /// Avvio del broker MQTT sulla porta 1883
        mqttClient.enableDebuggingMessages();

        /// Aggiungere Username e password contenuti in @file WiFi_secrets.hpp
        mqttClient.setURI(MQTT_BROKER); 
        mqttClient.enableLastWillMessage("brokerStatus", "offline");
        mqttClient.setKeepAlive(60);
        mqttClient.setAutoReconnect(true);
    #endif

    WiFi.mode(WIFI_STA); //per evitare conflitti con le risorse del bluetooth
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setHostname("Decapsulator");
    Serial.printf("WiFi Connect - Connessione Wi-Fi\n\n");

    /// Timeout che se supera un certo tempo riavvia l'ESP (se configurato per farlo)
    while(WiFi.waitForConnectResult(DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS) != WL_CONNECTED)
    {
        /// Se non è definito il NO_ESP_RESTART_ON_CONNECTION_FAILURE allora rebootta
        #ifndef NO_ESP_RESTART_ON_CONNECTION_FAILURE
            restartESP32("WiFI CONNECTION ERROR TIMEOUT");
        #else
            Serial.printf("WiFi Connect - CONNECTION ERROR TIMEOUT.\nProceeding with program...\n\n");
            /// Debug
            uint32_t endTime = millis();
            Serial.printf("Tempo StartUp - Delta Time WiFi Startup : %d\n\n", endTime - startTime);
            return;
        #endif
    }
    Serial.printf("WiFi Connect - WiFi Connesso!\n\n");
    
    #if defined(LOG_ACTIVE) && defined(LOG_ACTIVE_MQTT)
        /// Timeout che se supera un certo tempo riavvia l'ESP (se configurato per farlo)
        mqttClient.loopStart();
        uint32_t ESP_timeoutRestart = millis();
        while(!mqttClient.isConnected())
        {
            Serial.printf("MQTT Client Connect - ErrorMqttConnection\n\n");
            vTaskDelay(pdMS_TO_TICKS(100));
            /// Se sfora il timeout allora restarta l'ESP32
            if(millis() - ESP_timeoutRestart >= DEFAULT_TIMEOUT_WIFI_CONNECTION_IN_MS)
            {
                /// Se non è definito il NO_ESP_RESTART_ON_CONNECTION_FAILURE allora rebootta
                #ifndef NO_ESP_RESTART_ON_CONNECTION_FAILURE
                    Serial.printf("MQTT Client Connect - CONNECTION ERROR TIMEOUT.\nRebooting...\n\n");
                    ESP.restart();
                #else
                    Serial.printf("MQTT Client Connect - CONNECTION ERROR TIMEOUT.\nProceeding with program...\n\n");
                    /// Debug
                    uint32_t endTime = millis();
                    Serial.printf("Tempo StartUp - Delta Time WiFi Startup : %d\n\n", endTime - startTime);
                    return;
                #endif
            }

        }
        Serial.printf("MQTT Client Connect - MQTT Connesso!\n\n");

        /// Printa info sull'IP del dispositivo e sul broker MQTT in cui si è connessi
        Serial.printf("WiFi Connect - WiFi connesso: %s\nBroker MQTT: %s\n\n", WiFi.localIP().toString().c_str(), MQTT_BROKER);
    #else
        /// Printa info sull'IP del dispositivo
        Serial.printf("WiFi Connect - WiFi connesso: %s\n\n", WiFi.localIP().toString().c_str());  
    #endif
   
    /// Debug
    uint32_t endTime = millis();
    Serial.printf("Tempo StartUp - Delta Time WiFi Startup : %d\n\n", endTime - startTime);
}



/**
 *  @brief Inizializza sia il logger (in preprocessor) che il WiFi (runtime)
 *
 *  @param timeout_for_each_initialization_ms è il timeout che ogni 
 *         inizializzazione (ex. WiFi.begin(), mqtt.connect()), deve rispettare.
 *
 *  @return tempo dal serial begin per poter aspettare che sia disponibile la seriale
 *         
 *  @note il timeout totale della funzione è derivato dalla moltiplicazione tra 
 *        @param timeout_for_each_initialization_ms e il numero di inizializzazioni
 *        che devono essere eseguite: ex. n_inizializzazioni = 2, fTimeoutTot = n_inizializzazioni * timeout = 2*timeout
 */
const uint32_t LogBegin(uint32_t timeout_for_each_initialization_ms)
{
    #ifdef LOG_ACTIVE
        /// Limita il baud al massimo consentito per la Serial port di Arduino IDE
        Serial.begin(115200); 
        
        /// Permette i log dalla WiFi Library
        Serial.setDebugOutput(true);    

        /// Delay per aspettare che la seriale sia inizializzata
        const uint32_t timeOfSerialBegin = millis();

        LoggerQueueHandler = xQueueCreate(LOGGER_QUEUE_LEN, sizeof(log_msg_t));
        if(LoggerQueueHandler == nullptr)
            restartESP32("Errore nella creazione della QUEUE del logger");
    
        #ifdef WiFi_ACTIVE
            /// Porta il watchdog interrupt timer ad un valore più alto per evitare reset involontari in inizializzazione
            changeWDT(timeout_for_each_initialization_ms * 10);

            /// Inizializza il WiFi @ref @file "WiFi_secrets.hpp"
            startWiFi(timeout_for_each_initialization_ms);

            /// Riporta il watchdog interrupt timer al valore di partenza
            changeWDT(CONFIG_ESP_INT_WDT_TIMEOUT_MS);
        #endif

        return timeOfSerialBegin;

    #endif
        
    return 0;
}




/**
 *
 *    OVERRIDE SECTION
 *
 */


/// @link alle funzioni std:: usate --> https://en.cppreference.com/w/cpp/string/basic_string.html @sezione: numeric conversion
template<typename varType>
static varType FromStringToVarType(const std::string &s)
{    
    if constexpr (is_same_v<varType, string>)
        return s;
    if constexpr (is_same_v<varType, String>)
        return String(s.c_str());
    else if constexpr (is_same_v<varType, bool>)
    {
        string boolVal = s;
        /// No case sensitive ---> @link soluzione di tolower: https://stackoverflow.com/questions/313970/how-to-convert-an-instance-of-stdstring-to-lower-case
        transform(boolVal.begin(), boolVal.end(), boolVal.begin(), [](unsigned char c){ return tolower(c); });
        if(boolVal == "1" || boolVal == "true")  return true;
        if(boolVal == "0" || boolVal == "false") return false;
        // se la stringa non rappresenta un bool...
        return stoll(boolVal) != 0;
    } 
    else if constexpr (is_integral_v<varType> && is_signed_v<varType>)
    {
        /// Usa stoll per qualsiasi signed int
        long long v = stoll(s);        
        return static_cast<varType>(v);
    }
    else if constexpr (is_integral_v<varType> && is_unsigned_v<varType>)
    {
        /// Usa stoll per qualsiasi unsigned int
        unsigned long long v = stoull(s); // usa stoull per unsigned
        return static_cast<varType>(v);
    }
    else if constexpr (is_floating_point_v<varType>)
    {
        /// Usa stoll per qualsiasi floating point (float o double)
        long double v = stold(s);     // usa stold per float/double
        return static_cast<varType>(v);
    }

    return varType{};
}


/**
 *    MQTT Library Custom Functions
 */
#if defined(LOG_ACTIVE) && defined(LOG_ACTIVE_MQTT)

    /// Funzioni aggiuntive dell'MQTT Client
    void onMqttConnect(esp_mqtt_client_handle_t client){}

    void printError(esp_mqtt_error_codes_t *error_handle)
    {
        LogError("MQTT error handler", "Errore MQTT Handler : %d", error_handle);
    }

    /// In base alla versione installata di ESP IDF sceglie tra una o l'altra
    #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        esp_err_t handleMQTT(esp_mqtt_event_handle_t event)
        {
            mqttClient.onEventCallback(event);
            return ESP_OK;
        }
    #else  // IDF CHECK
        void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
        {
            auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
            mqttClient.onEventCallback(event);
        }
    #endif
#endif




/**
 * @brief Task Logger, task dedicata alla scrittura dei log in seriale, in MQTT e in SD
 */
void LoggerTask(void* pvParameters)
{
    #ifndef LOG_ACTIVE
        /// Elimina questa task se non ci sono i log attivi
        vTaskDelete(NULL);
    #endif

    LogBegin(LOGGER_BEGIN_INIT_TIMEOUT_MS);
    
    log_msg_t to_log;

    /// Delay tra la fine di un print e l'inizio di un altro
    uint32_t lastTime = 0;
    constexpr uint32_t LOG_DELAY_MS = 1000;

    /// Salvataggio in SD
    #ifdef LOG_COPY_TO_SD
        const String logPathSD = "/log.txt";
        constexpr uint8_t SD_BUFFER_LEN = 25;
        String bufferToSD[SD_BUFFER_LEN];
        uint8_t buffIndexSD = 0;
        uint32_t lastTimeWriteSD = 0;
        constexpr uint32_t LOG_DELAY_SD_MS = 10000;
        SD_Card.rmfile(logPathSD); /// Ad ogni accensione viene rimosso il file di log dalla SD
    #endif

    while(1)
    {
        while(millis() - lastTime <  LOG_DELAY_MS);

        /// Attende all'infinito che qualcuno invii un log
        xQueueReceive(LoggerQueueHandler, &to_log, portMAX_DELAY);

        //to_log.message.resize(strlen(to_log.message.c_str()));

        if(to_log.message != "")
        {
            /// Printa il messaggio
            log_printf(to_log.message.c_str());

            /// Invia il messaggio in MQTT
            #ifdef LOG_ACTIVE_MQTT
                if(mqttClient.isConnected())
                    mqttClient.publish(to_log.mqttTopic, to_log.message);
            #endif

            /// Copia il messaggio in SD
            #ifdef LOG_COPY_TO_SD
                const uint32_t MILLIS = millis();
                if(MILLIS - lastTimeWriteSD >= LOG_DELAY_SD_MS || buffIndexSD == SD_BUFFER_LEN)
                {
                    /// Salva il nuovo tempo
                    lastTimeWriteSD = MILLIS;

                    /// Salva tutti i log nel buffer alla fine del file di log e pulisce il buffer man mano
                    for(uint8_t i = 0; i < buffIndexSD; i++)
                    {
                        SD_Card.appendFile(logPathSD, bufferToSD[i]);
                        bufferToSD[i] = ""; /// Pulisce il buffer di stringhe
                    }

                    /// Resetta la testa del buffer
                    buffIndexSD = 0;
                }
                else
                    bufferToSD[buffIndexSD++] = to_log.message.c_str();

            #endif

            /**
            * 
            *  @todo Capire con Pisan per la parte di Diagnostica nel decapsulator per salvare i dati in SD
            *        sin da quando viene ricevuto il log o solo una volta quando viene spento oppure quando 
            *        xQueueReceive=false sfrutta il momento per inviare i log nel file
            *
            *  @todo Capire con Pisan per la parte di Diagnostica nel decapsulator per VISUALIZZARE gli ultimi
            *        5 o 10 logs nella UI. capire cosa invia l'opzione scrollable se manda una pagina oppure boh.
            *        Usare readFileRow() per ottenere una determinata riga, tenerne traccia, magari implementare
            *        una funzione che spezzetta le stringhe dei LOGS ogni volta che c'è uno '\n' e all'inizio di
            *        ogni riga scrivere il numero così basta partire dall'ultima riga (trovando il numero
            *        di riga con String.startWith()) e fare un ciclo che restituisca una stringa composta da 5-10 righe che verrà mandata 
            *        direttamente alla UI nella pagina dei LOGS.
            * 
            */

            /// Acquisisce il nuovo tempo
            lastTime = millis();
        }
    }
    vTaskDelete(NULL);
}