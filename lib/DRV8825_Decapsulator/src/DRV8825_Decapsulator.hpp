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
#include "driver/rmt.h"
#include "esp_log.h"


//  setDirection
#define DRV8825_CLOCK_WISE         +1  //  pin LOW, incremento
#define DRV8825_COUNTERCLOCK_WISE  -1  //  pin HIGH, decremento

/// ABSOLUTE PERIOD VALUES
#define DRV8825_MIN_PERIOD_US 4 // Tempo minimo tra uno step e l'altro in microsecondi (us)




class DRV8825
{
  public:
    DRV8825();
    ~DRV8825();

    bool     begin(uint8_t DIR, uint8_t STEP, uint8_t EN = 255, uint8_t RST = 255, uint8_t SLP = 255, uint16_t number_of_steps_per_revolution = 200);

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
    rmt_channel_t _rmtChannel = RMT_CHANNEL_MAX;


    rmt_item32_t _stepPulse[1];

    void setTmr(uint64_t period_us);

    /// Definisce il timer di precisione usato per fare un delay senza
    /// CPU Load con frequenza maggiore rispetto a freertos
    esp_timer_handle_t DRV8825_timer = nullptr;

    static void __CallBackSteps(void* args);


  private:
    /// Questo mutex garantisce che una sola task alla volta acceda alla risorsa condivisa o alle variabili
    SemaphoreHandle_t _mutex = nullptr;

    /// Questo spinlock garantisce che una sola task alla volta acceda alla risorsa condivisa o alle variabili nella ISR
    portMUX_TYPE _spinlock = portMUX_INITIALIZER_UNLOCKED;
    
};



/*
 *  In fase di precompilazione per lo sviluppo della mod della libreria
 */

#ifndef digitaWriteFast
#define digitalWriteFast(pin, high_or_low) do{ gpio_set_level((gpio_num_t)pin, high_or_low); }while(0)
#endif

#ifndef digitalReadFast
#define digitalReadFast(pin)               ( gpio_get_level((gpio_num_t)pin) )
#endif

//  -- END OF FILE --