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
 * The Documentation is made following the Doxygen standard documentation style
 *
 */

#include "DecapsulatorPRG.hpp"





#pragma region (GLOBAL VARIABLE LIST)

typedef enum __sequence__ : uint8_t
{
    EMERGENCY_STATE,
    CONTAINER_FULL,
    TIMEOUT_STATE,
    MACHINE_STARTUP_STATE,
    PUNZONE_STARTUP_STATE,
    TAMBURO_STARTUP_STATE,
    QUIETE_STATE,
    SERVO_LOADER_OPEN_STATE,
    SERVO_LOADER_CLOSE_STATE,
    REACH_NEXT_STATION_STATE,
    PUNCHER_DOWN_FAST_STATE,
    PUNCHER_DOWN_SLOW_STATE,
    PUNCHER_UP_FAST_STATE,
} Sequence_t;



/// Mutex e spinlock
SemaphoreHandle_t _DecapsulatorMutex = nullptr;
portMUX_TYPE _DecapsulatorSpinlock = portMUX_INITIALIZER_UNLOCKED;

volatile QueueHandle_t ptrAutokillSharedVars;

/// @brief false = da il comando, true = esegue il comando
bool cmd_exec = false; 

#pragma endregion (GLOBAL VARIABLE LIST)



#pragma region (FUNCTION_PROTOTIPES)

const char* state_name_to_string(Sequence_t seq_switch);

#pragma endregion (FUNCTION_PROTOTIPES)



#pragma region (DECAPSULATOR MAIN PROGRAM)


/**
 *  @brief PROGRAMMA PRINCIPALE
 */
