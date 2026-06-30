#include <cmath>
#pragma once
//
//          FILE: DRV8825_Decapsulator.hpp
//    MOD AUTHOR: Manuel Randazzo
//  ORIGINAL AUTH: Rob Tillaart
//       VERSION: 0.2.0 ridotta al minimo indispensabile
//       PURPOSE: Arduino library for DRV8825 stepper motor driver
//          DATE: 2022-07-07
//           URL: https://github.com/RobTillaart/DRV8825


/**
 * 
 *  @info: CALCOLARE IL VALORE GIUSTO DELLA Vref (tensione sul potenziometro) DEL DRV8825
 * 
 *  @formula: Vref = Ipeak / 2
 * 
 *  @formula: Vref = Ipeak / (2 * sqrt(2))
 * 
 *  @formula: Corrente picco ==> Ipeak = Vref * 2
 * 
 *  @example Corrente per bobina = 1,75 [A] --> Vref = (1,75 - 30%) / 2 = 1.225 / 2 = 0.6125 [V]
 * 
 */

#include "Arduino.h"
#include "driver/rmt_tx.h"
#include "hal/gpio_ll.h"

/// è stato scelto di usare atomic per avere un accesso veramente atomico 
/// quando viene condiviso tra ISR e tasks e ridurre jitter in update
#include <atomic>


//  setDirection
typedef int8_t drv_direction_t;

#define DRV8825_CLOCK_WISE         (1)   //  pin LOW, incremento
#define DRV8825_COUNTERCLOCK_WISE  (-1)  //  pin HIGH, decremento

/// ABSOLUTE PERIOD VALUES
#define DRV8825_MIN_PERIOD_US 285 // Tempo minimo tra uno step e l'altro in microsecondi (us)
#define DRV8825_RMT_PSC (40)  // Prescaler
#define DRV8825_RMT_PULSE_US ((double)(DRV8825_RMT_PSC) / 80.0) // Prescaler : 40 / 80MHz = 0.5us
constexpr uint32_t DRV8825_RMT_MAX_LOOP_COUNT = 32767; // Step massimi possibili in un singola trasmissione

#define DRV8825_RMT_PULSE_US_COMPARE (DRV8825_RMT_PSC / 8)
#if DRV8825_RMT_PULSE_US_COMPARE >= 20
    /// Pulse maggiore o uguale a 2us quindi basta solo 1 tick
    constexpr uint16_t DRV8825_RMT_DURATION_0 = (uint16_t)(1);
#else
    /// Pulse minore di 2us quindi calcola i ticks necessari per avere almeno un impulso alto >=2us (con ceil->arrotonda al numero maggiore)
    constexpr uint16_t DRV8825_RMT_DURATION_0 = (uint16_t)(ceil(2.0 / DRV8825_RMT_PULSE_US));
#endif

/// @brief errori restituiti dal drv
typedef int8_t drv_err_t;

