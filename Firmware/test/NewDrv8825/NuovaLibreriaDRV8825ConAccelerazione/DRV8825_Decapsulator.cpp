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

#define __MUTEX_TIMEOUT_TICKS__ pdMS_TO_TICKS(25)


bool IRAM_ATTR drv8825_rmt_tx_done_cb(rmt_channel_handle_t channel, const rmt_tx_done_event_data_t *edata, void *user_data)
{
    DRV8825 *drv = static_cast<DRV8825*>(user_data);

    BaseType_t HigherPriorityTaskWoken = pdFALSE;

    /// Setta a false _rmtBusy in modo atomico
    drv->_rmtBusy.store(false, std::memory_order_release);

    /// Se è stato fornito l'handler di una task a cui inviare la notifica viene fatto
    if(drv->TaskHandler != nullptr)
        xTaskNotifyFromISR(drv->TaskHandler, 1, eSetBits, &HigherPriorityTaskWoken);

    return HigherPriorityTaskWoken == pdTRUE;
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
        .resolution_hz = 80000000 / DRV8825_RMT_PSC,   // Prescaler : 160 / 80MHz = 2us (periodo di uno step)

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

drv_err_t DRV8825::update()
{
    static uint32_t StartTime = 0;
    if(this->_isStepDone.load(std::memory_order_acquire))
        return DRV_OK;

    /// @link https://en.cppreference.com/cpp/atomic/atomic/load
    if(this->_rmtBusy.load(std::memory_order_acquire))
        return DRV_RMT_TX_BUSY;

    uint64_t localStepsLeft = this->_stepsLeft.load(std::memory_order_acquire);
  
    /// protegge atomicamente in lettura (acquire)
    if(localStepsLeft == 0)
    {
        const uint32_t endTime = micros();
        Serial.printf("Execution Time : %.6fs\n", double(endTime - StartTime) / 1000000.0);
        Serial.printf("Error Time : %dus\n\n\n", 1000000 - (endTime - StartTime));
        StartTime = 0;
        this->_isStepDone.store(true, std::memory_order_release);
        StatoMoto = ACCELERATION;
        return DRV_OK;
    }

    if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
      return DRV_ERR_MUX_TAKE_TIMEOUT;

    uint16_t next_tx_steps;
    switch(this->StatoMoto)
    {
        case ACCELERATION :
            next_tx_steps = 1;

            if(localStepsLeft <= this->_acc_end_steps)
            {
                this->_acc_end_steps = 0;


                if(this->_const_end_steps != 0)
                {
                    /// Profilo trapezoidale
                    StatoMoto = CONSTANT;
                }
                else 
                    /// Profilo triangolare
                    StatoMoto = DECELERATION;

            }
            else
                this->_stepPulse[0].duration1 = this->_duration_acc_dec[duration_acc_dec_index++];

        break;

        case CONSTANT :
        {
            this->_stepPulse[0].duration1 = (uint16_t)(this->_period_us);

            uint32_t remaining_const = localStepsLeft - this->_const_end_steps;
            next_tx_steps = std::min<uint32_t>(remaining_const, DRV8825_RMT_MAX_LOOP_COUNT);
            Serial.printf("next_tx_steps const : %d\n", next_tx_steps);

            // Bug #1 fix: transisci a DEC solo quando tutti i const step sono esauriti
            if(next_tx_steps >= remaining_const)
                StatoMoto = DECELERATION;
            // else: rimane CONSTANT al prossimo update()

        }
        break;

        case DECELERATION :
            next_tx_steps = 1;

            this->_stepPulse[0].duration1 = this->_duration_acc_dec[duration_acc_dec_index++];

        break;
    }

    xSemaphoreGive(this->_mutex);

    /// Feed degli steps all'hardware
    this->transmit_cfg.loop_count = next_tx_steps;

    esp_err_t errTx = rmt_transmit(this->_rmtChannel, this->step_encoder, &this->_stepPulse, this->STEP_PULSE_SIZE, &this->transmit_cfg);

    if(StartTime == 0)
        StartTime = micros();

    if(errTx != ESP_OK)
        return DRV_ERR_RMT_TRANSMIT_CMD;

    /// E' come fare (stepsLeft -= next_tx_steps) @link https://en.cppreference.com/cpp/atomic/atomic/fetch_sub
    this->_stepsLeft.fetch_sub(next_tx_steps, std::memory_order_release);

    /// Setta a true _rmtBusy in modo atomico @link https://en.cppreference.com/cpp/atomic/atomic/store
    this->_rmtBusy.store(true, std::memory_order_release);

    return DRV_RMT_TX_BUSY;
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
    /// Se la direzione non è valida esce
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
 
    if(this->_period_us > 0)
    {
        /// Salva il tempo
        uint32_t tmrEndSteps = micros();
        int64_t stepsFatti = (tmrEndSteps - this->_tmrStartOfRmtTransmit) / this->_period_us;
        this->_absStepCounter += this->_direction * stepsFatti;
    }
    int64_t absolute_position = this->_absStepCounter;

    xSemaphoreGive(this->_mutex);

    return absolute_position;
}

drv_err_t DRV8825::isStepDone()
{  
    return this->_isStepDone.load(std::memory_order_acquire) ? DRV_TRUE : DRV_FALSE;
}

drv_err_t DRV8825::step(uint64_t numberOfStepsToDo, uint64_t period_us, uint64_t acceleration_step_s2, uint64_t deceleration_step_s2)
{
    Serial.printf("ACCELERATION = %d\n", acceleration_step_s2);
    Serial.printf("DECELERATION = %d\n", deceleration_step_s2);
    /// Se il numero di step è più alto di 32767 (max di loop_count)
    /// deve essere fatto ripartire più volte
    this->_stepsLeft.store(numberOfStepsToDo, std::memory_order_release);

    if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
        return DRV_ERR_MUX_TAKE_TIMEOUT;
    
    /// Imposta l'RMT per andare per un certo numero di step
    this->setAndEnableRMT(period_us, acceleration_step_s2, deceleration_step_s2);

    xSemaphoreGive(this->_mutex);
    
    this->_isStepDone.store(false, std::memory_order_release);

    return DRV_OK;
}


uint64_t DRV8825::abortCurrentMovement()
{
    esp_err_t err;

    /// Salva il tempo in cui viene effettivamente disattivato il canale RMT
    uint32_t tmrEndSteps;
    const uint32_t tmoRmtDisableErr = micros();
    do
    {
        err = rmt_disable(this->_rmtChannel);

        /// Salva il tempo in cui viene effettivamente disattivato il canale RMT
        tmrEndSteps = micros();
    }
    while(err != ESP_OK && (tmrEndSteps - tmoRmtDisableErr < 200));

    if(err != ESP_OK)
        return 0;

    /// release = protegge atomicamente in scrittura
    this->_isStepDone.store(true, std::memory_order_release);

    /// acquire = protegge atomicamente in lettura
    uint64_t passiRimanenti = this->_stepsLeft.load(std::memory_order_acquire);

    /// release = protegge atomicamente in scrittura
    this->_stepsLeft.store(0, std::memory_order_release); 

    /// Setta a false _rmtBusy in modo atomico. release = protegge atomicamente in scrittura
    this->_rmtBusy.store(false, std::memory_order_release);

    if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
        return passiRimanenti;

    if(this->_period_us > 0)
    {
        int64_t stepsFatti = (tmrEndSteps - this->_tmrStartOfRmtTransmit) / this->_period_us;
        this->_absStepCounter += this->_direction * stepsFatti;
    }

    this->_period_us = 0;

    this->_tmrStartOfRmtTransmit = 0;


    xSemaphoreGive(this->_mutex);

    return passiRimanenti;
}

drv_err_t DRV8825::stepContinuous(uint64_t period_us)
{
    if(xSemaphoreTake(this->_mutex, __MUTEX_TIMEOUT_TICKS__) == pdFAIL)
        return DRV_ERR_MUX_TAKE_TIMEOUT;

    /// Imposta l'RMT per andare all'infinito (fino a quando non si chiama abortCurrentCommand())
    this->setAndEnableRMT(period_us, 0, 0);
    this->transmit_cfg.loop_count = -1;
    this->_tmrStartOfRmtTransmit = micros();
    rmt_transmit(this->_rmtChannel, this->step_encoder, &this->_stepPulse, this->STEP_PULSE_SIZE, &this->transmit_cfg);
    
    xSemaphoreGive(this->_mutex);

    this->_isStepDone.store(false, std::memory_order_release);
    
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

void DRV8825::setAndEnableRMT(uint64_t period_us, const uint64_t ACC_STEPS_S2, const uint64_t DEC_STEPS_S2)
{
    /// Per proprietà hardware del DRV8825 3.8us è il periodo minimo di lavoro dello step pin
    if(period_us <= DRV8825_MIN_PERIOD_US)
        period_us = DRV8825_MIN_PERIOD_US;

    this->_period_us = period_us;

    this->calcRampSteps(ACC_STEPS_S2, DEC_STEPS_S2);

    /// Abilita il canale RMT
    rmt_enable(this->_rmtChannel);
}

void DRV8825::calcRampSteps(const uint64_t ACC_STEPS_S2, const uint64_t DEC_STEPS_S2)
{
    /// acquire = protegge atomicamente in lettura
    uint64_t spazio_tot = this->_stepsLeft.load(std::memory_order_acquire);

    /// Inizialmente da per scontato non ci siano né acc. che dec. nel caso in cui esca dalla funzione   
    this->_stepPulse[0].duration1 = (this->_period_us / DRV8825_RMT_PULSE_US - DRV8825_RMT_DURATION_0);
    Serial.printf("Duration Base : %d\n", this->_stepPulse[0].duration1 + this->_stepPulse[0].duration0);
    this->_const_end_steps = 0;   
    this->StatoMoto = CONSTANT;

    if(spazio_tot == 0 || ACC_STEPS_S2 == 0 || DEC_STEPS_S2 == 0)
        return;

    double k =  (double)(ACC_STEPS_S2 + DEC_STEPS_S2) /  (double)(2 * ACC_STEPS_S2 * DEC_STEPS_S2);

    /// Tempo in secondi
    double t_tot = ((double)(this->_period_us) / 1000000) * spazio_tot ;

    double delta = (t_tot * t_tot) - (4 * k * spazio_tot);

    Serial.printf("spazio_tot : %d\n", spazio_tot);
    Serial.printf("Tot : %.10f\n", t_tot);
    Serial.printf("K : %.10f\n", k);
    Serial.printf("Delta : %.40f\n", delta);
    if(delta < 0.0)
        return;

    double vel_max = (t_tot - sqrt(delta)) / (2 * k);
    Serial.printf("vel_max : %.5f\n", vel_max);

    /// Tempi del profilo trapezoidale in secondi
    double t_acc   = vel_max / (double)(ACC_STEPS_S2);
    double t_dec   = vel_max / (double)(DEC_STEPS_S2);
    double t_const = t_tot - t_acc - t_dec;
    Serial.printf("t_acc : %.10f\n", t_acc);
    Serial.printf("t_dec : %.10f\n", t_dec);
    Serial.printf("t_const : %.10f\n", t_const);

    /// Spazi del profilo trapezoidale in step
    uint64_t steps_acc   = (uint64_t)(((double)(ACC_STEPS_S2) / 2) * (t_acc*t_acc));
    uint64_t steps_dec   = (uint64_t)(((double)(DEC_STEPS_S2) / 2) * (t_dec*t_dec));
    uint64_t steps_const = spazio_tot - steps_dec - steps_acc;

    Serial.printf("steps_acc : %d\n", steps_acc);
    Serial.printf("steps_dec : %.d\n", steps_dec);
    Serial.printf("steps_const : %d\n", steps_const);
    Serial.printf("steps_tot_reali : %d\n", steps_acc + steps_const + steps_dec);

    /// A che step finisce ogni sezione, acc, const e dec=0 sempre
    this->_acc_end_steps = spazio_tot - steps_acc;
    if(delta == 0)
        this->_const_end_steps = 0;
    else
        this->_const_end_steps = spazio_tot - steps_acc - steps_const;

    /// Adatta il periodo in base alla nuova velocità massima trovata
    double vel_media = spazio_tot / t_tot;
    uint64_t v_max_us = (uint64_t)(this->_period_us * (vel_media / vel_max));
    this->_period_us = (uint16_t)(v_max_us / DRV8825_RMT_PULSE_US) - DRV8825_RMT_DURATION_0;
    setAccDecDurations(v_max_us, steps_acc, t_acc, steps_dec, t_dec);
    this->StatoMoto = ACCELERATION;

    Serial.printf("v_max_us : %d\n", v_max_us);
    Serial.printf("_const_end_steps : %d\n", this->_const_end_steps);
}

void DRV8825::setAccDecDurations(uint32_t v_max_us, uint32_t n_acc_steps, double t_acc, uint32_t n_dec_steps, double t_dec)
{
    /// Prima parte di sgrossatura del profilo trapezoidale/triangolare

    double total_time_us = 0;
    if(this->_duration_acc_dec != nullptr)
    {
        delete[] this->_duration_acc_dec;
        this->_duration_acc_dec = nullptr;
    }

    uint32_t total = n_acc_steps + n_dec_steps;
    if(total == 0)
        return;

    this->_duration_acc_dec = new uint16_t[total];
    this->duration_acc_dec_index = 0;

    // Sicurezza: evita divisioni inutili
    if(n_acc_steps > 0)
    {
        const double t_start = v_max_us;
        const double t_end   = v_max_us * 0.4;

        for(uint32_t i = 0; i < n_acc_steps; i++)
        {
            double ratio = (double)i / (double)(n_acc_steps - 1);
            double t = t_start + (t_end - t_start) * ratio;

            uint32_t duration = (uint32_t)(t / DRV8825_RMT_PULSE_US - DRV8825_RMT_DURATION_0);

            if(duration < 1)
                duration = 1;

            this->_duration_acc_dec[i] = (uint16_t)duration;

            total_time_us += (duration + DRV8825_RMT_DURATION_0) * DRV8825_RMT_PULSE_US;
        }
    }

    if(n_dec_steps > 0)
    {
        const double t_start = v_max_us * 0.4;
        const double t_end   = v_max_us;

        for(uint32_t i = 0; i < n_dec_steps; i++)
        {
            double ratio = (double)i / (double)(n_dec_steps - 1);
            double t = t_start + (t_end - t_start) * ratio;

            uint32_t duration = (uint32_t)(t / DRV8825_RMT_PULSE_US - DRV8825_RMT_DURATION_0);

            if(duration < 1)
                duration = 1;

            this->_duration_acc_dec[n_acc_steps + i] = (uint16_t)duration;

            total_time_us += (duration + DRV8825_RMT_DURATION_0) * DRV8825_RMT_PULSE_US;
        }
    }


    /// Seconda parte di compensazione del profilo trapezoidale/triangolare

    /// Fattore di compensazione dell'errore
    double fattore_di_correzione = (t_acc+t_dec)*1000000 / total_time_us;
    Serial.printf("TOTAL_TIME_CALCULATED = %f\nSCALE : %f\n", total_time_us, fattore_di_correzione);

    for(uint32_t i = 0; i < total; i++)
    {
        uint32_t raw = (_duration_acc_dec[i] + DRV8825_RMT_DURATION_0);
        raw = (uint32_t)(raw * fattore_di_correzione);

        if(raw < 1)
            raw = 1;

        _duration_acc_dec[i] = (uint16_t)(raw - DRV8825_RMT_DURATION_0);
    }
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
        case DRV_RMT_TX_BUSY :              return "DRV_RMT_TX_BUSY";
        case DRV_ERR_RMT_CREATION :         return "DRV_ERR_RMT_CREATION";
        case DRV_ERR_RMT_ENABLE :           return "DRV_ERR_RMT_ENABLE";
        case DRV_ERR_RMT_COPY_ENCODER :     return "DRV_ERR_RMT_COPY_ENCODER";
        case DRV_ERR_RMT_TX_TIMEOUT :       return "DRV_ERR_RMT_TX_TIMEOUT";
        case DRV_ERR_RMT_TRANSMIT_CMD :     return "DRV_ERR_RMT_TRANSMIT_CMD";
        //case DRV_ERR_ :                   return "DRV_ERR_"; 
        default :                           return "NOT_A_DRV_CODE";
    }
}

//  -- END OF FILE --