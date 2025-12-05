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


#define __MUTEX_TIMEOUT__ portMAX_DELAY


/// FUNZIONE NON MEMBRO DELLA CLASSE PER SAPERE QUALE CANALE RMT ASSEGNARE
static portMUX_TYPE used_channels_lock = portMUX_INITIALIZER_UNLOCKED;
static bool used_channels[8] = {false};

rmt_channel_t findFreeRMTChannel()
{
  portENTER_CRITICAL(&used_channels_lock);
  for(uint8_t i = 0; i < 8; i++)
    if(!used_channels[i])
    {
      used_channels[i] = true;
      portEXIT_CRITICAL(&used_channels_lock);
      return rmt_channel_t(i);
    }
  portEXIT_CRITICAL(&used_channels_lock);

    return RMT_CHANNEL_MAX; // nessun canale disponibile
}



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

  /// Libera l'RMT channel
  if(this->_rmtChannel != RMT_CHANNEL_MAX)
  {
    rmt_driver_uninstall(this->_rmtChannel);
    used_channels[this->_rmtChannel] = false;
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
  rmt_channel_t rmtCh;
  xSemaphoreTake(_mutex, __MUTEX_TIMEOUT__);
  this->_directionPin = DIR;
  this->_stepPin      = STEP;
  this->_enablePin    = EN;
  this->_resetPin     = RST;
  this->_sleepPin     = SLP;
  this->_stepsPerRevolution = number_of_steps_per_revolution;
  rmtCh = this->_rmtChannel;
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

  /// Trova il primo canale dell'RMT disponibile, se non lo è riutilizza il canale precedentemente assegnato
  if(rmtCh == RMT_CHANNEL_MAX)
    rmtCh = findFreeRMTChannel();

  if(rmtCh == RMT_CHANNEL_MAX)
  {
    Serial.println("Errore DRV8825 RMT Init, Non ci sono più canali RMT disponibili (0 su 8)");
    return false;
  }
    
  portENTER_CRITICAL(&_spinlock);
  if(this->_rmtChannel != rmtCh)
    _rmtChannel = rmtCh;
    
  /// Ricopia l'handler del timer
  this->DRV8825_timer = tmrDRV8825;
  
  /// Crea impulso HIGH per 2.2µs + LOW per 2.2µs, questo è fisso così da non rompere...
  /// duration = _stepPulse.durationX * rmtTick = 1 * 2.2us
  _stepPulse[0].level0 = 1;
  _stepPulse[0].duration0 = 1;
  _stepPulse[0].level1 = 0;
  _stepPulse[0].duration1 = 1;
  portEXIT_CRITICAL(&_spinlock);

  /// Setup RMT per il pin STEP
  rmt_config_t rmt_tx;
  rmt_tx.rmt_mode = RMT_MODE_TX;
  rmt_tx.channel = _rmtChannel;
  rmt_tx.gpio_num = (gpio_num_t)_stepPin;
  rmt_tx.clk_div = 176; // 2.2 µs di risoluzione (80 MHz / 153 ~= 522.875 kHz) (il drv al massimo deve avere almeno 1.9us di impulso)
  rmt_tx.mem_block_num = 1;
  rmt_tx.tx_config.loop_en = false;
  rmt_tx.tx_config.carrier_en = false;
  rmt_tx.tx_config.idle_output_en = true;
  rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;

  rmt_config(&rmt_tx);
  esp_err = rmt_driver_install(rmt_tx.channel, 0, 0);
  if(esp_err != ESP_OK)
    return false;

  return true;
}

bool DRV8825::setDirection(int8_t direction)
{
  // Se la direzione non è valida esce
  if(direction != DRV8825_CLOCK_WISE && direction != DRV8825_COUNTERCLOCK_WISE)
    return false;

  uint8_t dirPin;
  
  portENTER_CRITICAL(&_spinlock);
  this->_direction = direction;
  dirPin = this->_directionPin;
  portEXIT_CRITICAL(&_spinlock);

  //  timing from datasheet 650 ns figure 1
  delayMicroseconds(1);
  digitalWriteFast(dirPin, direction == DRV8825_CLOCK_WISE ? LOW : HIGH);
  delayMicroseconds(1);

  return true;
}


int8_t DRV8825::getDirection()
{
  portENTER_CRITICAL(&_spinlock);
  uint8_t dirPin = this->_directionPin;
  portEXIT_CRITICAL(&_spinlock);
  
  return digitalReadFast(dirPin) == LOW ? DRV8825_CLOCK_WISE : DRV8825_COUNTERCLOCK_WISE;
}

/// @warning evitare di eseguire questa funzione quando il motore gira
void DRV8825::setAbsPosition(int64_t absolute_position)
{  
  portENTER_CRITICAL(&_spinlock);
  _absStepCounter = absolute_position;
  portEXIT_CRITICAL(&_spinlock);
}

int64_t DRV8825::getAbsPosition()
{
  int64_t absolute_position;

  portENTER_CRITICAL(&_spinlock);
  absolute_position = _absStepCounter;
  portEXIT_CRITICAL(&_spinlock);

  return absolute_position;
}

