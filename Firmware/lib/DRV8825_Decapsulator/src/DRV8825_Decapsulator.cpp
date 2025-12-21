//
//          FILE: DRV8825_Decapsulator.cpp
//    MOD AUTHOR: Manuel Randazzo
//  ORIGINAL AUTH: Rob Tillaart
//       VERSION: 0.2.0 ridotta al minimo indispensabile
//       PURPOSE: Arduino library for DRV8825 stepper motor driver
//          DATE: 2022-07-07
//           URL: https://github.com/RobTillaart/DRV8825

#include "DRV8825_Decapsulator.hpp"
#include "esp_timer.h"


#define __MUTEX_TIMEOUT__ 100



DRV8825::DRV8825()
{
}

DRV8825::~DRV8825()
{
  /// Elimina il timer
  if(this->DRV8825_timer != nullptr)
  {
    esp_timer_stop(this->DRV8825_timer);
    esp_timer_delete(this->DRV8825_timer);
  }

  /// Elimina il mutex
  if(_mutex != nullptr)
    vSemaphoreDelete(_mutex);
}

bool DRV8825::begin(uint8_t DIR, uint8_t STEP, uint8_t EN, uint8_t RST, uint8_t SLP, uint16_t number_of_steps_per_revolution)
{
  /// Crea il timer MoveHandlerTMR
  const esp_timer_create_args_t DRV8825_timer_args =
  {
    .callback = &__CallBackSteps,
    .arg = this,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "DRV8825_timer"
  };

  /// Crea il mutex che gestirà le variabili in modo atomico
  if(_mutex == nullptr)
  {
    _mutex = xSemaphoreCreateMutex();
    if(_mutex == nullptr)
    {
      Serial.println("Mutex Creation failed");
      return false;
    }
  }

  /// Critical Section
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  this->TaskHandler   = xTaskGetCurrentTaskHandle();
  this->_directionPin = DIR;
  this->_stepPin      = STEP;
  this->_enablePin    = EN;
  this->_resetPin     = RST;
  this->_sleepPin     = SLP;
  this->_stepsPerRevolution = number_of_steps_per_revolution;
  xSemaphoreGive(_mutex);

  esp_err_t esp_err = ESP_OK;

  /// Si salva il nuovo timer
  esp_timer_handle_t tmrDRV8825;
  esp_err = esp_timer_create(&DRV8825_timer_args, &tmrDRV8825);
  if(esp_err != ESP_OK || tmrDRV8825 == nullptr)
  {
    Serial.println("esp_timer_create failed");
    return false;
  }

  if(DIR == 255 || STEP == 255)
  {
    Serial.println("Errore: Pin DIR e/o STEP non hanno un pin assegnato, verificare le definizioni dei pin e il cablaggio");
    return false;
  }

  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  digitalWriteFast(DIR, LOW);
  digitalWriteFast(STEP, LOW);

  //  handle conditional parameters
  if(EN != 255)
    pinMode(EN, OUTPUT);
  if(SLP != 255)
    pinMode(SLP, OUTPUT);
  if(RST != 255)
  {
    pinMode(RST, OUTPUT);
    digitalWriteFast(RST, HIGH);
  }

    
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
    
  /// Ricopia l'handler del timer
  this->DRV8825_timer = tmrDRV8825;
  
  /// Crea impulso HIGH per 2.2µs + LOW per 2.2µs, questo è fisso così da non rompere...
  /// duration = _stepPulse.durationX * rmtTick = 1 * 2.2us
  _stepPulse[0].level0 = 1;
  _stepPulse[0].duration0 = 1;
  _stepPulse[0].level1 = 0;
  _stepPulse[0].duration1 = 1;
  xSemaphoreGive(_mutex);

  /// Setup RMT per il pin STEP
  rmt_tx_channel_config_t rmt_tx_cfg;
  rmt_tx_cfg.gpio_num = (gpio_num_t)_stepPin;
  rmt_tx_cfg.clk_src = RMT_CLK_SRC_DEFAULT;    // clock source (default)
  rmt_tx_cfg.resolution_hz = 80000000 / 176;   // periodo = 1/clk_freq = 176/80MHz = 2.2us
  rmt_tx_cfg.mem_block_symbols = 256;           
  rmt_tx_cfg.trans_queue_depth = 4;            // profondità della coda di trasferimento
  rmt_tx_cfg.intr_priority = 0;                // 0 -> priorità bassa
  rmt_tx_cfg.flags.invert_out = 0;
  rmt_tx_cfg.flags.with_dma = 1;
  rmt_tx_cfg.flags.io_loop_back = 0;
  rmt_tx_cfg.flags.io_od_mode = 0;
  rmt_tx_cfg.flags.allow_pd = 0;
  rmt_tx_cfg.flags.init_level = 1;

  esp_err = rmt_new_tx_channel(&rmt_tx_cfg, &this->_rmtChannel);
  if(esp_err != ESP_OK)
    return false;
  /// Se il canale RMT è stato creato corretamente lo abilita
  rmt_enable(this->_rmtChannel);

  rmt_copy_encoder_config_t enc_cfg = {};
  ESP_ERROR_CHECK(rmt_new_copy_encoder(&enc_cfg, &this->step_encoder));


  return true;
}