#define DRV_TRUE                        1       /*!< Codice (drv_err_t) che indica un booleano true ) */
#define DRV_FALSE                       0       /*!< Codice (drv_err_t) che indica un booleano false  */
#define DRV_OK                          3       /*!< Codice (drv_err_t) che indica successo (no errori) */
#define DRV_FAIL                        -1      /*!< Codice (drv_err_t) generico che indica insuccesso  */
#define DRV_NO_NOTIFY                   4       /*!< Codice (drv_err_t) che indica che non è arrivata nessuna notifica (non un errore) */
#define DRV_ERR_MUX_CREATION            5       /*!< Codice (drv_err_t) che indica la creazione fallita del mutex */
#define DRV_ERR_MUX_TAKE_TIMEOUT        6       /*!< Codice (drv_err_t) che indica un timeout da parte del mutex */
#define DRV_ERR_TMR_CREATION            7       /*!< Codice (drv_err_t) che indica la creazione fallita del timer */
#define DRV_ERR_TMR_UNDEFINED           8       /*!< Codice (drv_err_t) che indica la creazione fallita del timer */
#define DRV_ERR_NO_DIR_PIN              9       /*!< Codice (drv_err_t) che indica che non esiste un pin DIR */
#define DRV_ERR_NO_STEP_PIN             10      /*!< Codice (drv_err_t) che indica che non esiste un pin STEP */
#define DRV_ERR_NO_EN_PIN               11      /*!< Codice (drv_err_t) che indica che non esiste un pin EN */
#define DRV_ERR_NO_SLP_PIN              12      /*!< Codice (drv_err_t) che indica che non esiste un pin SLP */
#define DRV_ERR_NO_RST_PIN              13      /*!< Codice (drv_err_t) che indica che non esiste un pin RST */
#define DRV_RMT_TX_BUSY                 14      /*!< Codice (drv_err_t) che indica che sta ancora avvenendo la trasmissione del canale rmt (niente polling bloccante) */
#define DRV_ERR_RMT_CREATION            15      /*!< Codice (drv_err_t) che indica che un errore nella creazione del canale rmt di trasmissione */
#define DRV_ERR_RMT_ENABLE              16      /*!< Codice (drv_err_t) che indica che un errore nell'abilitazione del canale rmt di trasmissione */
#define DRV_ERR_RMT_COPY_ENCODER        17      /*!< Codice (drv_err_t) che indica che un errore nella copia in memoria nel canale rmt di trasmissione */
#define DRV_ERR_RMT_TX_TIMEOUT          18      /*!< Codice (drv_err_t) che indica un timeout nella trasmissione del canale rmt */
#define DRV_ERR_RMT_TRANSMIT_CMD        19      /*!< Codice (drv_err_t) che indica che un errore nella trasmissione del canale rmt */

/**
  * @brief Ritorna una stringa di codici errori di tipo drv_err_t 
  * 
  * @param code codice errore drv_err_t
  * 
  * @return stringa del messaggio d'errore
  */
extern const char *drv_err_to_name(drv_err_t code);


/*
 *  Lettura/Scrittura dei pin a basso livello senza controllo di validità del pin
 */
#ifndef digitaWriteFast
    /**
     * @brief Definisce una scrittura del pin low level (gpio_ll_set_level ha l'attributo always inline) 
     * 
     * @param gpio_pin è il pin che verrà scritto
     */
    #define digitalWriteFast(gpio_pin, level) gpio_ll_set_level(&GPIO, gpio_pin, level)
#endif

#ifndef digitalReadFast
    /**
     * @brief Definisce una lettura del pin low level (gpio_ll_get_level ha l'attributo always inline) 
     * 
     * @param gpio_pin è il pin che verrà letto
     * 
     * @return gpio digital level
     */
    #define digitalReadFast(gpio_pin)         gpio_ll_get_level(&GPIO, gpio_pin)
#endif







class DRV8825
{
    public:
        DRV8825();
        ~DRV8825();

        drv_err_t     begin(uint8_t DIR, uint8_t STEP, uint8_t EN = 255, uint8_t RST = 255, uint8_t SLP = 255, uint16_t number_of_steps_per_revolution = 200);
        drv_err_t     update();

        // Di norma non è neccesario in quanto usa la task in cui viene
        // fatto il begin() ricavandosi l'Handler in autonomia
        drv_err_t     setUpdateTask(TaskHandle_t handler);

        //       DIRECTION
        //       +1 = DRV8825_CLOCK_WISE
        //       -1 = DRV8825_COUNTERCLOCK_WISE
        //       returns false if parameter out of range.
        drv_err_t     setDirection(drv_direction_t direction = DRV8825_CLOCK_WISE);
        drv_err_t     getDirection();

        //       ABSOLUTE POSITION
        drv_err_t     setAbsPosition(int64_t absolute_position);
        int64_t       getAbsPosition();

        //       STEPS
        drv_err_t     step(uint64_t numberOfStepsToDo, uint64_t period_us, uint64_t acceleration_step_s2 = 0, uint64_t deceleration_step_s2 = 0);
        uint64_t      abortCurrentMovement();
        drv_err_t     stepContinuous(uint64_t period_us);
        drv_err_t     isStepDone();

        //       ENABLE pin should be set.
        drv_err_t     enable();
        drv_err_t     disable();
        drv_err_t     isEnabled();

        //       RESET pin should be set.
        drv_err_t     reset();

