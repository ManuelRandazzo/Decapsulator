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
  #define digitalReadFast(gpio_pin)  gpio_ll_get_level(&GPIO, gpio_pin)
#endif



class DebPinHandler
{
    public :
        DebPinHandler(uint8_t pinNumber, const char* pinName = "No Pin Name", uint8_t input_mode = INPUT, 
                      void (*ISR)() = nullptr, uint32_t debounce_ms = 30);

        DebPinHandler(uint8_t pinNumber, const char* pinName = "No Pin Name", uint8_t input_mode = INPUT,
                      uint8_t level_trigger = CHANGE, void (*ISR)() = nullptr, uint32_t debounce_ms = 30);

        DebPinHandler(uint8_t pinNumber, const char* pinName = "No Pin Name", uint8_t input_mode = INPUT,
                      uint8_t level_trigger = CHANGE, uint32_t debounce_ms = 30);

        ~DebPinHandler();


        /**
         * @brief  Aggiorna l'istanza e fa il debounce in INTERRUPT
         * @return Se è avvenuto o no un cambio di stato del pin
         * @attention E' solo per INTERRUPT:
         */
        bool update();

        /**
         * @brief  Aggiorna l'istanza e fa il debounce in POLLING
         * @param  trigger RISING, FALLING, CHANGE 
         * @return Se è avvenuto o no un cambio di stato del pin
         * @attention E' solo per POLLING:
         */
        bool update(uint8_t trigger);

    private :
        SemaphoreHandle_t mutex = nullptr;

        /// Parametri da settare
        const char* name;              /*!< Optional: Nome amichevole del pin che serve per il debug */
        unsigned level : 1;            /*!< Stato reale del pin dopo il debounce */
        const uint8_t pin;             /*!< Pin in cui viene fatto il debounce */
        const uint32_t debounce_ms;    /*!< Tempo di debounce in millisecondi (default = 30ms) */
        void (*ISR)();                 /*!< Interrupt Service Routine che gestisce il debounce */
        uint8_t inputMode;             /*!< Modalità di ingresso del pin ex. INPUT, INPUT_PULLUP, INPUT_PULLDOWN */
        const uint8_t TRIGGER;         /*!< Trigger su cui viene rilevato un fronte di RISING, FALLING, CHANGE */

        volatile unsigned flag : 1;    /*!< Flag da usare nell'ISR con accesso atomico di natura */

        /// Per debounce function
        unsigned precPinLevel : 1;
        uint32_t lastTime;
        unsigned debState : 5;

        void __Init();

        bool __debouncePolling();

        bool __debounceInterrupt();

        static void __InternalISR(void* pvParameters);
};