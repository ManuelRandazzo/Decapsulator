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

#define __MUTEX_TIMEOUT_TICKS__ pdMS_TO_TICKS(1000)
#define __TIMEOUT_WAIT_ALL_DONE_RMT_TX_MS__ 10    /// NON E' BLOCCANTE, MA TENERLO BREVE COMUNQUE


bool IRAM_ATTR drv8825_rmt_tx_done_cb(
        rmt_channel_handle_t channel,
        const rmt_tx_done_event_data_t *edata,
        void *user_data)
{
    DRV8825 *drv = static_cast<DRV8825*>(user_data);

    BaseType_t hpTaskWoken = pdFALSE;

    drv->_rmtBusy = false;

    if(drv->TaskHandler != nullptr)
        xTaskNotifyFromISR(drv->TaskHandler, 1, eSetBits, &hpTaskWoken);

    return hpTaskWoken == pdTRUE;
}

DRV8825::DRV8825()
{
}

DRV8825::~DRV8825()
{
  /// Elimina il canale RMT
  if(this->_rmtChannel != nullptr)
  {
    rmt_disable(this->_rmtChannel);
    rmt_del_channel(this->_rmtChannel);
  }

  /// Elimina il mutex
  if(this->_mutex != nullptr)
    vSemaphoreDelete(this->_mutex);
}

drv_err_t DRV8825::begin(uint8_t DIR, uint8_t STEP, uint8_t EN, uint8_t RST, uint8_t SLP, uint16_t number_of_steps_per_revolution)
{
  esp_err_t esp_err = ESP_OK;

  /// Disattiva dei log fastidiosissimi di quando la payload dell'RMT non è finita
  esp_log_level_set("*", ESP_LOG_NONE);
  
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

  /// Setup RMT per il pin STEP -> @doc: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/rmt.html
  rmt_tx_channel_config_t rmt_tx_cfg =
  {
    .gpio_num = (gpio_num_t)_stepPin,
    .clk_src = RMT_CLK_SRC_DEFAULT,    // clock source (default)
    .resolution_hz = 80000000 / DRV8825_RMT_PSC,   // Prescaler : 176 / 80MHz = 2.2us (periodo di uno step)

    /// Necessario per risolvere l'errore descritto 
    /// in questo forum https://esp32.com/viewtopic.php?t=42301 
    /// mem = 48 * 4 = 192 bytes
    .mem_block_symbols = 48,
    .trans_queue_depth = 4,            // profondità della coda di trasferimento
    .intr_priority = 2,                // 0 -> priorità bassa
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

  rmt_tx_event_callbacks_t tx_event_callback = { .on_trans_done = drv8825_rmt_tx_done_cb, };

  esp_err = rmt_tx_register_event_callbacks(this->_rmtChannel, &tx_event_callback, this);

  if(esp_err != ESP_OK)
    return DRV_ERR_RMT_CREATION;
  
  rmt_copy_encoder_config_t enc_cfg = {};
  esp_err = rmt_new_copy_encoder(&enc_cfg, &this->step_encoder);
  if(esp_err != ESP_OK)
    return DRV_ERR_RMT_COPY_ENCODER;

  xSemaphoreGive(this->_mutex);

  return DRV_OK;
}

/*
drv_err_t DRV8825::update()
{
  //uint32_t startTime = micros();
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
      return DRV_ERR_MUX_TAKE_TIMEOUT;

  if(this->_isStepDone)
  {
    xSemaphoreGive(this->_mutex);
    return DRV_OK;
  }

  if(!this->_rmtBusy)
  {
    if(this->_stepsLeft == 0)
    {
      this->_isStepDone = true;
      xSemaphoreGive(this->_mutex);
      return DRV_OK;
    }

    /// Quanti step dovrà fare nella prossima trasmissione
    uint16_t next_tx_steps = std::min<uint32_t>(this->_stepsLeft, DRV8825_RMT_MAX_LOOP_COUNT);

    this->transmit_cfg.loop_count = next_tx_steps;    
    
    esp_err_t errTx = rmt_transmit(this->_rmtChannel, this->step_encoder, &this->_stepPulse, this->STEP_PULSE_SIZE, &this->transmit_cfg);

    if(errTx != ESP_OK)
    {
      rmt_disable(this->_rmtChannel);
      xSemaphoreGive(this->_mutex);
      return DRV_ERR_RMT_TRANSMIT_CMD;
    }

    this->_tmrStartOfRmtTransmit = millis();
    this->_stepsLeft -= next_tx_steps;
    this->_rmtBusy = true;

    xSemaphoreGive(this->_mutex);
    return DRV_WAITING_RMT_TX_TO_FINISH;
  }

  if(rmt_tx_wait_all_done(this->_rmtChannel, 0) == ESP_OK)
  {
    this->_rmtBusy = false;

    if(this->_stepsLeft == 0)
    {
      this->_isStepDone = true;
      xSemaphoreGive(this->_mutex);
      return DRV_OK;
    }
  }

  xSemaphoreGive(this->_mutex);

  //uint32_t stopTime = micros();
  //Serial.printf("Tempo di esecuzione update : %d\n\n", stopTime - startTime);
  
  return DRV_WAITING_RMT_TX_TO_FINISH;
}*/

drv_err_t DRV8825::update()
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  if(this->_isStepDone)
  {
    xSemaphoreGive(this->_mutex);
    return DRV_OK;
  }

  if(this->_rmtBusy.load(std::memory_order_acquire))
  {
    xSemaphoreGive(this->_mutex);
    return DRV_WAITING_RMT_TX_TO_FINISH;
  }

  if(this->_stepsLeft == 0)
  {
    this->_isStepDone = true;
    this->_absStepCounter += this->_direction * this->_stepsRequestedThisMove;
    xSemaphoreGive(this->_mutex);
    return DRV_OK;
  }

  uint16_t next_tx_steps = std::min<uint32_t>(
      this->_stepsLeft,
      DRV8825_RMT_MAX_LOOP_COUNT
  );

  this->transmit_cfg.loop_count = next_tx_steps;

  esp_err_t errTx = rmt_transmit(
      this->_rmtChannel,
      this->step_encoder,
      &this->_stepPulse,
      this->STEP_PULSE_SIZE,
      &this->transmit_cfg
  );

  if(errTx != ESP_OK)
  {
    xSemaphoreGive(this->_mutex);
    return DRV_ERR_RMT_TRANSMIT_CMD;
  }

  this->_stepsLeft -= next_tx_steps;
  this->_rmtBusy.store(true, std::memory_order_release);

  xSemaphoreGive(this->_mutex);

  return DRV_WAITING_RMT_TX_TO_FINISH;
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

  this->_absStepCounter = absolute_position;

  xSemaphoreGive(this->_mutex);

  return DRV_OK;
}