        //       SLEEP pin should be set.
        drv_err_t     sleep();
        drv_err_t     wakeup();
        bool          isSleeping();

    protected:
        TaskHandle_t TaskHandler = nullptr;

        uint8_t  _directionPin   = 255;
        uint8_t  _stepPin        = 255;
        uint8_t  _enablePin      = 255;
        uint8_t  _resetPin       = 255;
        uint8_t  _sleepPin       = 255;

        int8_t  _direction       = DRV8825_CLOCK_WISE;

        uint64_t  _period_us     = 0;
        std::atomic<uint64_t> _stepsLeft{0};
        std::atomic<bool> _isStepDone{false};
        int64_t  _absStepCounter = 0;
        uint16_t _stepsPerRevolution;
        rmt_channel_handle_t _rmtChannel = nullptr;

        std::atomic<bool> _rmtBusy{false};

        uint32_t _tmrStartOfRmtTransmit  = 0;
        uint64_t _acc_end_steps = 0;
        uint64_t _const_end_steps   = 0;


        typedef enum __stati_moto__ : uint8_t
        {
            ACCELERATION,
            CONSTANT,
            DECELERATION,
        } StatoMoto_t;
        StatoMoto_t StatoMoto;
      

      
        /// Crea impulso HIGH per 2.2µs + LOW per i µs necessari, questi sono costanti, cambia solo la duration del level LOW
        //         2*1us    period voluto​ 
        //        ╠═════╬═════════════════╣
        // HIGH-> ╔═════╗
        //        ​║     ║
        //        ║     ║
        //  LOW-> ╝     ╚═════════════════
        rmt_symbol_word_t _stepPulse[1] =
        { 
            [0] = {
                      .duration0 = DRV8825_RMT_DURATION_0, // HIGH
                      .level0 = 1,     // HIGH
                      .duration1 = 0,  // LOW
                      .level1 = 0,     // LOW
                  }
        };
        
        /// si conosce sin da subito la size di _stepPulse
        static constexpr size_t STEP_PULSE_SIZE = sizeof(_stepPulse);

        inline void setAndEnableRMT(const uint64_t ACC_STEPS_S2, const uint64_t DEC_STEPS_S2, uint64_t period_us);

        // V[steps/s] ^            
        //            ​​║         
        //       Vmax ​║ ¯ ¯ ¯/¯¯¯¯¯¯¯¯¯¯¯¯¯\     
        //            ​║     /               \           
        //     Vmedia ​║- - / - - - - - - - - \- - ┐  <-- Detta anche Vrichiesta         
        //            ​║   /                   \   |      
        //            ​║  /                     \  |      
        //            ​║ /                       \ |      
        //            ​║/                         \_____________    
        //            ╚════════════════════════════════════════════> t [s]          
        //            ╠══════╬═════════════╬══════╬═══════════╣
        //              Tacc      Tcost      Tdec     Tstop
        void calcRampSteps(const uint64_t ACC_STEPS_S2, const uint64_t DEC_STEPS_S2);

        /// Periodo in us per ogni step in accelerazione e decelerazione pre calcolato
        uint16_t* _duration_acc_dec = nullptr;
        uint32_t duration_acc_dec_index = 0;
        
        void setAccDecDurations(uint32_t v_max_us, uint32_t n_acc_steps, double t_acc, uint32_t n_dec_steps, double t_dec);

    private:
        /// Questo mutex garantisce che una sola task alla volta acceda alla risorsa condivisa o alle variabili
        SemaphoreHandle_t _mutex = nullptr;

        bool _isDriverInitialized = false;

        rmt_transmit_config_t transmit_cfg =
        {
          .loop_count = 0,
          .flags = { .eot_level = 0 }  // livello LOW dopo la trasmissione
        };
        
        rmt_encoder_handle_t step_encoder = nullptr;

        /// Tocca far così perchè se no la callback dell'rmt non vede i membri della classe
        friend bool IRAM_ATTR drv8825_rmt_tx_done_cb(rmt_channel_handle_t channel, const rmt_tx_done_event_data_t *edata, void *user_data);
};
//  -- END OF FILE --