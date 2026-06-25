#include "JogMotoriPRG.hpp"
#include "HMI_UI_EEZ/created_post_prj_export/ui_vars_mutexs.h"
#include "HMI_UI_EEZ/vars.h"


void prgJogMotoriTask(void *pvParameters)
{
    TickType_t getLastTick = xTaskGetTickCount();

    /// Sequenza Homing
    uint8_t StateHoming = 0;

    double gradi_per_click_ralla = 360.0;
    double speed_motore_ralla    = 90.0;
    double gradi_per_click_punz  = 360.0;
    double speed_motore_punz     = 90.0;

    /// Si assicura di Auto-sospendersi per evitare problemi con le risorse condivise (motori)
    /// Sarà la task dell'HMI che deciderà se sospendere o attivare le task del programma main e di jog
    vTaskSuspend(NULL);

    uint32_t tmr = 0;
    
    while(1)
    {
        #pragma region (PUNZONE)

        /// Legge il sensore di finecorsa MINIMO del Punzone
        if(MotPunzone.HardMin.event())
            set_var_stato_finecorsa_min(MotPunzone.HardMin.rawRead() == (int8_t)(PUNZ_CAM_SIGNAL));

        /// Legge il sensore di finecorsa MASSIMO del Punzone
        if(MotPunzone.HardMax.event())
            set_var_stato_finecorsa_max(MotPunzone.HardMax.rawRead() == (int8_t)(PUNZ_CAM_SIGNAL));
        
        /// Converte le stringhe in numeri double positivi
        gradi_per_click_punz = abs(String(get_var_gradi_per_click_punz()).toDouble());
        speed_motore_punz    = abs(String(get_var_speed_motore_punz()).toDouble());

        /// Enable/Disable Motore Punzone
        if(get_var_comando_motore_punzone() == true)
        {
            if(MotPunzone.isDetached())
            {
                MotPunzone.attach();
                MotPunzone.Start();
            }
        }
        else
        {
            if(MotPunzone.isAttached())
            {
                MotPunzone.abortCurrentCommand();
                xQueueReset(queue_direzione_comando_punzone);
                MotPunzone.Stop(RELEASE);
                MotPunzone.detach();
            }
        }

        if(MotPunzone.isStepDone())
        {
            int8_t punz_jog_dir = 0;
            if(xQueueReceive(queue_direzione_comando_punzone, &punz_jog_dir, 0) == pdTRUE)
                MotPunzone.moveRel(punz_jog_dir * gradi_per_click_punz, speed_motore_punz);
        }

        
        set_var_stato_motore_punzone(StateHoming == 0 ? !MotPunzone.isStepDone() : !MotPunzone.isHomeDone());

        #pragma endregion (PUNZONE)
        




        
        
        #pragma region (RALLA)
        
        /// Converte le stringhe in numeri double positivi
        gradi_per_click_ralla = abs(String(get_var_gradi_per_click_ralla()).toDouble());
        speed_motore_ralla    = abs(String(get_var_speed_motore_ralla()).toDouble());

        /// Legge il sensore di calibrazione della RALLA
        set_var_stato_sensore_di_calibrazione(MotRalla.HardMax.rawRead() == (int8_t)(RALLA_CAM_SIGNAL));
                    
        /// Enable/Disable Motore Tamburo
        if(get_var_comando_motore_ralla() == true)
        {
            if(MotRalla.isDetached())
            {
                MotRalla.attach();
                MotRalla.Start();
            }
        }
        else
        {
            if(MotRalla.isAttached())
            {
                MotRalla.abortCurrentCommand();
                xQueueReset(queue_direzione_comando_ralla);
                MotRalla.Stop(RELEASE);
                MotRalla.detach();
            }
        }

        if(MotRalla.isStepDone())
        {
            int8_t ralla_jog_dir = 0;
            if(xQueueReceive(queue_direzione_comando_ralla, &ralla_jog_dir, 0) == pdTRUE)
                MotRalla.moveRel(ralla_jog_dir * gradi_per_click_ralla, speed_motore_ralla);
        }

        set_var_stato_motore_ralla(StateHoming == 0 ? !MotRalla.isStepDone() : !MotRalla.isHomeDone());
            
        #pragma endregion (RALLA)
        
        

        #pragma region (HOMING)

        /// Blocca l'homing se l'utente ripreme il tasto
        if(get_var_homing() == false && StateHoming != 0)
        {
            MotPunzone.abortCurrentCommand();
            MotRalla.abortCurrentCommand();
            MotRalla.removeHardLimits();
            StateHoming = 0;
        }

        switch(StateHoming)
        {
            case 0 : /// NO HOMING IN CORSO
            {
                if(get_var_homing() == true)                
                {
                    MotPunzone.abortCurrentCommand();
                    MotRalla.abortCurrentCommand();
                    MotRalla.removeHardLimits();
                    xQueueReset(queue_direzione_comando_punzone);
                    xQueueReset(queue_direzione_comando_ralla);
                    StateHoming++;
                }
            }
            break;

            case 1 : /// HOMING CMD PUNZONE
            {
                MotPunzone.home(PUNZ_HOME_SPEED, HARD_MAX, PUNZ_HOME_DIR, PUNZ_POST_HOME_POS);
                StateHoming++;
            }
            break;

            case 2 : /// HOMING WAIT DONE PUNZONE
            {
                if(MotPunzone.isHomeDone())
                {
                    MotRalla.reattachHardLimits();
                    StateHoming++;
                }
            }
            break;

            case 3 : /// HOMING CMD RALLA
            {
                int8_t RallaHardMaxStatus = MotRalla.HardMax.rawRead();
                
                if(RallaHardMaxStatus != -1)
                {
                    if(RallaHardMaxStatus == RALLA_CAM_SIGNAL)
                    {
                        if(MotRalla.isStepDone())
                            MotRalla.moveRel(-RALLA_HOME_DIR * 360.0 * GEAR_RATIO_RALLA, RALLA_HOME_CONT_SPEED);
                    }
                    else
                    {
                        double gradi_post_home = 0.0;
                        if(!MotRalla.isStepDone())
                        {
                            MotRalla.abortCurrentCommand();
                            gradi_post_home = -3.3 * GEAR_RATIO_RALLA;
                        }
                        else
                            gradi_post_home = RALLA_POST_HOME_POS * GEAR_RATIO_RALLA;

                        MotRalla.home(RALLA_HOME_SPEED, HARD_MAX, RALLA_HOME_DIR, gradi_post_home);
                        StateHoming++;
                    }
                }
            }
            break;

            case 4 : /// HOMING WAIT DONE RALLA
            {
                if(MotRalla.isHomeDone())
                {
                    MotRalla.removeHardLimits();
                    StateHoming = 0;
                    set_var_homing(false);
                    /// @todo notificare che bisogna skippare un'apertura del ServoParatia
                }
            }
            break;
        }

        #pragma endregion (HOMING)
                

        xTaskDelayUntil(&getLastTick, JogMotori_delay);
    }

    vTaskDelete(NULL);
}