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
#define DRV8825_CLOCK_WISE         +1  //  pin LOW, incremento
#define DRV8825_COUNTERCLOCK_WISE  -1  //  pin HIGH, decremento

/// ABSOLUTE PERIOD VALUES
#define DRV8825_MIN_PERIOD_US 4 // Tempo minimo tra uno step e l'altro in microsecondi (us)



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

    bool     begin(uint8_t DIR, uint8_t STEP, uint8_t EN = 255, uint8_t RST = 255, uint8_t SLP = 255, uint16_t number_of_steps_per_revolution = 200);
    void     update();

    //       DIRECTION
    //       +1 = DRV8825_CLOCK_WISE
    //       -1 = DRV8825_COUNTERCLOCK_WISE
    //       returns false if parameter out of range.
    bool     setDirection(int8_t direction = DRV8825_CLOCK_WISE);
    int8_t   getDirection();

    //       ABSOLUTE POSITION
    void     setAbsPosition(int64_t absolute_position);
    int64_t  getAbsPosition();

    //       STEPS
    void     step(uint64_t numberOfStepsToDo, uint64_t period_us);
    void     stepGradi(double gradi, double gradi_al_secondo);
    void     stepContinuous(uint64_t period_us);
    bool     isStepDone();

    //       ENABLE pin should be set.
    bool     enable();
    bool     disable();
    bool     isEnabled();

    //       RESET pin should be set.
    bool     reset();

    //       SLEEP pin should be set.
    bool     sleep();
    bool     wakeup();
    bool     isSleeping();

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
    bool     _isStepDone     = false;
    int64_t  _absStepCounter = 0;
    uint16_t _stepsPerRevolution;
    rmt_channel_handle_t _rmtChannel = NULL;


    rmt_symbol_word_t _stepPulse[1];

    void setTmr(uint64_t period_us);

    /// Definisce il timer di precisione usato per fare un delay senza
    /// CPU Load con frequenza maggiore rispetto a freertos
    esp_timer_handle_t DRV8825_timer = nullptr;

    static void __CallBackSteps(void* args);


  private:
    /// Questo mutex garantisce che una sola task alla volta acceda alla risorsa condivisa o alle variabili
    SemaphoreHandle_t _mutex = nullptr;  
    
    const rmt_transmit_config_t transmit_cfg =
    {
      .loop_count = 0,
      .flags = { .eot_level = 0 }  // livello LOW dopo la trasmissione
    };
    
    rmt_encoder_handle_t step_encoder = NULL;
};




//  -- END OF FILE --