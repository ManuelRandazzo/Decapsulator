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

    while(1)
    {
        #pragma region (PUNZONE)

        /// Legge il sensore di finecorsa MASSIMO del Punzone
        if(PUNZ_INPUT_PULL == INPUT_PULLUP)
            set_var_stato_finecorsa_max(!digitalReadFast(PUNZ_MAX_POS_PIN));
        else
            set_var_stato_finecorsa_max(digitalReadFast(PUNZ_MAX_POS_PIN));

        /// Legge il sensore di finecorsa MINIMO del Punzone
        if(PUNZ_INPUT_PULL == INPUT_PULLUP)
            set_var_stato_finecorsa_max(!digitalReadFast(PUNZ_MIN_POS_PIN));
        else
            set_var_stato_finecorsa_max(digitalReadFast(PUNZ_MIN_POS_PIN));
        
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

        set_var_stato_motore_punzone(!MotPunzone.isStepDone());

        #pragma endregion (PUNZONE)
        




        
        
        #pragma region (RALLA)
        
        /// Converte le stringhe in numeri double positivi
        gradi_per_click_ralla = abs(String(get_var_gradi_per_click_ralla()).toDouble());
        speed_motore_ralla    = abs(String(get_var_speed_motore_ralla()).toDouble());

        /// Legge il sensore di calibrazione della Ralla
        if(RALLA_INPUT_PULL == INPUT_PULLUP)
            set_var_stato_sensore_di_calibrazione(!digitalReadFast(RALLA_CALIB_PIN));
        else
            set_var_stato_sensore_di_calibrazione(digitalReadFast(RALLA_CALIB_PIN));
                    
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

        set_var_stato_motore_ralla(!MotRalla.isStepDone());
            
        #pragma endregion (RALLA)
        
        

        #pragma region (HOMING)

        /// Blocca l'homing se l'utente ripreme il tasto
        if(get_var_homing() == false && StateHoming != 0)
        {
            MotPunzone.abortCurrentCommand();
            MotRalla.abortCurrentCommand();
            StateHoming = 0;
        }

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

        

        xTaskDelayUntil(&getLastTick, JogMotori_delay);
    }

    vTaskDelete(NULL);
}