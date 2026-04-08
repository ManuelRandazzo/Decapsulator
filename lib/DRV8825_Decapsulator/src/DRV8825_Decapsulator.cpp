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

#define __MUTEX_TIMEOUT_TICKS__ pdMS_TO_TICKS(20)
#define __TIMEOUT_WAIT_ALL_DONE_RMT_TX_MS__ 10    /// NON E' BLOCCANTE, MA TENERLO BREVE COMUNQUE


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

  /// Elimina il canale RMT
  if(this->_rmtChannel != nullptr)
  {
    rmt_disable(this->_rmtChannel);
    rmt_del_channel(this->_rmtChannel);
  }

  /// Elimina il mutex
  if(this->_mutex != nullptr)
    vSemaphoreDelete(_mutex);
}

drv_err_t DRV8825::begin(uint8_t DIR, uint8_t STEP, uint8_t EN, uint8_t RST, uint8_t SLP, uint16_t number_of_steps_per_revolution)
{
  /// Crea il mutex che gestirà le variabili in modo atomico
  if(this->_mutex == NULL)
  {
    this->_mutex = xSemaphoreCreateMutex();
    if(this->_mutex == NULL)
      return DRV_ERR_MUX_CREATION;
  }

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;
  
  if(this->TaskHandler == NULL)
    this->TaskHandler = xTaskGetCurrentTaskHandle();
  this->_directionPin = DIR;
  this->_stepPin      = STEP;
  this->_enablePin    = EN;
  this->_resetPin     = RST;
  this->_sleepPin     = SLP;
  this->_stepsPerRevolution = number_of_steps_per_revolution;
  xSemaphoreGive(this->_mutex);

  esp_err_t esp_err;

  /// Configura il timer MoveHandlerTMR
  const esp_timer_create_args_t DRV8825_timer_args =
  {
    .callback = &__CallBackSteps,
    .arg = this,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "DRV8825_timer"
  };

  /// Si salva il nuovo timer
  esp_timer_handle_t tmrDRV8825;

  /// Crea il timer MoveHandlerTMR
  esp_err = esp_timer_create(&DRV8825_timer_args, &tmrDRV8825);
  if(esp_err != ESP_OK || tmrDRV8825 == nullptr)
    return DRV_ERR_TMR_CREATION;

  if(DIR == 255)
    return DRV_ERR_NO_DIR_PIN;
  pinMode(DIR, OUTPUT);
  digitalWriteFast(DIR, LOW);

  if(STEP == 255)
    return DRV_ERR_NO_STEP_PIN;
  pinMode(STEP, OUTPUT);
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

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;
    
  /// Ricopia l'handler del timer
  this->DRV8825_timer = tmrDRV8825;
  
  /// Crea impulso HIGH per 2.2µs + LOW per 2.2µs, questo è fisso così da non rompere...
  /// duration = _stepPulse.durationX * rmtTick = 1 * 2.2us
  _stepPulse[0].level0 = 1;
  _stepPulse[0].duration0 = 1;
  _stepPulse[0].level1 = 0;
  _stepPulse[0].duration1 = 1;
  xSemaphoreGive(this->_mutex);

  /// Setup RMT per il pin STEP -> @doc: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/rmt.html
  rmt_tx_channel_config_t rmt_tx_cfg =
  {
    .gpio_num = (gpio_num_t)_stepPin,
    .clk_src = RMT_CLK_SRC_DEFAULT,    // clock source (default)
    .resolution_hz = 80000000 / 176,   // periodo = 1/clk_freq = 176/80MHz = 2.2us          
    
    /// Necessario per risolvere l'errore descritto 
    /// in questo forum https://esp32.com/viewtopic.php?t=42301 
    /// mem = 48 * 4 = 192 bytes
    .mem_block_symbols = 48,
    .trans_queue_depth = 4,            // profondità della coda di trasferimento
    .intr_priority = 0,                // 0 -> priorità bassa
    .flags = 
    {
      .invert_out = 0,
      .with_dma = 0,
      .io_loop_back = 0,
      .io_od_mode = 0,
      .allow_pd = 0,
      .init_level = 1
    }
  };

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  /// Pulisce eventuale canale RMT precedente
  if(this->_rmtChannel != nullptr)
  {
    rmt_disable(this->_rmtChannel);
    rmt_del_channel(this->_rmtChannel);
    this->_rmtChannel = nullptr;
  }
  
  /// Pulisce eventuale canale step_encoder per RMT precedente
  if(this->step_encoder != nullptr)
  {
    rmt_del_encoder(this->step_encoder);
    this->step_encoder = nullptr;
  }

  esp_err = rmt_new_tx_channel(&rmt_tx_cfg, &this->_rmtChannel);
  if(esp_err != ESP_OK)
  {
    Serial.printf("Errore drv: %s\n", esp_err_to_name(esp_err));
    return DRV_ERR_RMT_CREATION;
  }

  /// Se il canale RMT è stato creato corretamente lo abilita
  rmt_enable(this->_rmtChannel);
  if(esp_err != ESP_OK)
    return DRV_ERR_RMT_ENABLE;
  
  rmt_copy_encoder_config_t enc_cfg = {};
  esp_err = rmt_new_copy_encoder(&enc_cfg, &this->step_encoder);
  if(esp_err != ESP_OK)
    return DRV_ERR_RMT_COPY_ENCODER;
  
  xSemaphoreGive(this->_mutex);

  return DRV_OK;
}

