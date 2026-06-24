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
#include "HMI_UI_EEZ/vars.h"
#include "filePathsSD.hpp"
#include "DateAndTimePRG.hpp"
#include "AutoKillPRG.hpp"




#pragma region (GLOBAL VARIABLE LIST)

Sequence_t sequenza = MACHINE_STARTUP_STATE;  // Gestione della sequenza del movimento del Decapsulator

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
    
    bool FORCE_THE_STARTUP = false;

    bool skipOneLoadingPhase = false;  
    R_TRIG RallaStepDone;
    R_TRIG PunzoneStepDone;

    uint16_t local_caps_ses = 0;
    uint16_t local_caps_tot = 0; // = getCapsTotFromSD; /// Prende il dato dalla SD
    uint8_t cntContainerCapsuleFull = 0;
    uint8_t cntContainerCoffeeFull = 0;
    int nCicliRimanenti = SD_Card.getValueByKey<int>(DECAPSULATOR_GLOBAL_STATUS_PATH_SD, "nCicliRimanenti"); // prende il numero dalla SD Card
        

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
        cadutaCaps.intrUpdate();
        presenzaCaps.pollUpdate();
                
        RallaStepDone.CLK(MotRalla.isStepDone());
        PunzoneStepDone.CLK(MotPunzone.isStepDone());


        /// Se viene premuto stop il macchinario si arresta
        switch(sequenza)
        {
            /// La macchina non si blocca se si stoppa il macchinario dopo aver già aperto
            /// il servo paratia perchè deve richiudersi e muovere il tamburo alla
            /// prossima posizione oppure il macchinario non partirà (se il servo non viene chiuso)
            /// oppure si incastrerà la capsula (se il tamburo non viene spostata alla posizione successiva)
            case PUNCHER_FIRST_DOWN_STATE ... PUNCHER_SECOND_UP_STATE :
                if(get_var_comando_macchina() == false)
                {
                    ServoParatia.write(SERVO_CLOSED_POS);
                    sequenza = QUIETE_STATE;
                }
            break;
        }

        #ifdef LOG_ACTIVE_MAIN_PRG
            static Sequence_t last_seq;
            if(last_seq != sequenza)
            {
                last_seq = sequenza;
                LogDebug("MAIN PRG LOOP", "Sequenza : %s\npresenzaCaps.event() : %d\ncadutaCaps.event() : %d", state_name_to_string(sequenza), presenzaCaps.event(), cadutaCaps.event());
            }
        #endif
        
        switch(sequenza)
        {
            case EMERGENCY_STATE :
            {
                /// @todo
                set_var_comando_macchina(false);
                set_var_nome_errore("EMERGENZA : Il macchinario necessità di restart, consultare manuale di uso e manutenzione prima di ogni azione");
                MainPrgStopAllMotors();
                Ventola.on();
                ServoParatia.write(SERVO_CLOSED_POS);
                LogError("EMERGENCY", "Si è entrati in uno stato di EMERGENZA");
            }
            break;

            case TIMEOUT_STATE :
            {
                if(!cmd_exec)
                {
                    /// Disattiva il pulsante start
                    set_var_comando_macchina(false);

                    MainPrgStopAllMotors();                    

                    Ventola.off();

                    cmd_exec = true;
                }
                /// @todo
            }
            break;

            case CONTAINER_FULL :
            {
                if(!cmd_exec)
                {
                    set_var_comando_macchina(false);

                    if(cntContainerCapsuleFull >= MAX_CAPSULE_CONTAINER && cntContainerCoffeeFull >= MAX_COFFEE_CONTAINER)
                    {
                        set_var_nome_errore("Container Capsule e Caffè pieni, svuotarli e premere OK");
                        LogDebug("CONTAINERS", "Container Capsule e Caffè pieni, svuotarli e premere OK");
                    }
                    else
                    {
                        if(cntContainerCoffeeFull >= MAX_CAPSULE_CONTAINER)
                        {
                            set_var_nome_errore("Container Capsule pieno, svuotarlo e premere OK");
                            LogDebug("CONTAINERS", "Container Capsule pieno");
                        }
                        else if(cntContainerCoffeeFull >= MAX_COFFEE_CONTAINER)
                        {
                            set_var_nome_errore("Container Caffè pieno, svuotarlo e premere OK");
                            LogDebug("CONTAINERS", "Container Caffè pieno");
                        }
                    }
                    Ventola.setDuty(50);
                    cmd_exec = true;
                }
                else
                {
                    /// Se rileva che è stato premuto OK vuol dire che sono stati svuotati i contenitori (il pulsante ha logica negata)
                    if(!get_var_pulsante_errore())
                    {
                        sequenza = QUIETE_STATE;
                        cmd_exec = false;
                    }
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
                {
                    /// Avvio macchina completato (HMI)
                    set_var_stato_avvio_macchina(true);
                    sequenza = QUIETE_STATE;
                }
            }
            break;

            case PUNZONE_STARTUP_STATE :
            {
                if(!cmd_exec) // Da il comando
                {
                    Ventola.on(); // Si assicura che la ventola sia accesa
                    MotPunzone.attach();
                    MotPunzone.Start();
                    MotPunzone.home(PUNZ_HOME_SPEED, PUNZ_HOME_ACC, PUNZ_HOME_DEC, HARD_MAX, PUNZ_HOME_DIR, PUNZ_POST_HOME_POS);
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
                            MotPunzone.detach(); // toglie la coppia al punzone
                            sequenza = TAMBURO_STARTUP_STATE;
                            Ventola.on(); // Si assicura che la ventola sia accesa
                            MotRalla.attach();
                            MotRalla.Start();
                            MotRalla.reattachHardLimits();
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
                    int8_t RallaHardMaxStatus = MotRalla.HardMax.rawRead();
                    
                    if(RallaHardMaxStatus != -1)
                    {
                        if(RallaHardMaxStatus == RALLA_CAM_SIGNAL)
                        {
                            if(MotRalla.isStepDone())
                            {
                                LogDebug("TAMBURO_STARTUP_STATE", "move Continuous indietro");
                                MotRalla.moveRel(-RALLA_HOME_DIR * 360.0 * GEAR_RATIO_RALLA, RALLA_HOME_CONT_SPEED);
                            }
                        }
                        else
                        {
                            double gradi_post_home = 0.0;
                            if(!MotRalla.isStepDone())
                            {
                                MotRalla.abortCurrentCommand();
                                gradi_post_home = 0.0 * GEAR_RATIO_RALLA;
                            }
                            else
                                gradi_post_home = RALLA_POST_HOME_POS * GEAR_RATIO_RALLA;

                            LogDebug("TAMBURO_STARTUP_STATE", "MotRalla.home -> gradi_post_home : %f", gradi_post_home);
                            
                            MotRalla.home(RALLA_HOME_SPEED, RALLA_HOME_ACC, RALLA_HOME_DEC, HARD_MAX, RALLA_HOME_DIR, gradi_post_home);

                            if(nCicliRimanenti != 0)
                                skipOneLoadingPhase = true;

                            tmoRallaHome = MILLIS;
                            cmd_exec = true;
                        }
                    }
                }
                else
                {
                    if(1/*MILLIS - tmoRallaHome < TIMEOUT_RALLA_HOME_MS*/)
                    {
                        if(MotRalla.isHomeDone() == true) // Aspetta la fine del comando
                        {
                            /// Rimuove il sensore di calibrazione
                            MotRalla.removeHardLimits();
                            vTaskDelay(500);
                            
                            sequenza = QUIETE_STATE;

                            /// Avvio macchina completato (HMI)
                            set_var_stato_avvio_macchina(true);
                            xTaskNotify(DateAndTimeHandler, DATE_TIME_FORCE_UPDATE, eSetBits);
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
                if(get_var_comando_macchina())
                {
                    int8_t presenzaEvt = presenzaCaps.rawEvent();
                    if(presenzaEvt != -1)
                    {
                        /// Se arriva una capsula allora può essere eseguito un altro ciclo
                        if(presenzaEvt == 1)
                        {
                            Ventola.on(); // Si assicura che la ventola sia accesa
                            MotRalla.attach();   // mette in coppia il tamburo
                            MotPunzone.attach(); // mette in coppia il punzone

                            /// Dopo un errore critico, un homing o uno stop skippa
                            /// una volta l'apertura e la rotazione del tamburo
                            /// poichè potrebbe esserci una capsula ancora non punzonata
                            /// che potrebbe quindi venire raschiata danneggiando il raschiatore
                            if(!skipOneLoadingPhase)
                            {
                                ServoParatia.attach(SERVO_PIN); // Riattiva il servo paratia
                                sequenza = SERVO_LOADER_OPEN_STATE;
                            }
                            else
                            {
                                skipOneLoadingPhase = false;
                                sequenza = PUNCHER_FIRST_DOWN_STATE;
                            }
                        }
                        else
                        {
                            set_var_nome_errore("Nessuna capsula inserita");
                            
                            /// Disattiva il pulsante start
                            set_var_comando_macchina(false);
                        }
                    }
                }
            }
            break;

            case SERVO_LOADER_OPEN_STATE :
            {
                int8_t presenzaEvt = presenzaCaps.rawEvent();
                if(presenzaEvt != -1)
                {
                    /// Se arriva una capsula allora può essere eseguito un altro ciclo
                    if(presenzaEvt == 1)
                    {
                        /// Impedisce che scendano le capsule quando il contatore segnala che il serbatoio è pieno
                        if(cntContainerCapsuleFull < MAX_CAPSULE_CONTAINER && cntContainerCoffeeFull < MAX_COFFEE_CONTAINER)  
                        {
                            ServoParatia.write(SERVO_OPEN_POS);
                            tmoCaduta = MILLIS;
                            sequenza = SERVO_LOADER_CLOSE_STATE;
                        }
                        else
                        {                        
                            cmd_exec = false;
                            sequenza = CONTAINER_FULL;
                        }
                    }
                    else
                    {
                        if(nCicliRimanenti > 0)
                        {
                            nCicliRimanenti--;
                            LogDebug("nCicliRimanenti", "Continuo a fare il resto del ciclo");
                            cmd_exec = false;
                            sequenza = REACH_NEXT_STATION_STATE;
                        }
                        else
                        {
                            LogDebug("Cicli Finiti", "Fermo il macchinario");
                            
                            /// Toglie la coppia ai motori così che non scaldino
                            // MotRalla.detach(); Alla RALLA no per poter garantire la posizione
                            MotPunzone.detach();

                            /// Non spegne completamente la ventola ma rimane bassa per raffreddare i drivers
                            Ventola.setDuty(50);

                            /// Riporta il pulsante in off
                            set_var_comando_macchina(false); 

                            sequenza = QUIETE_STATE;
                        }
                    }
                }
            }
            break;

            case SERVO_LOADER_CLOSE_STATE :
            {
                if(1/*MILLIS - tmoCaduta <= TIMEOUT_CADUTA_CAPS_MS*/)
                {
                    /// Cambio di stato dovuto dall'Interrupt della Fotocellula conferma capsula nel tamburo
                    if(cadutaCaps.event() == true)
                    {
                        nCicliRimanenti = 3;
                        /// Se il pezzo è passato, i tot ms di debounce sono passati e non si è intasato
                        ServoParatia.write(SERVO_CLOSED_POS);
                        sequenza = REACH_NEXT_STATION_STATE;
                        vTaskDelay(200 + (uint32_t)(TEMPO_CADUTA_CAPSULA_MS));
                        cmd_exec = false;
                    }
                }
                else
                {
                    set_var_nome_errore("Capsula incastrata nello scivolo, spegnere il macchinario e estrarla.Leggere manuale di istruzioni prima di ogni operazione");
                    
                    sequenza = TIMEOUT_STATE;
                    cmd_exec = false;
                }
            }
            break;

            case REACH_NEXT_STATION_STATE :
            {
                if(!cmd_exec) // Dà il comando
                {
                    /// Setta la direzione di marcia del tamburo e si muove alla posizione successiva
                    MotRalla.moveRel(+90.0 * GEAR_RATIO_RALLA, RALLA_SPEED);
                    LogDebug("moveRel", "moveRel");
                    cmd_exec = true;
                }
                else if(RallaStepDone.Q() == true) // Aspetta la fine del comando
                {
                    sequenza = PUNCHER_FIRST_DOWN_STATE;
                    vTaskDelay(500);
                    cmd_exec = false;
                }
            }
            break;

            case PUNCHER_FIRST_DOWN_STATE :
            {
                if(!cmd_exec) // Dà il comando
                {
                    /// Setta la direzione di marcia del punzone e mette in coda
                    MotPunzone.moveRel(PUNZ_MOVE_ROTATIONS * -360.0, PUNZ_SPEED);
                    vTaskDelay(500);
                    cmd_exec = true;
                }
                else if(PunzoneStepDone.Q() == true) // Aspetta la fine del comando
                {
                    sequenza = PUNCHER_FIRST_UP_STATE;
                    cmd_exec = false;
                }
            }
            break;

            case PUNCHER_FIRST_UP_STATE :
            {                
                if(!cmd_exec) // Dà il comando
                {
                    MotPunzone.moveRel(PUNZ_MOVE_ROTATIONS * +360.0, PUNZ_SPEED);   
                    cmd_exec = true;
                    vTaskDelay(500); 
                }
                else if(PunzoneStepDone.Q() == true) // Aspetta la fine del comando
                {
                    sequenza = PUNCHER_SECOND_DOWN_STATE;
                    cmd_exec = false;
                }
            }
            break;

            case PUNCHER_SECOND_DOWN_STATE :
            {
                if(!cmd_exec) // Dà il comando
                {
                    /// Setta la direzione di marcia del punzone e mette in coda
                    MotPunzone.moveRel(PUNZ_MOVE_ROTATIONS * -360.0, PUNZ_SPEED);
                    cmd_exec = true;
                    vTaskDelay(500);
                }
                else if(PunzoneStepDone.Q() == true) // Aspetta la fine del comando
                {
                    sequenza = PUNCHER_SECOND_UP_STATE;
                    cmd_exec = false;
                }
            }
            break;

            case PUNCHER_SECOND_UP_STATE :
            {
                if(!cmd_exec) // Dà il comando
                {
                    /// Torna nella posizione 
                    MotPunzone.moveRel(PUNZ_MOVE_ROTATIONS * +360.0, PUNZ_SPEED);
                    vTaskDelay(100);
                    cmd_exec = true;
                }
                else if(PunzoneStepDone.Q() == true) // Aspetta la fine del comando
                {
                    cntContainerCapsuleFull++;
                    cntContainerCoffeeFull++;

                    /// Incrementa i contatori della UI
                    local_caps_ses++;
                    local_caps_tot++;
                    set_var_contatore_caps_ses(local_caps_ses);
                    set_var_contatore_caps_totali(local_caps_tot);

                    sequenza = SERVO_LOADER_OPEN_STATE; // Ricomincia il ciclo
                    cmd_exec = false;
                }
            }
            break;
        }

        /**
         * Notifica
         */
        uint32_t ulNotifyVal = 0;
        BaseType_t xNotifyReceived = xTaskNotifyWait(0, ULONG_MAX, &ulNotifyVal, 0);
        if(xNotifyReceived == pdTRUE)
        {
            /// Richiesta di spegnimento da parte dell'Autokill
            if(xNotifyReceived & AUTOKILL_NOTIFY)
            {
                DatasToSave SendToAutokill;
                SendToAutokill.capsuleTotali = local_caps_tot;
                SendToAutokill.lastSequenza = sequenza;
                SendToAutokill.nCicliRimanenti = nCicliRimanenti;
                SendToAutokill.stepRimanentiPunzone = MotPunzone.abortCurrentCommand();
                SendToAutokill.stepRimanentiRalla = MotRalla.abortCurrentCommand();
                xQueueSend(AutokillQueueHandler, &SendToAutokill, portMAX_DELAY);
            }
        }

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
            case EMERGENCY_STATE                : return "EMERGENCY_STATE";
            case CONTAINER_FULL                 : return "CONTAINER_FULL";                
            case TIMEOUT_STATE                  : return "TIMEOUT_STATE";           
            case MACHINE_STARTUP_STATE          : return "MACHINE_STARTUP_STATE";       
            case PUNZONE_STARTUP_STATE          : return "PUNZONE_STARTUP_STATE";        
            case TAMBURO_STARTUP_STATE          : return "TAMBURO_STARTUP_STATE";      
            case QUIETE_STATE                   : return "QUIETE_STATE";            
            case SERVO_LOADER_OPEN_STATE        : return "SERVO_LOADER_OPEN_STATE";  
            case SERVO_LOADER_CLOSE_STATE       : return "SERVO_LOADER_CLOSE_STATE"; 
            case REACH_NEXT_STATION_STATE       : return "REACH_NEXT_STATION_STATE"; 
            case PUNCHER_FIRST_DOWN_STATE       : return "PUNCHER_FIRST_DOWN_STATE";   
            case PUNCHER_FIRST_UP_STATE         : return "PUNCHER_FIRST_UP_STATE";   
            case PUNCHER_SECOND_DOWN_STATE      : return "PUNCHER_SECOND_DOWN_STATE";
            case PUNCHER_SECOND_UP_STATE        : return "PUNCHER_SECOND_UP_STATE";   
            default                             : return "__INVALID_STATE__";    
        }
    #endif

    return "";
}