void prgDecapsulatorTask(void *pvParameters)
{
    /**
     *    @setup:
     */
    TickType_t getLastTick = xTaskGetTickCount();

    ///ATTENZIONE: Programma con variabili a caso ancora da definire
    ///            e da rendere THREAD SAFE tramite notifiche e/o code
    bool FORCE_THE_STARTUP = false;//true;

    CommandQueueHMI_t FromHMI = defaultCommandQueueHMI;

    EventQueueHMI_t ToHMI = defaultEventQueueHMI;
    bool sendChangesToHMI = false;

    uint8_t cntContainerFull = 0;
    int nCicliRimanenti = 0; // prende il numero dalla SD Card

    Sequence_t sequenza = MACHINE_STARTUP_STATE;  // Gestione della sequenza del movimento del Decapsulator

    bool doAnotherCycle = false;    
    R_TRIG RallaStepDone;
    R_TRIG PunzoneStepDone;

    /// TIMEOUTS: TIMERS:
    uint32_t tmoPunzHome = 0;
    uint32_t tmoRallaHome = 0;
    uint32_t tmoCaduta = 0;

    #ifdef LOG_ACTIVE_MAIN_PRG
        LogInfo("setup main prg", "creata la tasks - Free Stack Space: %d\nSwitch Initial : %s", uxTaskGetStackHighWaterMark(NULL), state_name_to_string(sequenza));
    #endif
  
    /**
     *    @loop:
     */
    while(1)
    {
        /**
         * @info: Gestione della sequenza completa del decapsulator
         * 
         * @attention
         * 
         * @todo Farlo Funzionare :)
         */

        /// acquisisce il tempo attuale di millis()
        const uint32_t MILLIS = millis();

        /// Update dei pin d'evento
        autoKill.intrUpdate();
        cadutaCaps.intrUpdate();
        presenzaCaps.intrUpdate();

        RallaStepDone.CLK(MotRalla.isStepDone());
        PunzoneStepDone.CLK(MotPunzone.isStepDone());

        /// Se arriva una capsula allora bisogna può essere eseguito un altro ciclo
        if(presenzaCaps.event())
            doAnotherCycle = true;

        checkUpdateHMI(&FromHMI);

        Serial.printf("Sequenza : %s\npresenzaCaps.event() : %d\ncadutaCaps.event() : %d", state_name_to_string(sequenza), presenzaCaps.event(), cadutaCaps.event());

        switch(sequenza)
        {
            case EMERGENCY_STATE :
            {        
                /// @todo
                MainPrgStopAllMotors();
                Ventola.on();
                LogError("EMERGENCY", "Si è entrati in uno stato di EMERGENZA");
              
            }
            break;

            case TIMEOUT_STATE :
            {
                if(!cmd_exec)
                {
                    if(ToHMI.xErrorInitPunz || ToHMI.xErrorInitRalla)
                        MainPrgStopAllMotors();

                    Ventola.off();

                    cmd_exec = true;
                }
                /// @todo
            }
            break;

            case CONTAINER_FULL :
            {
                Ventola.off();
                if(FromHMI.restartAfterContainerEmptied == true)
                {
                    FromHMI.restartAfterContainerEmptied = false;
                    cntContainerFull = 0;
                }
            }
            break;

            /// Inizializza il macchinario
            case MACHINE_STARTUP_STATE :
            {
                /// Fa tutti gli attach dei motori e li prepara ad essere comandati
                ServoParatia.attach(SERVO_PIN);
                ServoParatia.write(SERVO_CLOSED_POS); // Chiude la paratia mossa dal servomotore

                cmd_exec = false;

                if(FORCE_THE_STARTUP == false)
                    sequenza = PUNZONE_STARTUP_STATE;
                else
                    sequenza = QUIETE_STATE;
            }
            break;

            case PUNZONE_STARTUP_STATE :
            {
                if(!cmd_exec) // Da il comando
                {
                    Ventola.on(); // Si assicura che la ventola sia accesa
                    MotPunzone.attach();
                    MotPunzone.Start();
                    MotPunzone.home(PUNZ_HOME_SPEED, PUNZ_HOME_ACC, PUNZ_HOME_DEC, PUNZ_HOME_DIR, PUNZ_POST_HOME_POS);
                    tmoPunzHome = MILLIS;
                    cmd_exec = true;
                }
                else
                {
                    if(1/*MILLIS - tmoPunzHome < TIMEOUT_PUNZ_HOME_MS*/)
                    {
                        if(MotPunzone.isHomeDone() == true) // Aspetta la fine del comando
                        {
                            /// Deve assicurarsi di portare in posizione il punzone prima di poter muovere la ralla
                            sequenza = TAMBURO_STARTUP_STATE;
                            cmd_exec = false;
                        }
                    }
                    else
                    {
                        cmd_exec = false;
                        sequenza = TIMEOUT_STATE;
                    }
                }
            }
            break;

            case TAMBURO_STARTUP_STATE :
            {
                if(!cmd_exec) // Da il comando
                {
                    Ventola.on(); // Si assicura che la ventola sia accesa
                    MotRalla.attach();
                    MotRalla.Start();
                    MotRalla.reattachHardLimits();
                    MotRalla.home(RALLA_HOME_SPEED, RALLA_HOME_ACC, RALLA_HOME_DEC, RALLA_HOME_DIR, RALLA_POST_HOME_POS);
                    tmoRallaHome = MILLIS;
                    cmd_exec = true;
                }
                else
                {
                    if(1/*MILLIS - tmoRallaHome < TIMEOUT_RALLA_HOME_MS*/)
                    {
                        if(MotRalla.isHomeDone() == true) // Aspetta la fine del comando
                        {
                            /// Rimuove il sensore di calibrazione
                            MotRalla.removeHardLimits();
                            Ventola.off(); // Si assicura che la ventola sia spenta
                            sequenza = QUIETE_STATE;
                            cmd_exec = false;
                        }
                    }
                    else
                    {
                        sequenza = TIMEOUT_STATE;
                        cmd_exec = false;
                    }
                }
            }
            break;

            case QUIETE_STATE :
            {
                /// Se sono presenti le capsule nello scivolo e c'è stato il segnale di start inizia il ciclo
                if(FromHMI.StartMachine)
                {
                    FromHMI.StartMachine = false;
                  
                    if(doAnotherCycle == true)
                    {
                        Ventola.on(); // Si assicura che la ventola sia accesa
                        sequenza = SERVO_LOADER_OPEN_STATE;
                    }
                }
            }
            break;

            case SERVO_LOADER_OPEN_STATE :
            {
                if(doAnotherCycle == true)
                {
                    if(cntContainerFull <= MAX_CAPSULE_CONTAINER) // Impedisce che scendano le capsule quando il contatore segnala  
                    {
                        if(!cmd_exec) // Da il comando
                        {
                            ServoParatia.write(SERVO_OPEN_POS);
                            tmoCaduta = MILLIS;
                            cmd_exec = true;
                        }
                        else
                        {
                            if(1/*MILLIS - tmoCaduta <= TIMEOUT_CADUTA_CAPS_MS*/)
                            {
                                /// Cambio di stato dovuto dall'Interrupt della Fotocellula conferma capsula nel tamburo
                                if(cadutaCaps.event() == true)
                                {
                                    //cntContainerFull++;
                                    doAnotherCycle = false;
                                    sequenza = SERVO_LOADER_CLOSE_STATE;
                                    cmd_exec = false;
                                }
                            }
                            else
                            {
                                ToHMI.xErrorCapsIncastrata = true;
                                sequenza = TIMEOUT_STATE;
                                cmd_exec = false;
                            }
                        }
                    }
                    else
                    {
                        cmd_exec = false;
                        sequenza = CONTAINER_FULL;
                    }
                }
                else
                    sequenza = QUIETE_STATE;
            }
            break;

            case SERVO_LOADER_CLOSE_STATE :
            {
                /// Se il pezzo è passato, i tot ms di debounce sono passati e non si è intasato
                ServoParatia.write(SERVO_CLOSED_POS);
                sequenza = REACH_NEXT_STATION_STATE;
            }
            break;

            case REACH_NEXT_STATION_STATE :
            {
                if(!cmd_exec) // Dà il comando
                {
                    /// Setta la direzione di marcia del tamburo e si muove alla posizione successiva
                    MotRalla.moveRel(+90.0 * GEAR_RATIO_RALLA, RALLA_SPEED, RALLA_ACC, RALLA_DEC);
                    cmd_exec = true;
                }
                else if(RallaStepDone.Q() == true) // Aspetta la fine del comando
                {
                    sequenza = PUNCHER_DOWN_FAST_STATE;
                    cmd_exec = false;
                }
            }
            break;

            case PUNCHER_DOWN_FAST_STATE :
            {
                if(!cmd_exec) // Dà il comando
                {
                    /// Setta la direzione di marcia del punzone e mette in coda 
                    /// due movimenti uno veloce (bassa coppia) e uno lento (alta coppia)
                    MotPunzone.moveRel(PUNZ_FAST_ROTATIONS * -360.0, PUNZ_FAST_SPEED, PUNZ_ACC, PUNZ_DEC); // Fa 10 giri = 20mm lineari ad alta velocità
                    cmd_exec = true;
                }
                else if(PunzoneStepDone.Q() == true) // Aspetta la fine del comando
                {
                    sequenza = PUNCHER_DOWN_SLOW_STATE;
                    cmd_exec = false;
                }
            }
            break;

            case PUNCHER_DOWN_SLOW_STATE :
            {
                if(!cmd_exec) // Dà il comando
                {
                    /// Fa 15 giri = 30mm lineari ad alta coppia
                    MotPunzone.moveRel(PUNZ_SLOW_ROTATIONS * -360.0, PUNZ_SLOW_SPEED, PUNZ_ACC, PUNZ_DEC);    
                    cmd_exec = true;
                }
                else if(PunzoneStepDone.Q() == true) // Aspetta la fine del comando
                {
                    sequenza = PUNCHER_UP_FAST_STATE;     
                    cmd_exec = false;
                }
            }
            break;

            case PUNCHER_UP_FAST_STATE :
            {
                if(!cmd_exec) // Dà il comando
                {
                    /// Torna nella posizione 
                    MotPunzone.moveRel(PUNZ_ROTATIONS_TOT * +360.0, PUNZ_FAST_SPEED, PUNZ_ACC, PUNZ_DEC);
                    vTaskDelay(100);
                    cmd_exec = true;
                }
                else if(PunzoneStepDone.Q() == true) // Aspetta la fine del comando
                {
                    sequenza = SERVO_LOADER_OPEN_STATE; // Ricomincia il ciclo
                    cmd_exec = false;
                }
            }
            break;
        }

        /// Aggiorna in caso vengano richiesti dei cambiamenti da segnalare all'HMI
        sendUpdateHMI(&ToHMI, &sendChangesToHMI);

        //LogDebug("debug", "TaskTime : %d\n", millis() - MILLIS);

        xTaskDelayUntil(&getLastTick, MainPrg_delay);
    }

    /// Elimina la task qualora uscisse dal while(1)
    vTaskDelete(NULL);
}