bool DRV8825::isStepDone()
{
  bool tmpIsStepDone;

  portENTER_CRITICAL(&_spinlock);
  tmpIsStepDone = this->_isStepDone;
  portEXIT_CRITICAL(&_spinlock);
  
  return tmpIsStepDone;
}

void DRV8825::step(uint64_t numberOfStepsToDo, uint64_t period_us)
{
  portENTER_CRITICAL(&_spinlock);
  this->_isContinuous = false;

  this->_isStepDone = false;

  this->_stepsLeft = numberOfStepsToDo;
  portEXIT_CRITICAL(&_spinlock);

  setTmr(period_us);
}

void stepGradi(double gradi, uint64_t period_us)
{
  /*
  xSemaphoreTake(_mutex);
  
  xSemaphoreGive(_mutex);
  uint64_t stepsToDo = 2233; /// Numero a caso lol
  */
}

void DRV8825::stepContinuous(uint64_t period_us)
{
  portENTER_CRITICAL(&_spinlock);
  this->_isContinuous = true;

  this->_isStepDone = false;

  this->_stepsLeft = 0;
  portEXIT_CRITICAL(&_spinlock);

  setTmr(period_us);
}


//  Table page 3
bool DRV8825::enable()
{
  uint8_t enPin;

  portENTER_CRITICAL(&_spinlock);
  enPin = this->_enablePin;
  portEXIT_CRITICAL(&_spinlock);

  if(enPin == 255)
    return false;

  digitalWriteFast(enPin, LOW);
  return true;
}

bool DRV8825::disable()
{
  uint8_t enPin;

  portENTER_CRITICAL(&_spinlock);
  enPin = this->_enablePin;
  portEXIT_CRITICAL(&_spinlock);

  if(enPin == 255)
    return false;

  digitalWriteFast(enPin, HIGH);
  return true;
}

bool DRV8825::isEnabled()
{
  uint8_t enPin;

  portENTER_CRITICAL(&_spinlock);
  enPin = this->_enablePin;
  portEXIT_CRITICAL(&_spinlock);

  if(enPin == 255)
    return false;

  return (digitalReadFast(enPin) == LOW);
}


bool DRV8825::reset()
{
  uint8_t rstPin;

  portENTER_CRITICAL(&_spinlock);
  rstPin = this->_resetPin;
  portEXIT_CRITICAL(&_spinlock);

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
  
  portENTER_CRITICAL(&_spinlock);
  slpPin = this->_sleepPin;
  portEXIT_CRITICAL(&_spinlock);
  
  if(slpPin == 255)
    return false;

  digitalWriteFast(slpPin, LOW);
  return true;
}

bool DRV8825::wakeup()
{
  uint8_t slpPin;
  
  portENTER_CRITICAL(&_spinlock);
  slpPin = this->_sleepPin;
  portEXIT_CRITICAL(&_spinlock);

  if(slpPin == 255)
    return false;

  digitalWriteFast(slpPin, HIGH);
  return true;
}

bool DRV8825::isSleeping()
{
  uint8_t slpPin;
  
  portENTER_CRITICAL(&_spinlock);
  slpPin = this->_sleepPin;
  portEXIT_CRITICAL(&_spinlock);

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
  //uint32_t startTime = micros();

  DRV8825 *INST = static_cast<DRV8825*>(args);
  if(!INST)
    return;

  bool exitCondition = false;


  portENTER_CRITICAL(&INST->_spinlock);
  /// @return se ha finito gli steps che doveva fare
  if(!INST->_isContinuous && INST->_stepsLeft == 0)
  {
    INST->_isStepDone = true;
    exitCondition = true;
  }
  portEXIT_CRITICAL(&INST->_spinlock);
  
  if(exitCondition)
  {
    /// Ferma il timer
    if(INST->DRV8825_timer)
      esp_timer_stop(INST->DRV8825_timer);
    return;
  }

  /// Controlla che l'RMT channel sia definito
  if(INST->_rmtChannel == RMT_CHANNEL_MAX)
    return;

  /// Impulso di 2.2us HIGH e 2.2us LOW
  esp_err_t err = rmt_write_items(INST->_rmtChannel, INST->_stepPulse, 1, true);
  if(err == ESP_OK)
  {
    portENTER_CRITICAL(&INST->_spinlock);
    if(!INST->_isContinuous && INST->_stepsLeft > 0)
    {
      INST->_stepsLeft--;
      if (INST->_stepsLeft == 0)
      {
        INST->_isStepDone = true;
        exitCondition = true;
      }
    }
    INST->_absStepCounter += INST->_direction;
    portEXIT_CRITICAL(&INST->_spinlock);

    if(exitCondition && INST->DRV8825_timer)
      esp_timer_stop(INST->DRV8825_timer);
  }
  
  //uint32_t stopTime = micros();
  //Serial.printf("Tempo di esecuzione __CallbackSteps: %d\n\nErrore RMT: %sus\n", stopTime - startTime, err==ESP_OK ? "ESP_OK" : "ESP_ERROR");
}
//  -- END OF FILE --
