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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/rmt_tx.h"
#include "soc/gpio_struct.h"
#include "hal/gpio_ll.h"
#include "esp_log.h"


//  setDirection
typedef int8_t drv_direction_t;

#define DRV8825_CLOCK_WISE         1   //  pin LOW, incremento
#define DRV8825_COUNTERCLOCK_WISE  -1  //  pin HIGH, decremento

/// ABSOLUTE PERIOD VALUES
#define DRV8825_MIN_PERIOD_US 900 // Tempo minimo tra uno step e l'altro in microsecondi (us)


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
#define DRV_WAITING_RMT_TX_TO_FINISH    14      /*!< Codice (drv_err_t) che indica che sta ancora avvenendo la trasmissione del canale rmt (niente polling bloccante) */
#define DRV_ERR_RMT_CREATION            15      /*!< Codice (drv_err_t) che indica che un errore nella creazione del canale rmt di trasmissione */
#define DRV_ERR_RMT_ENABLE              16      /*!< Codice (drv_err_t) che indica che un errore nell'abilitazione del canale rmt di trasmissione */
#define DRV_ERR_RMT_COPY_ENCODER        17      /*!< Codice (drv_err_t) che indica che un errore nella copia in memoria nel canale rmt di trasmissione */
#define DRV_ERR_RMT_TX_TIMEOUT          18      /*!< Codice (drv_err_t) che indica un timeout nella trasmissione del canale rmt */
#define DRV_ERR_RMT_TRANSMIT_CMD        19      /*!< Codice (drv_err_t) che indica che un errore nella trasmissione del canale rmt */
#define DRV_CMD_ABORTED                 20      /*!< Codice (drv_err_t) che indica un comando abortito dal metodo abortCurrent Movement */

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
    drv_err_t     step(uint64_t numberOfStepsToDo, uint64_t period_us);
    void          abortCurrentMovement();
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

    uint64_t  _stepsLeft     = 0;
    bool     _isContinuous   = false;
    bool     _abortCommand   = false;
    bool     _isStepDone     = true;
    int64_t  _absStepCounter = 0;
    uint16_t _stepsPerRevolution;
    rmt_channel_handle_t _rmtChannel = NULL;

    bool     _waitRmtAsyncTransmit   = false;

    uint32_t _timeoutRmtTransmit     = 0;

    rmt_symbol_word_t _stepPulse[1];

    drv_err_t setTmr(uint64_t period_us);

    /// Definisce il timer di precisione usato per fare un delay senza
    /// CPU Load con frequenza maggiore rispetto a freertos
    esp_timer_handle_t DRV8825_timer = nullptr;

    static void __CallBackSteps(void* args);


  private:
    /// Questo mutex garantisce che una sola task alla volta acceda alla risorsa condivisa o alle variabili
    SemaphoreHandle_t _mutex = nullptr;

    bool _isDriverInitialized = false;
    
    const rmt_transmit_config_t transmit_cfg =
    {
      .loop_count = 0,
      .flags = { .eot_level = 0 }  // livello LOW dopo la trasmissione
    };
    
    rmt_encoder_handle_t step_encoder = NULL;
};
//  -- END OF FILE --