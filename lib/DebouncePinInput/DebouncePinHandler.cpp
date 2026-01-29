#include "DebouncePinHandler.hpp"



DebPinHandler::DebPinHandler(uint8_t pinNumber, const char* pinName, uint8_t input_mode, 
                             void (*ISR)(), uint32_t debounce_ms)
    : mutex(nullptr), name(pinName), pin(pinNumber), debounce_ms(debounce_ms), ISR(ISR), 
        inputMode(input_mode), flag(0), precPinLevel(0), lastTime(0), debState(0), TRIGGER(CHANGE)
{
    this->__Init();
}


DebPinHandler::DebPinHandler(uint8_t pinNumber, const char* pinName, uint8_t input_mode, 
                             uint8_t level_trigger, void (*ISR)(), uint32_t debounce_ms)
    : mutex(nullptr), name(pinName), pin(pinNumber), debounce_ms(debounce_ms), ISR(ISR), 
        inputMode(input_mode), flag(0), precPinLevel(0), lastTime(0), debState(0), TRIGGER(level_trigger)
{
    this->__Init();
}


DebPinHandler::DebPinHandler(uint8_t pinNumber, const char* pinName, uint8_t input_mode,
                             uint8_t level_trigger, uint32_t debounce_ms)
    : mutex(nullptr), name(pinName), pin(pinNumber), debounce_ms(debounce_ms), ISR(nullptr), 
        inputMode(input_mode), flag(0), precPinLevel(0), lastTime(0), debState(0), TRIGGER(level_trigger)
{
    this->__Init();
}

DebPinHandler::~DebPinHandler()
{
    detachInterrupt(digitalPinToInterrupt(this->pin));
}


/**
 * @brief  Aggiorna l'istanza e fa il debounce in INTERRUPT
 * @return Se è avvenuto o no un cambio di stato del pin
 * @attention E' solo per INTERRUPT:
 */
bool DebPinHandler::update()
{
    if(this->ISR == nullptr)
        return false;

    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return false;

    switch(this->debState)
    {
        case 0 : /// STATO ATTESA EVENTO
        
            /// Attende che si verifichi l'evento
            if(this->flag)
            {
                /// Si salva il tempo di start da quando avviene il rilevamento dell'interrupt
                this->lastTime = millis();    

                /// Restituisce lo stato di confronto
                this->precPinLevel = digitalReadFast(this->pin);

                /// Passa allo stato di attesa della conferma
                this->debState++;
            }

        break;

        case 1 : /// STATO ATTESA CONFERMA

            if(millis() - this->lastTime >= debounce_ms)
            {
                /// Flag reset
                this->flag = 0;

                /// Debounce state reset
                this->debState = 0;

                /// Restituisce lo stato reale del pin
                if(digitalReadFast(this->pin) == precPinLevel)
                {
                    this->level = this->precPinLevel;
                    xSemaphoreGive(mutex);
                    return true;
                }
            }

        break;
    }

    xSemaphoreGive(mutex);
    
    return false;
}


/**
 * @brief  Aggiorna l'istanza e fa il debounce in POLLING
 * @param  trigger RISING, FALLING, CHANGE 
 * @return Se è avvenuto o no un cambio di stato del pin
 * @attention E' solo per POLLING:
 */
bool DebPinHandler::update(uint8_t trigger)
{
    if(this->ISR != nullptr)
        return false;

    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return false;

    
    switch(this->debState)
    {
        case 0 : /// STATO ATTESA EVENTO

            /// Legge lo stato del pin
            bool tmpRead = digitalReadFast(this->pin);
            bool eventTriggered = false;

            switch(trigger)
            {
                case RISING :
                    eventTriggered = (tmpRead == HIGH && this->precPinLevel == LOW);
                break;

                case FALLING :
                    eventTriggered = (tmpRead == LOW && this->precPinLevel == HIGH);
                break;

                case CHANGE :
                    eventTriggered = (tmpRead != this->precPinLevel);
                break;

                default :
                    LogError("update", "Inserito un trigger non idoneo al pin \"%s (%d)\"", this->name, this->pin);
                break;
            }

            /// Attende che si verifichi l'evento
            if(eventTriggered)
            {
                /// Si salva il tempo di start da quando avviene il rilevamento dell'interrupt
                this->lastTime = millis();    

                /// Restituisce lo stato di confronto
                this->precPinLevel = tmpRead;

                /// Passa allo stato di attesa della conferma
                this->debState++;
            }

        break;

        case 1 : /// STATO ATTESA CONFERMA

            if(millis() - this->lastTime >= debounce_ms)
            {
                /// Debounce state reset
                this->debState = 0;

                /// Restituisce lo stato reale del pin
                if(digitalReadFast(this->pin) == this->precPinLevel)
                {
                    this->level = this->precPinLevel;
                    xSemaphoreGive(mutex);
                    return true;
                }
            }

        break;
    }

    xSemaphoreGive(mutex);

    return false;
}






/**
 * 
 *  PRIVATE:
 * 
 */

/**
 * @brief Inizializza un determinato input pin per avere un debounce
 */
void DebPinHandler::__Init()
{
  mutex = xSemaphoreCreateMutex();
  
  if(xSemaphoreTake(mutex, 0) == pdFAIL)
  {
    LogError("initDebPin", "Errore xSemaphoreTake del pin \"%s\"", name);
    return;
  }

  /// Inizializza l'input pin con la modalità voluta
  if(pin == 255)
  {
    LogError("initDebPin", "Errore pin non fornito del pin \"%s\"", name);
    return;
  }
  
  pinMode(pin, inputMode);

  /// Legge il valore iniziale del pin 
  level = precPinLevel = digitalReadFast(this->pin);

  /// Associa la relativa Interrupt Service Routine
  if(ISR != nullptr)
    attachInterrupt(digitalPinToInterrupt(pin), ISR, TRIGGER);
  
  xSemaphoreGive(mutex);
}



/**
 *  @brief Interrupt Service Routine invocata
 */
void IRAM_ATTR DebPinHandler::__InternalISR(void *pvParameters)
{
  DebPinHandler *ptrThis = static_cast<DebPinHandler *>(pvParameters);

  /// Alza il flag
  ptrThis->flag = 1;
}