#pragma once
// -*- Gruppo 1-FreeRTOS-C++

/*
 * This file is part of {{ Decapsulator }}.
 *
 * Developed for the ITS Meccatronico Montebelluna-Castelfranco Veneto 2024-2026 Gruppo 1.
 * This product includes software developed by the Gruppo 1 Project.
 * See the COPYRIGHT file at the top-level directory of this distribution
 * for details of code ownership.
 *
 * This program is NOT free software: you can NOT redistribute it and/or modify it.
 *
 * The Documentation is made following the Doxygen standard documentation rules
 *
 */


/**
 *
 * @brief QUESTO FILE CONTIENE LA CLASSE PER L'HANDLING DEI MOTORI STEPPER
 *
 */
#include "DRV8825_Decapsulator.hpp" /// Driver dello stepper
#include "Tasks.hpp"   /// Creazione delle tasks
#include "Interrupts.hpp"
#include "Debug.hpp"
#include "DebouncePinHandler.hpp"

/*-----------------------------------------------
  |                                             |
  |           CLASS USER ENUMERATIONS           |          
  |                                             |
  -----------------------------------------------*/


/// @enum per il numero di microsteps
typedef enum : uint8_t
{
  FULL_STEP    = 1,  // fa uno step intero per ogni segnale sul pin STEP
  STEP_1_TO_2  = 2,  // fa 1/2 passo per ogni segnale sul pin STEP
  STEP_1_TO_4  = 4,  // fa 1/4 di passo per ogni segnale sul pin STEP
  STEP_1_TO_8  = 8,  // fa 1/8 di passo per ogni segnale sul pin STEP
  STEP_1_TO_16 = 16, // fa 1/16 di passo per ogni segnale sul pin STEP
  STEP_1_TO_32 = 32, // fa 1/32 di passo per ogni segnale sul pin STEP
} uSteps_t;



/// @enum per l'OUTPUT del finecorsa
typedef enum : uint8_t
{
  ACTIVE_LOW = LOW,
  ACTIVE_HIGH = HIGH,
  UNKNOWN = 255,
} CalibSignal_t;



/// @enum per la direzione del motore
typedef enum : int8_t
{
  DIR_NEGATIVE = DRV8825_COUNTERCLOCK_WISE,
  DIR_POSITIVE = DRV8825_CLOCK_WISE,
  NO_DIR = 0,
} Direction_t;



/// @enum per sapere se il motore allo STOP rimane in coppia o no
typedef enum : uint8_t
{
  RELEASE = 0,
  HOLD    = 1,
} StopReleaseOrHold_t;



/// @enum per sapere il finecorsa che viene scelto
typedef enum : int8_t
{
  HARD_NONE = -1,
  HARD_MIN = 0,
  HARD_MAX = 1,
} HardLimit_t;



/*-----------------------------------------------
  |                                             |
  |               CLASS DECLARATION             |          
  |                                             |
  -----------------------------------------------*/




/**
 *
 *  @warning ATTENZIONE: I PIN "MODE 0, MODE 1, MODE 2" VENGONO DECISI DALLA CONFIGURAZIONE HARDWARE DIRETTAMENTE NEL PCB PER IL MICROSTEPPING
 *                       TABELLA DI @ref SUL DATASHEET.
 *  @warning ATTENZIONE: IL PIN DECAY FORSE SAREBBE MEGLIO LASCIARLO FLOATTANTE PER AVERE UN MIX DI PRECISIONE E SILENZIOSITA'
 *
 */

class MOTION
{
  public :
    /// Costruttori
    MOTION() : NAME("") {};
    MOTION(const char* motor_name) : NAME(motor_name) {};
    

    /// Distruttore
    ~MOTION();

    /// Inizializza il Motion Controller
    drv_err_t Init(uint16_t numberOfSteps, uint8_t dir_pin, uint8_t step_pin, uint8_t en_pin, uint8_t rst_pin, uint8_t sleep_pin,
                   UBaseType_t taskPriority, uSteps_t microSteps, uint8_t fault_pin = 255, void (*FaultISR)() = nullptr, uint32_t FaultISR_Heap = 4096, UBaseType_t FaultISR_priority = 15);

    /// Setta i limiti massimi e minimi oltre ai quali il motore non può arrivare
    void setHardLimits(uint8_t pinLimMax, uint8_t pinLimMin, bool IntrOrPoll, uint32_t debounce_ms = 30, uint8_t input_mode = INPUT, CalibSignal_t levelActive = UNKNOWN);
    
    /// Rimuove i limiti massimi e minimi oltre ai quali il motore non può arrivare
    void removeHardLimits();

