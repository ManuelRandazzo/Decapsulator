#include "decapsulator_io.hpp"

#pragma region (OGGETTI_IO)

MOTION MotRalla;
MOTION MotPunzone;
Servo ServoParatia;
FanCtrl Ventola;
DebPinHandler autoKill;
DebPinHandler cadutaCaps;
DebPinHandler presenzaCaps;

#pragma endregion (OGGETTI_IO)

/**
 * @brief Inizializza Motori, sensori, input e output del Decapsulator
 */
BaseType_t decapsulator_io_begin(void)
{
    /// SD Card - Configurazione dei pin SPI dedicati sull'ESP32-S3 (dichiarati nel platformio.ini)
    SD_Card.Init(TFT_SCLK, TFT_MISO, TFT_MOSI, SD_CS);
       


    /// Ventola
    Ventola.begin(VENTOLA_PIN, VENTOLA_FREQ, VENTOLA_RES);
    Ventola.on();



    /// Autokill
    autoKill.begin(INTR, AUTOKILL_DETECT_PIN, "Autokill Detection Pin", 10/* ms */, FALLING, INPUT);
    pinMode(AUTOKILL_SHUTDOWN_PIN, OUTPUT);



    /// Motore Tamburo
    drv_err_t drvErr;
    drvErr = MotRalla.Init(RALLA_MOTOR_STEPS, RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN,
                           RALLA_TASK_PRIORITY, RALLA_MICROSTEP);

    MotRalla.detach();

    MotRalla.setHardLimits(RALLA_CALIB_PIN, 255, RALLA_CALIB_INTR_OR_POLL, 30, RALLA_INPUT_PULL, RALLA_CAM_SIGNAL);


    if(drvErr != DRV_OK)
    {
        #ifdef LOG_ACTIVE_MAIN_PRG
            LogError("Main Prg", "Errore nell'inizializzazione del MOTION della Ralla, codice errore = %s (%d)", drv_err_to_name(drvErr), drvErr);
        #endif
        while(1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }



    /// Motore Punzone
    drvErr = MotPunzone.Init(PUNZ_MOTOR_STEPS, PUNZ_DIRECTION_PIN, PUNZ_STEP_PIN, PUNZ_ENABLE_PIN, PUNZ_RESET_PIN, PUNZ_SLEEP_PIN,
                             PUNZ_TASK_PRIORITY, PUNZ_MICROSTEP);
    MotPunzone.detach();
    
    if(drvErr != DRV_OK)
    {
        #ifdef LOG_ACTIVE_MAIN_PRG
            LogError("Main Prg", "Errore nell'inizializzazione del MOTION del Punzone, codice errore = %s (%d)", drv_err_to_name(drvErr), drvErr);
        #endif
        while(1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    MotPunzone.setHardLimits(PUNZ_MAX_POS_PIN, PUNZ_MIN_POS_PIN, PUNZ_HARD_LIM_INTR_OR_POLL, 30, PUNZ_INPUT_PULL, PUNZ_CAM_SIGNAL);



    /// Scivolo
    /// Inizializzazione Servo e relativi suoi timer[0-3] dell'hardware ledc
    for(uint8_t i = 0; i < 4; i++)
        ESP32PWM::allocateTimer(i);
	ServoParatia.setPeriodHertz(50);    // standard 50 hz servo

    cadutaCaps.begin(INTR, PIECE_PASSED_PIN   , "Caduta Capsule Pin"    , 10/* ms */, FALLING , INPUT);
    presenzaCaps.begin(INTR, PIECE_PRESENCE_PIN , "Presenza Capsule Pin"  , 30/* ms */, FALLING, INPUT); 

    if(cadutaCaps.event())
    {
        /// ATTENZIONE: Loggare Qualcosa nel display
        #ifdef LOG_ACTIVE_MAIN_PRG
            LogError("Main Prg", "C'è una capsula incastrata nello scivolo, consultare il Manuale di uso e manutenzione");
        #endif

        int8_t rawRead;
        do
        {
            rawRead = cadutaCaps.rawRead();
            vTaskDelay(pdMS_TO_TICKS(1000));
            #ifdef LOG_ACTIVE_MAIN_PRG
                LogError("Main Prg", "Valore lettura = %s", rawRead ? "HIGH" : "LOW");
            #endif
        } while(rawRead == cadutaCaps.getLevelTrig());
    }

    return pdTRUE;
}

/**
 *  @brief funzione chiamata in caso di emergenza, di servizio o di timeout
 * 
 *  @param ptrStepsLeftTamburo puntatore a cui viene settato il numero di gradi rimanenti
 *         per completare il movimento che verrà abortito chiamando questa funziione 
 *  
 *  @param ptrStepsLeftPunzone puntatore a cui viene settato il numero di gradi rimanenti
 *         per completare il movimento che verrà abortito chiamando questa funziione
 * 
 *  @note Se non si implementa una logica per riprendere l'ultimo movimento allora
 *        dopo aver chiamato questa funzione va rifatto l'homing
 * 
 *  @details Fa l'ABORT dei commandi attuali dei motori, FERMA i motori e fa DETACH dei pin RILASCIANDO la coppia
 */
void MainPrgStopAllMotors(double* ptrStepsLeftTamburo, double* ptrStepsLeftPunzone)
{  
    /// Abort dei comandi attuali ai motori
    int64_t stepsLeftRalla = (int64_t)MotRalla.abortCurrentCommand();
    int64_t stepsLeftPunz = (int64_t)MotPunzone.abortCurrentCommand();


    /// Ferma gli stepper e rilasciano la coppia
    MotRalla.Stop(RELEASE);
    MotPunzone.Stop(RELEASE);

    /// Scollega i pin dei motori
    MotRalla.detach();
    MotPunzone.detach();
    ServoParatia.detach();

    /// Restituisce il numero di gradi rimanenti del comando abortito
    if(ptrStepsLeftTamburo != nullptr)
        *ptrStepsLeftTamburo = MotRalla.stepsToGradi(stepsLeftRalla);

    if(ptrStepsLeftPunzone != nullptr)
        *ptrStepsLeftPunzone = MotPunzone.stepsToGradi(stepsLeftPunz);
}