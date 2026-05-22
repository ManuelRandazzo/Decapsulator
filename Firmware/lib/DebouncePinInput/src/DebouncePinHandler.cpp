#include "DebouncePinHandler.hpp"

DebPinHandler::DebPinHandler()
    : mutex(nullptr), level(0), pin(255), 
      debounce_ms(30), isInterrupt(INTR), isAttached(false), inputMode(INPUT), flag(0), 
      precPinLevel(0), lastTime(0), debState(0), TRIGGER(CHANGE), changeOccurred(false)
{

}

/**
 * @brief Costruttore del Debounce pin
 * @attention Non serve e non bisogna fare i pinMode dei pin, viene gestito tutto dal costruttore
 * 
 * @param IntrOrPoll    INTR se è interrupt oppure POLL se è polling
 * @param pinNumber     Numero del pin di input
 * @param pinName       Nome del pin usato per debugging
 * @param debounce_ms   Tempo per il debounce in millisecondi
 * @param input_mode    Modalità di input del pin INPUT, INPUT_PULLUP, INPUT_PULLDOWN
 * @param level_trigger Livello a cui viene triggerato il cambio di stato del pin
 */
void DebPinHandler::begin(bool IntrOrPoll, uint8_t pinNumber, const char* pinName,
                          uint32_t debounce_ms, uint8_t level_trigger, uint8_t input_mode)
{
    this->mutex = xSemaphoreCreateMutex();

    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
    {
        Serial.printf("Errore xSemaphoreTake del pin \"%s\"\n", this->name);
        //LogError("initDebPin", "Errore xSemaphoreTake del pin \"%s\"", this->name);
        return;
    }

    /// Inizializza l'input pin con la modalità voluta
    if(pinNumber == 255)
    {
        //LogError("initDebPin", "Errore pin non fornito del pin \"%s\"", this->name);
        xSemaphoreGive(this->mutex);
        return;
    }

    this->isInterrupt = IntrOrPoll;
    this->pin = pinNumber;
    this->name = pinName != "" ? pinName : "No Pin Name";
    this->debounce_ms = debounce_ms;
    this->TRIGGER = level_trigger;
    this->inputMode = input_mode;

    pinMode(this->pin, this->inputMode);

    /// Legge il valore iniziale del pin
    this->level = this->precPinLevel = digitalReadFast(this->pin);

    /// Legge se l'evento all'inizio del programma è attivo
    switch(this->TRIGGER)
    {
        case RISING  : this->changeOccurred = this->level == HIGH; this->levelTriggered = HIGH; break;
        case FALLING : this->changeOccurred = this->level == LOW;  this->levelTriggered = LOW;  break;
        case CHANGE  : this->changeOccurred = true; this->levelTriggered = CHANGE; break;
    }

    if(this->isInterrupt == INTR)
        /// Associa la relativa Interrupt Service Routine se esiste
        attachInterruptArg(digitalPinToInterrupt(this->pin), &this->__ISR, this, this->TRIGGER);

    this->isAttached = true;

    xSemaphoreGive(this->mutex);
}

/**
 * @brief Distruttore della classe
 */
DebPinHandler::~DebPinHandler()
{
    if(this->isInterrupt == INTR)
        detachInterrupt(digitalPinToInterrupt(this->pin));
}

/**
 * @brief Dopo aver fatto il detach permette di ricollegare il pin con i dati impostati
 */
void DebPinHandler::reattach()
{
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return;

    if(this->isAttached)
    {
        xSemaphoreGive(this->mutex);
        return;
    }

    if(this->isInterrupt == INTR)
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

    if(!this->isAttached)
    {
        xSemaphoreGive(this->mutex);
        return;
    }

    if(this->isInterrupt == INTR)
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
    bool flagHasOccured;
    
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return false;

    if(this->isAttached)
        flagHasOccured = this->changeOccurred;

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
    {
        xSemaphoreGive(this->mutex);
        return false;
    }

    if(this->isInterrupt == POLL)
    {
        xSemaphoreGive(this->mutex);
        return false;
    }

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

    if(this->isInterrupt == INTR)
    {
        xSemaphoreGive(this->mutex);
        return false;
    }

    if(!this->isAttached)
    {
        xSemaphoreGive(this->mutex);
        return false;
    }

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
                    xSemaphoreGive(this->mutex);
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
 * @brief Restituisce il valore del pin senza debounce
 */
int8_t DebPinHandler::rawRead()
{
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return -1;

    int8_t pinToRead = this->pin;

    xSemaphoreGive(this->mutex);

    /// legge il pin e restituisce il livello
    return digitalReadFast(pinToRead);
}


/**
 * @brief restituisce se il pin è polling
 * 
 * @return true se è interrupt, false se è polling
 */
bool DebPinHandler::IsInterrupt()
{
    bool isIntr;
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
    {
        Serial.printf("Errore in IsInterrupt xSemaphoreTake del pin \"%s\"\n", this->name);
        //LogError("IsInterrupt", "Errore xSemaphoreTake del pin \"%s\"", this->name);
        return false;
    }
    isIntr = this->isInterrupt == INTR;
    xSemaphoreGive(this->mutex);

    return isIntr;
}

/**
 * @brief restituisce se il pin è interrupt
 * 
 * @return true se è polling, false se è interrupt
 */
bool DebPinHandler::IsPolling()
{
    bool isPoll;
    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
    {
        Serial.printf("Errore in IsPolling xSemaphoreTake del pin \"%s\"\n", this->name);
        //LogError("IsPolling", "Errore xSemaphoreTake del pin \"%s\"", this->name);
        return false;
    }
    isPoll = this->isInterrupt == POLL;
    xSemaphoreGive(this->mutex);

    return isPoll;
}



/**
 * @brief Restituisce i livello in cui il pin viene triggerato
 * 
 * @returns LOW(0), HIGH(1), CHANGE(3), ERRORE MUTEX TAKE(-1)
 */
int8_t DebPinHandler::getLevelTrig()
{
    int8_t levelTrig;

    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return -1;
    levelTrig = this->levelTriggered;
    xSemaphoreGive(this->mutex);

    return levelTrig;
}




/**
 * @brief Restituisce la modalità di trigger del pin
 * 
 * @returns RISING(1), FALLING(2) or CHANGE(3)
 */
int8_t DebPinHandler::getTriggerMode()
{
    int8_t triggerMode;

    if(xSemaphoreTake(this->mutex, 0) == pdFAIL)
        return -1;
    triggerMode = this->TRIGGER;
    xSemaphoreGive(this->mutex);

    return triggerMode;
}




/**
 * 
 *  PRIVATE:
 * 
 */



/**
 *  @brief Interrupt Service Routine invocata
 */
void IRAM_ATTR DebPinHandler::__ISR(void* thisPtr)
{
    DebPinHandler *ptrThis = static_cast<DebPinHandler *>(thisPtr);

    /// Alza il flag
    ptrThis->flag = 1;
}