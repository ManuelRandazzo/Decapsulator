#include "JogMotoriPRG.hpp"
#include "HMI_UI_EEZ/created_post_prj_export/ui_vars_mutexs.h"
#include "HMI_UI_EEZ/vars.h"


void prgJogMotoriTask(void *pvParameters)
{
    TickType_t getLastTick = xTaskGetTickCount();

    /// NON SERVONO PIU' SE SI USANO GLI STATI
    bool lastRallaEnable = false;
    bool lastPunzoneEnable = false;

    /// Sequenza Homing
    uint8_t StateHoming = 0;


    double gradi_per_click_ralla = 0.0;
    double speed_motore_ralla    = 0.0;
    double gradi_per_click_punz  = 0.0;
    double speed_motore_punz     = 0.0;

    /// Si assicura di Auto-sospendersi per evitare problemi con le risorse condivise (motori)
    /// Sarà la task dell'HMI che deciderà se sospendere o attivare le task del programma main e di jog
    vTaskSuspend(NULL);

    while(1)
    {
        #pragma region (PUNZONE)

        if(MotPunzone.HardMax != nullptr)
            set_var_stato_finecorsa_max(MotPunzone.HardMax->rawRead() == MotPunzone.HardMax->getLevelTrig());
        
        if(MotPunzone.HardMin != nullptr)
            set_var_stato_finecorsa_min(MotPunzone.HardMin->rawRead() == MotPunzone.HardMin->getLevelTrig());

        /// Converte le stringhe in numeri double positivi
        gradi_per_click_punz = abs(String(get_var_gradi_per_click_punz()).toDouble());
        speed_motore_punz    = abs(String(get_var_speed_motore_punz()).toDouble());

        /// Enable/Disable Motore Punzone
        if(get_var_comando_motore_punzone() == true)
        {
            if(MotPunzone.isDetached())
            {
                LogDebug("punzone", "Attach");
                MotPunzone.attach();
                MotPunzone.Start();
            }
        }
        else
        {
            if(MotPunzone.isAttached())
            {
                LogDebug("punzone", "Detach");
                MotPunzone.abortCurrentCommand();
                MotPunzone.Stop(RELEASE);
                MotPunzone.detach();
            }
        }

        if(MotPunzone.isStepDone())
        {
            int8_t punz_jog_dir = 0;
            if(xQueueReceive(queue_direzione_comando_punzone, &punz_jog_dir, 0) == pdTRUE)
            {
                LogDebug("JogPunzCmd", "Comando Ralla Ricevuto Del Jog : %s (%d)\nGradi_per_click : %.2f\nSpeed : %.2f", punz_jog_dir == 1 ? "Jog+" : punz_jog_dir == -1 ? "Jog-" : "ERRORE", punz_jog_dir, gradi_per_click_punz, speed_motore_punz);
                MotPunzone.moveRel(punz_jog_dir * gradi_per_click_punz, speed_motore_punz);
            }
        }

        #pragma endregion (PUNZONE)
        




        
        
        #pragma region (RALLA)
        
        /// Converte le stringhe in numeri double positivi
        gradi_per_click_ralla = abs(String(get_var_gradi_per_click_ralla()).toDouble());
        speed_motore_ralla    = abs(String(get_var_speed_motore_ralla()).toDouble());

        if(MotRalla.HardMax != nullptr)
            set_var_stato_sensore_di_calibrazione(MotRalla.HardMax->rawRead() == MotRalla.HardMax->getLevelTrig());
        
        if(MotRalla.HardMin != nullptr)
            set_var_stato_sensore_di_calibrazione(MotRalla.HardMin->rawRead() == MotRalla.HardMin->getLevelTrig());

        /// Enable/Disable Motore Tamburo
        if(get_var_comando_motore_ralla() == true)
        {
            if(MotRalla.isDetached())
            {
                LogDebug("ralla", "Attach");
                MotRalla.attach();
                MotRalla.Start();
            }
        }
        else
        {
            if(MotRalla.isAttached())
            {
                LogDebug("ralla", "Detach");
                MotRalla.abortCurrentCommand();
                MotRalla.Stop(RELEASE);
                MotRalla.detach();
            }
        }

        if(MotRalla.isStepDone())
        {
            int8_t ralla_jog_dir = 0;
            if(xQueueReceive(queue_direzione_comando_ralla, &ralla_jog_dir, 0) == pdTRUE)
            {
                LogDebug("JogRallaCmd", "Comando Ralla Ricevuto Del Jog : %s (%d)\nGradi_per_click : %.2f\nSpeed : %.2f", ralla_jog_dir == 1 ? "Jog+" : ralla_jog_dir == -1 ? "Jog-" : "ERRORE", ralla_jog_dir, gradi_per_click_ralla, speed_motore_ralla);
                MotRalla.moveRel(ralla_jog_dir * gradi_per_click_ralla, speed_motore_ralla);
            }
        }
            
        #pragma endregion (RALLA)
        
        

        #pragma region (HOMING)

        
        switch(StateHoming)
        {
            case 0 : /// NO HOMING IN CORSO
                if(get_var_homing() == true)
                    StateHoming++;
            break;

            case 1 : /// HOMING CMD PUNZONE
                MotPunzone.home(PUNZ_HOME_SPEED, PUNZ_HOME_ACC, PUNZ_HOME_DEC, PUNZ_HOME_DIR, PUNZ_POST_HOME_POS);
                StateHoming++;
            break;

            case 2 : /// HOMING WAIT DONE PUNZONE
                if(MotPunzone.isHomeDone())
                    StateHoming++;
            break;

            case 3 : /// HOMING CMD RALLA
                MotRalla.reattachHardLimits();
                MotRalla.home(RALLA_HOME_SPEED, RALLA_HOME_ACC, RALLA_HOME_DEC, RALLA_HOME_DIR, RALLA_POST_HOME_POS);
                StateHoming++;
            break;

            case 4 : /// HOMING WAIT DONE RALLA
                if(MotRalla.isHomeDone())
                {
                    MotRalla.removeHardLimits();
                    StateHoming = 0;
                    set_var_homing(false);
                }
            break;
        }

        #pragma endregion (HOMING)

        
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

        xTaskDelayUntil(&getLastTick, JogMotori_delay);
    }

    vTaskDelete(NULL);
}