void DRV8825::update()
{
  //uint32_t startTime = micros();

  /// Aspetta la notifica e se non arriva esce dalla funzione update
  if(ulTaskNotifyTake(pdTRUE, 0) <= 0)
    return;

  bool exitCondition = false;

  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  /// @return se ha finito gli steps che doveva fare
  if(!_isContinuous && _stepsLeft == 0)
  {
    _isStepDone = true;
    exitCondition = true;
  }
  xSemaphoreGive(_mutex);
  
  if(exitCondition)
  {
    Serial.println("exitCondition = true");
    /// Ferma il timer
    if(DRV8825_timer)
      esp_timer_stop(DRV8825_timer);
    return;
  }

  
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  /// Impulso di 2.2us HIGH e 2.2us LOW
  esp_err_t err = rmt_transmit(this->_rmtChannel, this->step_encoder, this->_stepPulse, sizeof(this->_stepPulse), &this->transmit_cfg);
  if(err == ESP_OK)
  {
    /// Timeout in ms che aspetta la fine della trasmissione (polling bloccante)
    const int timeoutWaitAllDone = 10;
    err = rmt_tx_wait_all_done(this->_rmtChannel, timeoutWaitAllDone); 
    if(err == ESP_OK)
    {
      if(!_isContinuous && _stepsLeft > 0)
      {
        _stepsLeft--;
        if (_stepsLeft == 0)
        {
          _isStepDone = true;
          exitCondition = true;
        }
      }
      _absStepCounter += _direction;

      if(exitCondition && DRV8825_timer)
        esp_timer_stop(DRV8825_timer);
    }
  }
  xSemaphoreGive(_mutex);
  
  //uint32_t stopTime = micros();
  //Serial.printf("Tempo di esecuzione __CallbackSteps: %d\n\nErrore RMT: %sus\n", stopTime - startTime, err==ESP_OK ? "ESP_OK" : "ESP_ERROR");
}


bool DRV8825::setDirection(int8_t direction)
{
  // Se la direzione non è valida esce
  if(direction != DRV8825_CLOCK_WISE && direction != DRV8825_COUNTERCLOCK_WISE)
    return false;

  uint8_t dirPin;
  
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  this->_direction = direction;
  dirPin = this->_directionPin;
  xSemaphoreGive(_mutex);

  //  timing from datasheet 650 ns figure 1
  delayMicroseconds(1);
  digitalWriteFast(dirPin, direction == DRV8825_CLOCK_WISE ? LOW : HIGH);
  delayMicroseconds(1);

  return true;
}


int8_t DRV8825::getDirection()
{
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  uint8_t dirPin = this->_directionPin;
  xSemaphoreGive(_mutex);
  
  return digitalReadFast(dirPin) == LOW ? DRV8825_CLOCK_WISE : DRV8825_COUNTERCLOCK_WISE;
}

/// @warning evitare di eseguire questa funzione quando il motore gira
void DRV8825::setAbsPosition(int64_t absolute_position)
{  
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  _absStepCounter = absolute_position;
  xSemaphoreGive(_mutex);
}

int64_t DRV8825::getAbsPosition()
{
  int64_t absolute_position;

  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  absolute_position = _absStepCounter;
  xSemaphoreGive(_mutex);

  return absolute_position;
}

bool DRV8825::isStepDone()
{
  bool tmpIsStepDone;

  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  tmpIsStepDone = this->_isStepDone;
  xSemaphoreGive(_mutex);
  
  return tmpIsStepDone;
}