drv_err_t DRV8825::update()
{
  //uint32_t startTime = micros();

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  if(this->_abortCommand)
  {
    this->_isStepDone = true;
    this->_isContinuous = false;

    this->_stepsLeft = 0;
      
    this->_waitRmtAsyncTransmit = false;

    /// Dura solo una chiamata dell'update (eccezione quando non riesce a prendere il mutex)
    this->_abortCommand = false;

    /// Ferma il timer
    if(this->DRV8825_timer)
      esp_timer_stop(this->DRV8825_timer);

    xSemaphoreGive(this->_mutex);
    return DRV_CMD_ABORTED;
  }

  if(this->_waitRmtAsyncTransmit == false)
  {
    xSemaphoreGive(this->_mutex);

    /// Aspetta la notifica e se non arriva esce subito dalla funzione update
    if(ulTaskNotifyTake(pdTRUE, 0) <= 0)
      return DRV_NO_NOTIFY;

    if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
      return DRV_ERR_MUX_TAKE_TIMEOUT;
  
    /// Il movimento non è ancora finito
    this->_isStepDone = false;

    /// @return se ha finito gli steps che doveva fare...
    if(!_isContinuous && _stepsLeft == 0)
    {
      _isStepDone = true;
      
      /// Ferma il timer
      if(DRV8825_timer)
        esp_timer_stop(DRV8825_timer);

      xSemaphoreGive(this->_mutex);

      return DRV_OK;
    }
    
    /// Impulso di 2.2us HIGH e 2.2us LOW
    esp_err_t errTx = rmt_transmit(this->_rmtChannel, this->step_encoder, this->_stepPulse, sizeof(this->_stepPulse), &this->transmit_cfg);
    if(errTx != ESP_OK)
    {
      xSemaphoreGive(this->_mutex);
      return DRV_ERR_RMT_TRANSMIT_CMD;
    }

    /// Aspetta la trasmissione in modo asincrono
    this->_waitRmtAsyncTransmit = true;
    this->_timeoutRmtTransmit = millis();
  }

  rmt_channel_handle_t rmtChannelSafeCopy = this->_rmtChannel;

  xSemaphoreGive(this->_mutex);

  esp_err_t errTxWait = rmt_tx_wait_all_done(rmtChannelSafeCopy, -1);
    
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;
  
  /// Timeout in ms che aspetta la fine della trasmissione 
  if(errTxWait != ESP_OK)
  {
    if(millis() - this->_timeoutRmtTransmit >= __TIMEOUT_WAIT_ALL_DONE_RMT_TX_MS__)
    {
      xSemaphoreGive(this->_mutex); 
      return DRV_ERR_RMT_TX_TIMEOUT;
    }
    xSemaphoreGive(this->_mutex); 
    return DRV_WAITING_RMT_TX_TO_FINISH;
  }

  this->_waitRmtAsyncTransmit = false;
  
  if(!this->_isContinuous && this->_stepsLeft > 0)
  {
    this->_stepsLeft--;
    if(this->_stepsLeft == 0)
    {
      this->_isStepDone = true;

      if(this->DRV8825_timer)
        esp_timer_stop(this->DRV8825_timer);
    }
  }
  this->_absStepCounter += this->_direction;
  
  xSemaphoreGive(this->_mutex);
  

  //uint32_t stopTime = micros();
  //Serial.printf("Tempo di esecuzione __CallbackSteps: %d\n\n", stopTime - startTime);

  return DRV_OK;
}

