#pragma once

/**
  @file Interrupts.hpp
  @brief Questo file racchiude in una piccola ma efficente e facile da usare classe che permette di 
            gestire con pochi elementi una call da una ISR gestita internamente alla classe.
            Inoltre permette di aiutare molto nelle classi che gestiscono driver o safety.
*/



#include "Tasks.hpp"

/// @link https://learn.microsoft.com/it-it/cpp/standard-library/functional?view=msvc-170
#include <functional>

class INTERRUPTS
{
  public :
    /// Costruttore che crea solo l'oggetto
    INTERRUPTS() {};

    /// Crea L'Interrupt Service Routine per le normali funzioni
    void Init(const char * name, uint8_t pin, uint8_t modePin,  uint8_t IT_TriggerMode, uint32_t HeapSize,
              UBaseType_t priority, void (*functionToExecute)() = nullptr);

    /// Crea L'Interrupt Service Routine per le Classi
    template <class refClass>
    void Init(const char * name, uint8_t pin, uint8_t modePin, uint8_t IT_TriggerMode, uint32_t HeapSize,
              UBaseType_t priority, refClass* thisPointer, void (refClass::*methodToExecute)() = nullptr);

    /// Elimina l'Interrupt Service Routine e la task che lo gestisce
    ~INTERRUPTS();

    /// Restituisce l'Handler della task che gestisce l'Interrupt Service Routine
    /*inline*/ TaskHandle_t getHandlerIT();

    /*inline*/ void changePinIT(uint8_t pin, uint8_t modePin, uint8_t IT_TriggerMode);

    /// Metodo che cambia dinamicamente la funzione gestita nell'ISR
    /*inline*/ void setISR(void (*UserISR)());

    /// Metodo che cambia dinamicamente il metodo gestito nell'ISR
    template <class refClass>
    /*inline*/ void setISR(refClass* thisPointer, void (refClass::*methodToExecute)());

    /// Metodo per attivare / riattivare un Interrupt Service Routine (esegue l'attach del pin e il resume della task)
    /*inline*/ void resumeISR();

    /// Sospende momentaneamente l'Interrupt Service Routine (esegue il detach del pin e il suspend della task)
    /*inline*/ void suspendISR();

  private :
    uint8_t pin;                                        /*!< Pin normale che verrà convertito in pinInterrupt */
    
    uint8_t inputMode;                                  /*!< Modalità di input del pin Interrupt */

    uint8_t triggerIT;                                  /*!< CHANGE, RISING, FALLING, HIGH, LOW modes per il trigger*/
 
    unsigned __isInterruptAttached : 1;                 /*!< Variabile che serve a non fare un attach/detach se il pin è/non è già collegato all'interrupt, evita errori nei log */

    /// Interrupt Task
    TaskTypeDef TaskIT;

    TaskHandle_t TaskHandler = nullptr;                 /*!< Handler per la dichiarazione, creazione e distruzione di una task */

    /// Unica callback sia per metodi che per normali funzioni
    static void IRAM_ATTR __CallbackIT(void* thisPointer);

    /// Copia della funzione dell'Utente
    void (*__LoopIT)() = nullptr;

    /// Copia del metodo dell'Utente
    std::function<void()> __MethodLoopIT = nullptr;

    /// funzione che viene eseguita nel Loop della task 
    void __wrapperTaskLoopIT();
};