#pragma endregion (DECAPSULATOR MAIN PROGRAM)






/**
 * @brief Restituisce una stringa dello stato corrente (SOLO SE LOG ATTIVI)
 */
const char* state_name_to_string(Sequence_t seq_switch)
{
    #ifdef LOG_ACTIVE_MAIN_PRG
        switch(seq_switch)
        {
            case EMERGENCY_STATE          : return "EMERGENCY_STATE";
            case CONTAINER_FULL           : return "CONTAINER_FULL";                
            case TIMEOUT_STATE            : return "TIMEOUT_STATE";           
            case MACHINE_STARTUP_STATE    : return "MACHINE_STARTUP_STATE";       
            case PUNZONE_STARTUP_STATE    : return "PUNZONE_STARTUP_STATE";        
            case TAMBURO_STARTUP_STATE    : return "TAMBURO_STARTUP_STATE";      
            case QUIETE_STATE             : return "QUIETE_STATE";            
            case SERVO_LOADER_OPEN_STATE  : return "SERVO_LOADER_OPEN_STATE";  
            case SERVO_LOADER_CLOSE_STATE : return "SERVO_LOADER_CLOSE_STATE"; 
            case REACH_NEXT_STATION_STATE : return "REACH_NEXT_STATION_STATE"; 
            case PUNCHER_DOWN_FAST_STATE  : return "PUNCHER_DOWN_FAST_STATE";   
            case PUNCHER_DOWN_SLOW_STATE  : return "PUNCHER_DOWN_SLOW_STATE";   
            case PUNCHER_UP_FAST_STATE    : return "PUNCHER_UP_FAST_STATE";
            default                       : return "INVALID MAIN PRG SEQUENCE STATE";       
        }
    #endif
}