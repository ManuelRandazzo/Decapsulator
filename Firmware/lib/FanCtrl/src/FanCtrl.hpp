/**
 * Classe per comandare una ventola tramite l'hardware 
 * LEDC (LED Control) Integrato nell'ESP32.
 *
 * @link Documentazione LEDC : https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html
 */

#include "Arduino.h"

/// Default 35kHz --> fuori dalle frequenze udibili per non sentire la ventola fischiare
#define DEFAULT_FREQ 35000

/// Default 7 bits --> per funzionare bene con la frequenza default
#define DEFAULT_RES      7

/**
 * Calcolo risoluzione MASSIMA in base alla frequenza :
 * 
 * @formula: log₂(f_clk_periferica / f_pwm) - 1
 * 
 * @example f_pwm = 35000 ---> log₂(80000000 / f_pwm) - 1 =
 *                             log₂(80000000 / 35000) - 1 =
 *                             11.16 - 1 = 
 *                             10.16 -> MASSIMO 10 bit
 *                                      ==============
 */

class FanCtrl
{
    public :
        FanCtrl();




        ~FanCtrl();




        /**
         * @brief Inizializza il pin e l'hardware LEDC (LED Control)
         *
         * @param pin pin del micro a cui è connessa la ventola
         *
         * @param pwm_freq seleziona la frequenza della PWM.
         *
         * @param pwm_resolution_in_bits Range 1-20 bits. Seleziona la risoluzione in bit del canale LEDC.
         *
         * @return Se l'inizializzazione è avvenuta correttamente
         *
         * @attention Viene spenta la PWM alla fine del begin()
         */
        bool begin(uint8_t pin, uint32_t pwm_freq = DEFAULT_FREQ, uint8_t pwm_resolution_in_bits = DEFAULT_RES);




        /**
         * @brief Scollega il pin dalla PWM
         *
         * @return Se è stato scollegato il pin correttamente
         */
        bool detach();




        /**
         * @brief Cambia il duty-cycle della PWM
         *
         * @param duty_percentage Range 0-100. Seleziona il duty-cycle della PWM, espresso in percentuale [%].
         *
         * @attention se duty_percentage > 100 allora il duty viene impostato al 100%
         *
         * @return Se è stato cambiato correttamente il duty-cycle
         *
         * @example duty_percentage = 0 ---> ventola spenta
         *
         * @example duty_percentage = 100 ---> ventola accesa
         */
        bool setDuty(uint8_t duty_percentage);



        
        /**
         * @brief Fa una rampa a livello hardware (zero CPU load, zero overhead, zero blocking) in un certo tempo 
         *        partendo dal duty cycle precedentemente impostato e arrivando al "target_duty"
         *
         * @param target_duty Range 0-100. Duty-cycle finale espresso in [%]
         *
         * @param max_fade_time_ms Tempo della rampa espresso in [ms]. MAX = 32.767 secondi
         *
         * @return Se la rampa è stata settata corretamente
         * 
         * @example Voglio che la rampa mi faccia andare la ventola da
         *          0% a 100% in 5 secondi allora scrivo
         *          Ventola.ramp(100, 5000);
         * 
         * @example Voglio che la rampa mi faccia andare la ventola da
         *          25% a 80% in 2.5 secondi allora scrivo
         *          Ventola.ramp(80, 2500);
         * 
         * @deprecated In realtà ledcFade() lo fa nel tempo minimo possibile 
         *             quindi non rispetta la rampa, avrebbe bisogno di un loop ma
         *             diventa inutile se non lo può fare l'hardware
         */
        //bool ramp(uint32_t target_duty, int max_fade_time_ms);




        /**
         * @brief Setta la ventola al 100% - Ventola accesa
         */
        bool on();




        /**
         * @brief Setta la ventola al 0% - Ventola spenta
         */
        bool off();




        /**
         * @brief Cambia la frequenza e la risoluzione della PWM
         *
         * @param pwm_freq seleziona la frequenza della PWM.
         *
         * @param pwm_resolution_in_bits Range 1-20 bits. Seleziona la risoluzione in bit del canale LEDC.
         *
         * @return Se è stata cambiata correttamente la frequenza e la risoluzione
         */
        bool changeFrequency(uint32_t pwm_freq = DEFAULT_FREQ, uint8_t pwm_resolution_in_bits = DEFAULT_RES);


    
  
    private :
        /// pin della ventola
        uint8_t fan_pin = 255;


        /// Duty attuale espresso in [%]
        uint8_t duty = 0;

};