drv_err_t DRV8825::setUpdateTask(TaskHandle_t handler)
{
  if(handler == NULL)
    return DRV_FAIL;

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  this->TaskHandler = handler;

  xSemaphoreGive(this->_mutex);

  return DRV_OK;
}


drv_err_t DRV8825::setDirection(drv_direction_t direction)
{
  // Se la direzione non è valida esce
  if(direction != DRV8825_CLOCK_WISE && direction != DRV8825_COUNTERCLOCK_WISE)
    return DRV_FAIL;

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  this->_direction = direction;
  drv_direction_t dirPin = this->_directionPin;
  xSemaphoreGive(this->_mutex);

  //  timing from datasheet 650 ns figure 1
  delayMicroseconds(1);
  digitalWriteFast(dirPin, direction == DRV8825_CLOCK_WISE ? HIGH : LOW );
  delayMicroseconds(1);

  return DRV_OK;
}


drv_direction_t DRV8825::getDirection()
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_FAIL;

  drv_direction_t dirPin = this->_directionPin;
  xSemaphoreGive(this->_mutex);
  
  return digitalReadFast(dirPin) == LOW ? DRV8825_COUNTERCLOCK_WISE : DRV8825_CLOCK_WISE;
}

/// @warning evitare di eseguire questa funzione quando il motore gira
drv_err_t DRV8825::setAbsPosition(int64_t absolute_position)
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;
  _absStepCounter = absolute_position;
  xSemaphoreGive(this->_mutex);
  return DRV_OK;
}

int64_t DRV8825::getAbsPosition()
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_FAIL;
  int64_t absolute_position = _absStepCounter;
  xSemaphoreGive(this->_mutex);

  return absolute_position;
}

drv_err_t DRV8825::isStepDone()
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  drv_err_t tmpIsStepDone = this->_isStepDone ? DRV_TRUE : DRV_FALSE;
  xSemaphoreGive(this->_mutex);
  
  return tmpIsStepDone;
}

drv_err_t DRV8825::step(uint64_t numberOfStepsToDo, uint64_t period_us)
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  this->_isContinuous = false;

  this->_stepsLeft = numberOfStepsToDo;
  xSemaphoreGive(this->_mutex);

  return setTmr(period_us);
}


void DRV8825::abortCurrentMovement()
{
  this->_abortCommand = true;
}

drv_err_t DRV8825::stepContinuous(uint64_t period_us)
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  this->_isContinuous = true;

  this->_stepsLeft = 0;
  xSemaphoreGive(this->_mutex);

  return setTmr(period_us);
}


//  Table page 3
drv_err_t DRV8825::enable()
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  uint8_t enPin = this->_enablePin;

  xSemaphoreGive(this->_mutex);

  if(enPin == 255)
    return DRV_ERR_NO_EN_PIN;

  digitalWriteFast(enPin, LOW);
  return DRV_OK;
}

drv_err_t DRV8825::disable()
{
  uint8_t enPin;

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;
    
  enPin = this->_enablePin;
  xSemaphoreGive(this->_mutex);

  if(enPin == 255)
    return DRV_ERR_NO_EN_PIN;

  digitalWriteFast(enPin, HIGH);
  return DRV_OK;
}

drv_err_t DRV8825::isEnabled()
{
  uint8_t enPin;
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;
    
  enPin = this->_enablePin;
  xSemaphoreGive(this->_mutex);

  if(enPin == 255)
    return DRV_ERR_NO_EN_PIN;

  return (digitalReadFast(enPin) == LOW ? DRV_TRUE : DRV_FALSE);
}


drv_err_t DRV8825::reset()
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  uint8_t rstPin = this->_resetPin;
  xSemaphoreGive(this->_mutex);

  if(rstPin == 255)
    return DRV_ERR_NO_RST_PIN;

  digitalWriteFast(rstPin, LOW);
  vTaskDelay(pdMS_TO_TICKS(1));
  digitalWriteFast(rstPin, HIGH);

  return DRV_OK;
}


drv_err_t DRV8825::sleep()
{
  uint8_t slpPin;

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_OK;
    
  slpPin = this->_sleepPin;
  xSemaphoreGive(this->_mutex);
  
  if(slpPin == 255)
    return DRV_FAIL;

  digitalWriteFast(slpPin, LOW);
  return DRV_OK;
}

