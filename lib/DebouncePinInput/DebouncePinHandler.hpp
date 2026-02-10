#pragma once

#include "Arduino.h"
#include "Debug.hpp"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"
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


#define INTR (bool)(false)
#define POLL (bool)(true)



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
         *  @brief Trasforma tutte le variabili della classe in una stringa
         *  
         *  @return Stringa con tutti le variabili
         */
        String toString()
        {
            String str = "";

            str += String("mutex : ") + String((uintptr_t)mutex, HEX) + "\n";
            str += String("name : ") + name + "\n";
            str += String("level : ") + level + "\n";
            str += String("pin : ") + pin + "\n";
            str += String("isInterrupt : ") + isInterrupt + "\n";
            str += String("debounce_ms : ") + debounce_ms + "\n";
            str += String("inputMode : ") + inputMode + "\n";
            str += String("TRIGGER : ") + TRIGGER + "\n";
            str += String("changeOccurred : ") + changeOccurred + "\n";
            str += String("flag : ") + flag + "\n";
            str += String("precPinLevel : ") + precPinLevel + "\n";
            str += String("lastTime : ") + lastTime + "\n";
            str += String("debState : ") + debState + "\n";

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
        const uint8_t TRIGGER;      /*!< Trigger su cui viene rilevato un fronte di RISING, FALLING, CHANGE */
        bool changeOccurred;        /*!< Flag che segnala se è avvenuto l'evento */

        volatile unsigned flag : 1; /*!< Flag da usare nell'ISR con accesso atomico di natura */

        /// Per debounce function
        uint8_t precPinLevel;
        uint32_t lastTime;
        unsigned debState : 5;

        void __Init(bool inputMode);

        static void __ISR(void* thisPtr);
};