    /// Ricollega i limiti massimi e minimi oltre ai quali il motore non può arrivare
    void reattachHardLimits();

    /// Mette in coppia il motore
    void attach();    

    /// Disaccoppia il motore
    void detach();

    /// Restituisce se il motore è attached (true) o no (false)
    bool isAttached();

    /// Restituisce se il motore è detached (true) o no (false) 
    bool isDetached();

    /// Inizializza il motore con l'Homing in modo che si sappia il punto di partenza
    /// @attention Prima dell'home bisogna chiamate setHardLimits o ritornerà senza fare homing
    void home(double HomeVelocity_gradi_sec, HardLimit_t HardLimitToReach, Direction_t searchDirection, double gradiDopoHome);

    /// @return se è finito(true) o no(false) l'homing
    bool isHomeDone();

    /// @return true se l'ultimo homing è fallito (finecorsa inatteso durante la ricerca)
    bool isHomeFailed();

    /// Permette al motore di avviarsi e muoversi, almeno una volta deve essere chiamata questa funzione
    void Start();

    /// Ferma il motore lasciandolo in coppia o togliendo corrente e necessita di un nuovo Start()
    void Stop(StopReleaseOrHold_t rilasciaOppureMantieniCoppia = RELEASE);

    /// Ferma il motore lasciandolo in coppia non serve richiamare un nuovo Start()
    void Halt();

    /// @return true se il motore è fermo e non può essere comandato
    bool isStopped();

    /// @return true se il motore può essere comandato
    bool isStarted();

    /// @brief abortisce (cancella) il comando attuale e IsStepDone = true
    /// @returns Numero di step rimanenti del comando abortito
    uint64_t abortCurrentCommand();

    void forceStandStill() { this->selettore == STAND_STILL; }

    /// Muove il motore in una direzione e alla velocità specificata in modo RELATIVO
    void moveRel(double gradi, double speed_gradi_al_secondo = 0.0);

    /// Muove il motore in una direzione e alla velocità specificata in modo ASSOLUTO rispetto all'accensione
    void moveAbs(double gradi, double speed_gradi_al_secondo = 0.0);

    /// Muove il motore all'infinito verso la direzione specificata
    void moveContinuous(Direction_t direzione, double speed_gradi_al_secondo = 0.0);

    /// Restituisce se il movimento è finito o no così da poterne iniziare un altro
    bool isStepDone();

    /// Driver in low power mode, Disaccoppia il motore, Ignora TUTTI gli Input, spegne : clock, pompa di carica, regolatore interno 
    /// Se state = FALSE è "sveglio", se state = TRUE allora va in sleep mode, se è già spento o già acceso e viene ripetuta l'operazione non fa nulla
    void sleep(bool state = true);

    ///resetta il driver e le variabili
    void reset();

    /// Converte da gradi a steps con segno
    int64_t gradiToSteps(double gradi);

    /// Converte da steps a gradi con segno
    double stepsToGradi(int64_t steps);

    /// Definizione di una Interrupt Service Routine (ISR) relativa al pin nFAULT del DRV8825 per monitoraggio asincrono.
    void setFaultISR(uint8_t fault_pin, void (*FaultISR)(), uint32_t FaultISR_Heap = 4096, UBaseType_t priority = 15);

    /// @return la posizione assoluta in steps
    double getPosition();

    /// @return la posizione assoluta in steps
    int64_t getPositionInSteps();

    DebPinHandler HardMax; /*!< Oggetto del driver per rilevare il limite massimo */
    
    DebPinHandler HardMin; /*!< Oggetto del driver per rilevare il limite minimo */
  
  private : /// Dato che la libreria del driver fornisce come protected delle variabili la classe MOTION le eredita
    const char* NAME; /*!< Nome del motore utile per il debug */

    String complete_tag; /*!< Variabile per poter scrivere il nome nella tag dei logs */

    /// Metodo per scrivere "MOTOR_NAME + tag nei log
    const char* TAG(const char* tag);

    DRV8825 Motion; /*!< Oggetto del driver usato per il motore */
    
    typedef enum : uint8_t { STAND_STILL, MOVE_REL, MOVE_ABS, CONTINUOUS } SwitchMove_t;
    
    #ifdef LOG_ACTIVE_MOTION
      const char* SwitchMoveStr[5] = { "STAND STILL", "MOVE RELATIVE", "MOVE ABSOLUTE", "MOVE CONTINUOUS" };
    #endif

    int64_t absoluteStepCounter;                /*!< Variabile di quanti step ha fatto il motore dall'accensione  */

