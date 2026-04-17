#pragma once

#include "Arduino.h"
//#include "Debug.hpp"
#include "hal/gpio_ll.h"

#ifndef digitalReadFast
    /**
     * @brief Definisce una lettura del pin low level (gpio_ll_get_level ha l'attributo always inline) 
     * 
     * @param gpio_pin è il pin che verrà letto
     * 
     * @return gpio digital level
     */
    #define digitalReadFast(gpio_pin) gpio_ll_get_level(&GPIO, gpio_pin)
#endif


#define INTR (bool)(true)
#define POLL (bool)(false)



class DebPinHandler
{
    public :
        /**
         * @brief Costruttore del Debounce pin
         * @attention Non serve e non bisogna fare i pinMode dei pin, viene gestito tutto dal costruttore
         * 
         * @param IntrOrPoll    INTERRUPT oppure POLLING
         * @param pinNumber     Numero del pin di input
         * @param pinName       Nome del pin usato per debugging
         * @param debounce_ms   Tempo per il debounce in millisecondi
         * @param level_trigger Livello a cui viene triggerato il cambio di stato del pin
         * @param input_mode    Modalità di input del pin INPUT, INPUT_PULLUP, INPUT_PULLDOWN
         */
        DebPinHandler(bool IntrOrPoll, uint8_t pinNumber, const char* pinName = "No Pin Name",
                      uint32_t debounce_ms = 30, uint8_t level_trigger = CHANGE, uint8_t input_mode = INPUT);

        /// @brief Distruttore del Debounce pin
        ~DebPinHandler();

        /// @brief Dopo aver fatto il detach permette di ricollegare il pin con i dati impostati
        void reattach();

        /// @brief Disconnette il pin
        void detach();

        /**
         * @return true se il debounce ha dato esito positivo 
         *         false se negativo
         */
        bool event();

        /**
         * @brief  Aggiorna l'istanza e fa il debounce in INTERRUPT
         * @return Se è avvenuto o no un cambio di stato del pin
         * @attention E' solo per INTERRUPT:
         */
        bool intrUpdate();

        /**
         * @brief  Aggiorna l'istanza e fa il debounce in POLLING
         * @param  trigger RISING, FALLING, CHANGE 
         * @return Se è avvenuto o no un cambio di stato del pin
         * @attention E' solo per POLLING:
         */
        bool pollUpdate(uint8_t trigger = CHANGE);

        /**
         * @brief Restituisce il valore del pin senza debounce
         * 
         * @return HIGH se il livello è alto, LOW se il livello è basso 
         * 
         * @return -1 in caso di ERRORE di Semaforo non ottenuto
         */
        int8_t rawRead();

        /**
         * @brief restituisce se il pin è polling
         * 
         * @return true se è interrupt, false se è polling
         */
        bool IsInterrupt();

        /**
         * @brief restituisce se il pin è interrupt
         * 
         * @return true se è polling, false se è interrupt
         */
        bool IsPolling();

        /**
         * @brief Restituisce i livello in cui il pin viene triggerato
         * 
         * @returns LOW(0), HIGH(1)
         */
        int8_t getLevelTrig();

        /**
         * @brief Restituisce la modalità di trigger del pin
         * 
         * @returns RISING(1), FALLING(2) or CHANGE(3)
         */
        int8_t getTriggerMode();


        /**
         *  @brief Trasforma tutte le variabili della classe in una stringa
         *  
         *  @return Stringa con tutti le variabili
         */
        String toString()
        {
            String str = "";

            if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
                return "Errore xSemaphoreTake del pin \"%s\"\n";

            str += String("mutex : ") + String((uintptr_t)mutex, HEX) + "\n";
            str += String("name : ") + name + "\n";
            str += String("level : ") + level + "\n";
            str += String("pin : ") + pin + "\n";
            str += String("isInterrupt : ") + isInterrupt + "\n";
            str += String("debounce_ms : ") + debounce_ms + "\n";
            str += String("inputMode : ") + inputMode + "\n";
            str += String("levelTriggered : ") + levelTriggered + "\n";
            str += String("TRIGGER : ") + TRIGGER + "\n";
            str += String("changeOccurred : ") + changeOccurred + "\n";
            str += String("flag : ") + flag + "\n";
            str += String("precPinLevel : ") + precPinLevel + "\n";
            str += String("lastTime : ") + lastTime + "\n";
            str += String("debState : ") + debState + "\n";
            xSemaphoreGive(this->mutex);

            int8_t rawReadVal = rawRead();            
            String rawReadStr;
            str += String("raw read : ");
            switch(rawReadVal)
            {
              case -1   : str += "MUTEX ERROR (-1)"; break;
              case HIGH : str += "HIGH (1)"; break;
              case LOW  : str += "LOW (0)"; break;
              default   : str += "UNKNOWN : value " + String(rawReadVal); break;
            }
            str += "\n";

            return str;
        }

    private :
        SemaphoreHandle_t mutex = nullptr;

        /// Parametri da settare
        const char* name;           /*!< Optional: Nome amichevole del pin che serve per il debug */
        unsigned level : 1;         /*!< Stato reale del pin dopo il debounce */
        const uint8_t pin;          /*!< Pin in cui viene fatto il debounce */
        bool isInterrupt;           /*!< Flag che indica se è interrupt o polling */
        bool isAttached;            /*!< Flag che indica se il pin è attached o no */
        const uint32_t debounce_ms; /*!< Tempo di debounce in millisecondi (default = 30ms) */
        uint8_t inputMode;          /*!< Modalità di ingresso del pin ex. INPUT, INPUT_PULLUP, INPUT_PULLDOWN */
        uint8_t levelTriggered;     /*!< Livello per il quale viene considerato triggerato il pin */
        const uint8_t TRIGGER;      /*!< Trigger su cui viene rilevato un fronte di RISING, FALLING, CHANGE */
        bool changeOccurred;        /*!< Flag che segnala se è avvenuto l'evento */

        volatile unsigned flag : 1; /*!< Flag da usare nell'ISR con accesso atomico di natura */

        /// Per debounce function
        uint8_t precPinLevel;
        uint32_t lastTime;
        unsigned debState : 5;

        void __Init();

        static void __ISR(void* thisPtr);
};