int64_t DRV8825::getAbsPosition()
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_FAIL;

  int64_t absolute_position = this->_absStepCounter;

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

drv_err_t DRV8825::step(uint64_t numberOfStepsToDo, uint64_t period_us, int64_t acc, int64_t dec)
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  /// Se il numero di step è più alto di 32767 (max di loop_count)
  /// deve essere fatto ripartire più volte
  this->_stepsLeft = numberOfStepsToDo;
  this->_stepsRequestedThisMove = (int64_t)numberOfStepsToDo;
    
  /// Imposta l'RMT per andare per un certo numero di step
  this->setRMT(period_us);
  
  this->_isStepDone = false;

  xSemaphoreGive(this->_mutex);

  return DRV_OK;
}


drv_err_t DRV8825::abortCurrentMovement()
{
  const uint32_t tmrEndSteps = millis();
  rmt_channel_handle_t rmtCh;

  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  this->_isStepDone = true;

  this->_stepsLeft = 0;

  if(this->_period_us > 0)
  {
    int64_t stepsFatti = (tmrEndSteps - this->_tmrStartOfRmtTransmit) / this->_period_us;
    this->_absStepCounter += this->_direction * stepsFatti;
  }

  this->_period_us = 0;

  this->_tmrStartOfRmtTransmit = 0;
    
  this->_rmtBusy = false;

  rmtCh = this->_rmtChannel;

  xSemaphoreGive(this->_mutex);

  rmt_disable(rmtCh);

  return DRV_CMD_ABORTED;
}

drv_err_t DRV8825::stepContinuous(uint64_t period_us)
{
  if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
    return DRV_ERR_MUX_TAKE_TIMEOUT;

  /// Imposta l'RMT per andare all'infinito (fino a quando non si chiama abortCurrentCommand())
  this->setRMT(period_us);
  this->transmit_cfg.loop_count = -1;
  this->_tmrStartOfRmtTransmit = millis();
  rmt_transmit(this->_rmtChannel, this->step_encoder, &this->_stepPulse, this->STEP_PULSE_SIZE, &this->transmit_cfg);
  this->_isStepDone = false;
  
  xSemaphoreGive(this->_mutex);

  return DRV_OK;
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

void DRV8825::setRMT(uint64_t period_us)
{
  /// Per proprietà hardware del DRV8825 3.8us è il periodo minimo di lavoro dello step pin
  if(period_us <= DRV8825_MIN_PERIOD_US)
    period_us = DRV8825_MIN_PERIOD_US;

  this->_period_us = period_us;
  /// duration = _stepPulse.durationX * rmtTick
  this->_stepPulse[0].duration1 = (period_us / DRV8825_RMT_PULSE_US) - 1; // level LOW,   il -1 rappresenta l'HIGH

  /// Abilita il canale RMT
  rmt_enable(this->_rmtChannel);
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