    bool __isHomeFinished = false;              /*!< indica se l'homing è finito o no  */

    INTERRUPTS nFAULT_ISR;                      /*!< Oggetto della Classe della gestione degli Interrupts */
    
    void (*__FaultISR)();                       /*!< Funzione che viene chiamata all'interno della ISR dell'oggetto nFAULT_ISR*/

    uSteps_t uStepScelti;                       /*!< Microsteps scelti con la config. HW con pin MODE_0 - MODE_2  */
    
    uint16_t __stepsMotore;                     /*!< Si salva quanti step/giro ha il motore (Es. 200 step/giro)  */

    enum HomingState_t { HOMING_IDLE, HOMING_SEARCH, HOMING_BACKOFF };
    QueueHandle_t MoveQueueHandler;             /*!< E' l'Handler della coda usata per bufferizzare i comandi di movimento  */
    
    typedef struct xQueueMoveDataStruct
    {
      SwitchMove_t __SwitchMove;     /*!< Variabile switch per il movimento del motore nella task  */
      uint64_t __speed_steps_us;     /*!< Velocità step/microsecondo  */
      int64_t __move_steps;          /*!< Passi da eseguire scelti in runtime  */
      Direction_t __dir;             /*!< Direzione che verrà impostata all'invio del comando  */
    } MoveQueue_t;

    const MoveQueue_t defaultReceiverQueue =
    {
      .__SwitchMove = STAND_STILL,
      .__speed_steps_us = 10,
      .__move_steps = 0,
      .__dir = DIR_NEGATIVE,
    };
 
    MoveQueue_t receiverQueue;                  /*!< Struct che contiene gli attuali dati ricevuti  */

    BaseType_t MoveSendToQueue(MoveQueue_t StructToSend);

    static void MoveHandler(void *pvParameters);/*!< Funzione che esegue la task  */

    TaskHandle_t __MoveHandlerTask = NULL;    /*!< Handler della task del motion */

    static void UpdateMoveHandler(void *pvParameters);/*!< Funzione che esegue la task di update */

    TaskHandle_t __UpdateMoveHandlerTask = NULL;    /*!< Handler della task di update del motion  */

    static void HomingHandlerTask(void *pvParameters);

    QueueHandle_t HomingQueueHandler = NULL;
	  HomingState_t __homing_state = HOMING_IDLE;
    struct HomingQueue_t
    {
		  HardLimit_t __hardLimit;
      Direction_t __backDir;                    /*!< Direzione di Backoff dopo l'homing */
		  CalibSignal_t __calib_signal;             /*!< Valore considerato come sensore triggerato */
		  uint64_t __home_steps_us;                 /*!< Velocità dell'homing in step/secondo */
      int64_t __PostHomeVal;                    /*!< è il valore di cui si deve rispostare in avanti in cui vi sarà la posizione 0 dopo l'homing  */
		  Direction_t __search_dir;
    };
    const HomingQueue_t defaultHomingQueue = 
    {
      .__hardLimit = HARD_NONE,
      .__backDir = NO_DIR,               /*!< Direzione di Backoff dopo l'homing */
      .__calib_signal = UNKNOWN,
      .__home_steps_us = 0,
      .__PostHomeVal = 0,
      .__search_dir = NO_DIR,
    };

    TaskHandle_t __HomingHandlerTask = NULL;        /*!< Handler della task di update del motion  */

    /// Funzione per ottenere il delay per poter cambiare la velocità del movimento
    uint64_t getPeriodDelay(const double gradiSecondo);   
    
    SwitchMove_t selettore = STAND_STILL;

    bool __isStopped = true;                  /*!< Flag di motore stoppato o avviato modificato da Start() e Stop() e restituito da
                                                   isStopped e isStarted  */
    
    bool __isHalted = false;                  /*!< Flag di motore in Halt modificato da Halt e tutte le azioni di movimento  */

    bool __isAttached = false;                /*!< Flag di motore stoppato o avviato modificato da Start() e Stop() e restituito da
                                                   isStopped e isStarted  */

    bool __isHomingActive = false;            /*!< true se un homing è in corso: blocca MoveHandler per evitare accessi concorrenti a Motion */

    bool __isHomeFailed = false;              /*!< true se l'ultimo homing è fallito (finecorsa opposto, timeout, halt, configurazione errata) */

    bool __isHomingHaltRequested = false;     /*!< flag dedicato per Halt() consumato da HomingHandlerTask, separato da __isHalted per evitare race con MoveHandler */

    CalibSignal_t __calib_signal = UNKNOWN;
    
    Direction_t __limit_direction = NO_DIR;
};