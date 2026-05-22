#include "JogMotoriPRG.hpp"


void prgJogMotoriTask(void *pvParameters)
{
    TickType_t getLastTick = xTaskGetTickCount();
    CommandQueueHMI_t FromHMI = defaultCommandQueueHMI;
    EventQueueHMI_t ToHMI = defaultEventQueueHMI;
    bool sendChangesToHMI = false;

    /// NON SERVONO PIU' SE SI USANO GLI STATI
    bool lastRallaEnable = false;
    bool lastPunzoneEnable = false;

    /// Per sapere quando sta facendo la sequenza di homing
    bool nowHoming = false;
    uint8_t StateHoming = 0;

    /// Si assicura di Auto-sospendersi per evitare problemi con le risorse condivise (motori)
    /// Sarà la task dell'HMI che deciderà Se sospendere o attivare le task del programma main e di jog
    vTaskSuspend(NULL);

    while(1)
    {
        switch(FromHMI.jogStateCMD)
        {
            case NO_CMD :
                if(MotRalla.isStepDone() && MotPunzone.isStepDone() && !nowHoming)
                    checkUpdateHMI(&FromHMI);
            break;

            case TAMBURO_ENABLE :
                MotRalla.attach();
                MotRalla.Start();
                FromHMI.jogStateCMD = NO_CMD;
            break;

            case TAMBURO_DISABLE :
                MotRalla.Stop(RELEASE);
                MotRalla.detach();
                FromHMI.jogStateCMD = NO_CMD;
            break;

            case TAMBURO_JOG_POSITIVE :
                MotRalla.moveRel(FromHMI.jogRallaGradiPerClick, FromHMI.jogRallaSpeed);
                FromHMI.jogStateCMD = NO_CMD;
            break;

            case TAMBURO_JOG_NEGATIVE :
                MotRalla.moveRel(-1 * FromHMI.jogRallaGradiPerClick, FromHMI.jogRallaSpeed);
                FromHMI.jogStateCMD = NO_CMD;
            break;
                
            case PUNZONE_ENABLE :
                MotPunzone.attach();
                MotPunzone.Start();
                FromHMI.jogStateCMD = NO_CMD;
            break;

            case PUNZONE_DISABLE :
                MotPunzone.Stop(RELEASE);
                MotPunzone.detach();
                FromHMI.jogStateCMD = NO_CMD;
            break;

            case PUNZONE_JOG_POSITIVE :
                MotPunzone.moveRel(FromHMI.jogPunzoneGradiPerClick, FromHMI.jogPunzoneSpeed, PUNZ_ACC, PUNZ_DEC);
                FromHMI.jogStateCMD = NO_CMD;
            break;

            case PUNZONE_JOG_NEGATIVE :
                MotPunzone.moveRel(-1 * FromHMI.jogPunzoneGradiPerClick, FromHMI.jogPunzoneSpeed, PUNZ_ACC, PUNZ_DEC);
                FromHMI.jogStateCMD = NO_CMD;
            break;

            case HOMING :
                switch(StateHoming)
                {
                    case 0 : /// HOMING CMD PUNZONE
                        MotPunzone.home(PUNZ_HOME_SPEED, PUNZ_HOME_ACC, PUNZ_HOME_DEC, PUNZ_HOME_DIR, PUNZ_POST_HOME_POS);
                        nowHoming = true;
                        StateHoming++;
                    break;

                    case 1 : /// HOMING WAIT DONE PUNZONE
                        if(MotPunzone.isHomeDone())
                            StateHoming++;
                    break;

                    case 2 : /// HOMING CMD RALLA
                        MotRalla.home(RALLA_HOME_SPEED, RALLA_HOME_ACC, RALLA_HOME_DEC, RALLA_HOME_DIR, RALLA_POST_HOME_POS);
                        StateHoming++;
                    break;

                    case 3 : /// HOMING WAIT DONE RALLA
                        if(MotRalla.isHomeDone())
                        {
                            StateHoming = 0;
                            nowHoming = false;
                            FromHMI.jogStateCMD = NO_CMD;
                        }
                    break;
                }
            break;
        }

        
        /*/// PUNZONE
        if(FromHMI.jogRallaEnable != lastRallaEnable)
        {
            lastRallaEnable = FromHMI.jogRallaEnable;
            if(FromHMI.jogRallaEnable == 0)
            {
                MotRalla.Stop(RELEASE);
                MotRalla.detach();
            }
            else
            {
                MotRalla.attach();
                MotRalla.Start();
            }
        }

        if(MotRalla.isStepDone() == true && !nowHoming)
        {
            if(FromHMI.jogRallaJogPositive == true)
            {
                FromHMI.jogRallaJogPositive = false;
                MotRalla.moveRel(FromHMI.jogRallaGradiPerClick, FromHMI.jogRallaSpeed);
            }
            else if(FromHMI.jogRallaJogNegative == true)
            {
                FromHMI.jogRallaJogNegative = false;
                MotRalla.moveRel(-1 * FromHMI.jogRallaGradiPerClick, FromHMI.jogRallaSpeed);
            }

            
            if(FromHMI.jogPunzoneHome && MotPunzone.isHomeDone())
            {
                FromHMI.jogRallaHome = false;
                MotPunzone.home(PUNZ_HOME_SPEED, PUNZ_HOME_DIR, PUNZ_POST_HOME_POS);
            }
        }

        if(ToHMI.jogRallaIsMoving != (!MotRalla.isStepDone()))
        {
            ToHMI.jogRallaIsMoving = !MotRalla.isStepDone();
            sendChangesToHMI = true;
        }


        /// PUNZONE
        if(FromHMI.jogPunzoneEnable != lastPunzoneEnable)
        {
            lastPunzoneEnable = FromHMI.jogPunzoneEnable;
            if(FromHMI.jogPunzoneEnable == 1)
            {
                MotPunzone.attach();
                MotPunzone.Start();
            }
            else
            {
                MotPunzone.Stop(RELEASE);
                MotPunzone.detach();
            }
        }
        
        if(MotPunzone.isStepDone() == true)
        {
            if(FromHMI.jogPunzoneJogPositive == true)
            {
                FromHMI.jogPunzoneJogPositive = false;
                MotPunzone.moveRel(FromHMI.jogPunzoneGradiPerClick, FromHMI.jogPunzoneSpeed);
            }

            
            if(FromHMI.jogPunzoneJogNegative == true)
            {
                FromHMI.jogPunzoneJogNegative = false;
                MotPunzone.moveRel(-1 * FromHMI.jogPunzoneGradiPerClick, FromHMI.jogPunzoneSpeed);
            }


            if(FromHMI.jogPunzoneHome && MotPunzone.isHomeDone())
            {
                FromHMI.jogRallaHome = false;
                MotPunzone.home(PUNZ_HOME_SPEED, PUNZ_HOME_DIR, PUNZ_POST_HOME_POS);
            }
        }    

        if(ToHMI.jogPunzoneIsMoving != (!MotPunzone.isStepDone()))
        {
            ToHMI.jogPunzoneIsMoving = !MotPunzone.isStepDone();
            sendChangesToHMI = true;
        }*/

        /// Aggiorna in caso vengano richiesti dei cambiamenti da segnalare all'HMI
        sendUpdateHMI(&ToHMI, &sendChangesToHMI);

        xTaskDelayUntil(&getLastTick, JogMotori_delay);
    }

    vTaskDelete(NULL);
}