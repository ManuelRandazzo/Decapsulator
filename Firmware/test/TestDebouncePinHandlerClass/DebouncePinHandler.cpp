#include "DebouncePinHandler.hpp"



DebPinHandler::DebPinHandler(bool IntrOrPoll, uint8_t pinNumber, const char* pinName,
                             uint32_t debounce_ms, uint8_t input_mode, uint8_t level_trigger)
    : mutex(nullptr), name((pinName != "") ? pinName : "No Pin Name"), pin(pinNumber), 
      debounce_ms(debounce_ms), isInterrupt(false), inputMode(input_mode), flag(0), 
      precPinLevel(0), lastTime(0), debState(0), TRIGGER(level_trigger), changeOccurred(false)
{
    this->__Init(IntrOrPoll);
}


DebPinHandler::~DebPinHandler()
{
    if(this->isInterrupt == true)
        detachInterrupt(digitalPinToInterrupt(this->pin));
}


bool DebPinHandler::event()
{
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return false;

    bool flagHasOccured = this->changeOccurred;

    xSemaphoreGive(this->mutex);

    return flagHasOccured;
}


/**
 * @brief  Aggiorna l'istanza e fa il debounce in INTERRUPT
 * @return Se è avvenuto o no un cambio di stato del pin
 * @attention E' solo per INTERRUPT:
 */
bool DebPinHandler::intrUpdate()
{

    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return false;
    
    this->changeOccurred = false;

    if(!this->isInterrupt)
        return this->changeOccurred = false;

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

            if(millis() - this->lastTime >= this->debounce_ms)
            {
                /// Flag reset
                this->flag = 0;

                /// Debounce state reset
                this->debState = 0;

                /// Restituisce lo stato reale del pin
                if(digitalReadFast(this->pin) == this->precPinLevel)
                {
                    this->level = this->precPinLevel;
                    xSemaphoreGive(this->mutex);
                    return this->changeOccurred = true;
                }
            }

        break;
    }

    xSemaphoreGive(this->mutex);

    return this->changeOccurred = false;
}


/**
 * @brief  Aggiorna l'istanza e fa il debounce in POLLING
 * @param  trigger RISING, FALLING, CHANGE 
 * @return Se è avvenuto o no un cambio di stato del pin
 * @attention E' solo per POLLING:
 */
bool DebPinHandler::pollUpdate(uint8_t trigger)
{
    if(this->isInterrupt)
        return false;

    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return false;
    

    switch(this->debState)
    {
        case 0 : /// STATO ATTESA EVENTO
        {
            /// Legge lo stato del pin
            uint8_t tmpRead = digitalReadFast(this->pin);
            bool eventTriggered;

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
                    //LogError("update", "Inserito un trigger non idoneo al pin \"%s (%d)\"", this->name, this->pin);
                    return false;
                break;
            }
            this->precPinLevel = tmpRead;

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
        }
        case 1 : /// STATO ATTESA CONFERMA

            if(millis() - this->lastTime >= this->debounce_ms)
            {
                /// Debounce state reset
                this->debState = 0;

                /// Restituisce lo stato reale del pin
                if(digitalReadFast(this->pin) == this->precPinLevel)
                {
                    this->level = this->precPinLevel;
                    xSemaphoreGive(this->mutex);
                    return this->changeOccurred = true;
                }
            }

        break;
    }

    xSemaphoreGive(this->mutex);

    return this->changeOccurred = false;
}






/**
 * 
 *  PRIVATE:
 * 
 */

/**
 * @brief Inizializza un determinato input pin per avere un debounce
 */
void DebPinHandler::__Init(bool IntrOrPoll)
{
  this->mutex = xSemaphoreCreateMutex();
  
  if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
  {
    LogError("initDebPin", "Errore xSemaphoreTake del pin \"%s\"", this->name);
    return;
  }

  /// Inizializza l'input pin con la modalità voluta
  if(this->pin == 255)
  {
    LogError("initDebPin", "Errore pin non fornito del pin \"%s\"", this->name);
    return;
  }
  
  pinMode(this->pin, this->inputMode);

  /// Legge il valore iniziale del pin 
  this->level = this->precPinLevel = digitalReadFast(this->pin);

  if(IntrOrPoll == INTR)
  {
    this->isInterrupt = true;
    /// Associa la relativa Interrupt Service Routine se esiste
    attachInterruptArg(digitalPinToInterrupt(this->pin), &this->__ISR, this, this->TRIGGER);

  }

  xSemaphoreGive(this->mutex);
}



/**
 *  @brief Interrupt Service Routine invocata
 */
void IRAM_ATTR DebPinHandler::__ISR(void* thisPtr)
{
  DebPinHandler *ptrThis = static_cast<DebPinHandler *>(thisPtr);

  /// Alza il flag
  ptrThis->flag = 1;
}