drv_err_t DRV8825::wakeup()
{
  uint8_t slpPin;
  
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;
  
  slpPin = this->_sleepPin;
  xSemaphoreGive(this->_mutex);

  if(slpPin == 255)
    return DRV_FAIL;

  digitalWriteFast(slpPin, HIGH);
  return DRV_OK;
}

bool DRV8825::isSleeping()
{  
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return false;

  uint8_t slpPin = this->_sleepPin;
  xSemaphoreGive(this->_mutex);

  if(slpPin == 255)
    return false;

  return (digitalReadFast(slpPin) == LOW);
}


drv_err_t DRV8825::setTmr(uint64_t period_us)
{
  /// Per proprietà hardware del DRV8825 3.8us è il periodo minimo di lavoro dello step pin
  if(period_us <= DRV8825_MIN_PERIOD_US)
    period_us = DRV8825_MIN_PERIOD_US;
  
  uint64_t __prev_period_us;

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;
    
  esp_timer_handle_t tmrDRV8825;

  if(this->DRV8825_timer)
    tmrDRV8825 = this->DRV8825_timer;
  else
  {
    xSemaphoreGive(this->_mutex);
    return DRV_ERR_TMR_UNDEFINED;
  }
  xSemaphoreGive(this->_mutex);

  esp_timer_get_period(tmrDRV8825, &__prev_period_us);

  /// Se il periodo è diverso e il timer è già attivo...
  if(period_us != __prev_period_us)
  {
    /// Fermiamo il timer (richiesto esplicitamente dalla documentazione)
    esp_timer_stop(tmrDRV8825);
      
    /// Avvia il timer con il ritardo specificato in microsecondi
    esp_timer_start_periodic(tmrDRV8825, period_us);
  }
  
  return DRV_OK;
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





/**
  * @brief Ritorna una stringa di codici errori di tipo drv_err_t 
  * 
  * @param code codice errore drv_err_t
  * 
  * @return stringa del messaggio d'errore
  */
const char *drv_err_to_name(drv_err_t code)
{
  switch(code)
  {
    case DRV_TRUE :                     return "DRV_TRUE";
    case DRV_FALSE :                    return "DRV_FALSE";
    case DRV_OK   :                     return "DRV_OK";
    case DRV_FAIL :                     return "DRV_FAIL";
    case DRV_NO_NOTIFY :                return "DRV_NO_NOTIFY";
    case DRV_ERR_MUX_CREATION :         return "DRV_ERR_MUX_CREATION";
    case DRV_ERR_MUX_TAKE_TIMEOUT :     return "DRV_ERR_MUX_TAKE_TIMEOUT";
    case DRV_ERR_TMR_CREATION :         return "DRV_ERR_TMR_CREATION";
    case DRV_ERR_TMR_UNDEFINED :        return "DRV_ERR_TMR_UNDEFINED"; 
    case DRV_ERR_NO_DIR_PIN :           return "DRV_ERR_NO_DIR_PIN";
    case DRV_ERR_NO_STEP_PIN :          return "DRV_ERR_NO_STEP_PIN";
    case DRV_ERR_NO_EN_PIN :            return "DRV_ERR_NO_EN_PIN";
    case DRV_ERR_NO_SLP_PIN :           return "DRV_ERR_NO_SLP_PIN";
    case DRV_ERR_NO_RST_PIN :           return "DRV_ERR_NO_RST_PIN"; 
    case DRV_WAITING_RMT_TX_TO_FINISH : return "DRV_WAITING_RMT_TX_TO_FINISH";
    case DRV_ERR_RMT_CREATION :         return "DRV_ERR_RMT_CREATION";
    case DRV_ERR_RMT_ENABLE :           return "DRV_ERR_RMT_ENABLE";
    case DRV_ERR_RMT_COPY_ENCODER :     return "DRV_ERR_RMT_COPY_ENCODER";
    case DRV_ERR_RMT_TX_TIMEOUT :       return "DRV_ERR_RMT_TX_TIMEOUT";
    case DRV_ERR_RMT_TRANSMIT_CMD :     return "DRV_ERR_RMT_TRANSMIT_CMD";
    case DRV_CMD_ABORTED :              return "DRV_CMD_ABORTED";
    //case DRV_ERR_ :                   return "DRV_ERR_"; 
    default :                           return "NOT_A_DRV_CODE";
  }
}

//  -- END OF FILE --