void DRV8825::step(uint64_t numberOfStepsToDo, uint64_t period_us)
{
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  this->_isContinuous = false;

  this->_isStepDone = false;

  this->_stepsLeft = numberOfStepsToDo;
  xSemaphoreGive(_mutex);

  setTmr(period_us);
}

void stepGradi(double gradi, uint64_t period_us)
{
  /*
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  
  xSemaphoreGive(_mutex);
  uint64_t stepsToDo = 2233; /// Numero a caso lol
  */
}

void DRV8825::stepContinuous(uint64_t period_us)
{
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  this->_isContinuous = true;

  this->_isStepDone = false;

  this->_stepsLeft = 0;
  xSemaphoreGive(_mutex);

  setTmr(period_us);
}


//  Table page 3
bool DRV8825::enable()
{
  uint8_t enPin;

  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  enPin = this->_enablePin;
  xSemaphoreGive(_mutex);

  if(enPin == 255)
    return false;

  digitalWriteFast(enPin, LOW);
  return true;
}

bool DRV8825::disable()
{
  uint8_t enPin;

  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  enPin = this->_enablePin;
  xSemaphoreGive(_mutex);

  if(enPin == 255)
    return false;

  digitalWriteFast(enPin, HIGH);
  return true;
}

bool DRV8825::isEnabled()
{
  uint8_t enPin;

  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  enPin = this->_enablePin;
  xSemaphoreGive(_mutex);

  if(enPin == 255)
    return false;

  return (digitalReadFast(enPin) == LOW);
}


bool DRV8825::reset()
{
  uint8_t rstPin;

  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  rstPin = this->_resetPin;
  xSemaphoreGive(_mutex);

  if(rstPin == 255)
    return false;

  digitalWriteFast(rstPin, LOW);
  delay(1);
  digitalWriteFast(rstPin, HIGH);

  return true;
}


bool DRV8825::sleep()
{
  uint8_t slpPin;
  
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  slpPin = this->_sleepPin;
  xSemaphoreGive(_mutex);
  
  if(slpPin == 255)
    return false;

  digitalWriteFast(slpPin, LOW);
  return true;
}

bool DRV8825::wakeup()
{
  uint8_t slpPin;
  
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  slpPin = this->_sleepPin;
  xSemaphoreGive(_mutex);

  if(slpPin == 255)
    return false;

  digitalWriteFast(slpPin, HIGH);
  return true;
}

bool DRV8825::isSleeping()
{
  uint8_t slpPin;
  
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  slpPin = this->_sleepPin;
  xSemaphoreGive(_mutex);

  if(slpPin == 255)
    return false;

  return (digitalReadFast(slpPin) == LOW);
}


void DRV8825::setTmr(uint64_t period_us)
{
  /// Per proprietà hardware del DRV8825 3.8us è il periodo minimo di lavoro dello step pin
  if(period_us <= DRV8825_MIN_PERIOD_US)
    period_us = DRV8825_MIN_PERIOD_US;
  
  uint64_t __prev_period_us;

  esp_timer_handle_t tmrDRV8825;

  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  if(this->DRV8825_timer)
    tmrDRV8825 = this->DRV8825_timer;
  else
  {
    Serial.println("Errore, timer non definito");
    xSemaphoreGive(_mutex);
    return;
  }
  xSemaphoreGive(_mutex);

  esp_timer_get_period(tmrDRV8825, &__prev_period_us);

  /// Se il periodo è diverso e il timer è già attivo...
  if(period_us != __prev_period_us)
  {
    /// Fermiamo il timer (richiesto esplicitamente dalla documentazione)
    esp_timer_stop(tmrDRV8825);
      
    /// Avvia il timer con il ritardo specificato in microsecondi
    esp_timer_start_periodic(tmrDRV8825, period_us);
  }
}


/**
 *  @private Element Of The Class
 *
 *  @brief Questo metodo aggiorna la posizione del motore tramite 
 *         una task ad alta priorità se .dispatch_method del timer = ESP_TIMER_TASK
 */
void DRV8825::__CallBackSteps(void* args)
{
  DRV8825 *INST = static_cast<DRV8825*>(args);

  /// Se diventa pdTRUE passa subito alla task se non ci sono task più prioritarie di questa
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  /// Crea la notifica dell'interrupt
  vTaskNotifyGiveFromISR(INST->TaskHandler, &xHigherPriorityTaskWoken);

  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


//  -- END OF FILE --