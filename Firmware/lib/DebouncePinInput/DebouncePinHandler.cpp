#include "DebouncePinHandler.hpp"


/**
 * @brief Costruttore del Debounce pin
 * @attention Non serve e non bisogna fare i pinMode dei pin, viene gestito tutto dal costruttore
 * 
 * @param IntrOrPoll    INTERRUPT oppure POLLING
 * @param pinNumber     Numero del pin di input
 * @param pinName       Nome del pin usato per debugging
 * @param debounce_ms   Tempo per il debounce in millisecondi
 * @param input_mode    Modalità di input del pin INPUT, INPUT_PULLUP, INPUT_PULLDOWN
 * @param level_trigger Livello a cui viene triggerato il cambio di stato del pin
 */
DebPinHandler::DebPinHandler(bool IntrOrPoll, uint8_t pinNumber, const char* pinName,
                             uint32_t debounce_ms, uint8_t level_trigger, uint8_t input_mode)
    : mutex(nullptr), name((pinName != "") ? pinName : "No Pin Name"), pin(pinNumber), 
      debounce_ms(debounce_ms), isInterrupt(false), inputMode(input_mode), flag(0), 
      precPinLevel(0), lastTime(0), debState(0), TRIGGER(level_trigger), changeOccurred(false)
{
    this->__Init(IntrOrPoll);
}

/**
 * @brief Distruttore della classe
 */
DebPinHandler::~DebPinHandler()
{
    if(this->isInterrupt == true)
        detachInterrupt(digitalPinToInterrupt(this->pin));
}

/**
 * @brief Dopo aver fatto il detach permette di ricollegare il pin con i dati impostati
 */
void DebPinHandler::reattach()
{
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return;

    if(this->isInterrupt)
        attachInterruptArg(digitalPinToInterrupt(this->pin), &this->__ISR, this, this->TRIGGER);

    this->isAttached = true;

    xSemaphoreGive(this->mutex);
}

/**
 * @brief Disconnette il pin
 */
void DebPinHandler::detach()
{
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return;

    if(this->isInterrupt)
        detachInterrupt(this->pin);

    this->isAttached = false;

    xSemaphoreGive(this->mutex);
}

/**
 * @return true se il debounce ha dato esito positivo 
 *         false se negativo
 */
bool DebPinHandler::event()
{
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return false;

    bool flagHasOccured
    if(this->isAttached)
         = this->changeOccurred;

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

    if(!this->isAttached)
        return false;

    if(!this->isInterrupt)
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
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return false;

    this->changeOccurred = false;

    if(this->isInterrupt)
        return false;

    if(!this->isAttached)
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
        
        this->isAttached = true;
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