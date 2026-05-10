#include "FanCtrl.hpp"




FanCtrl::FanCtrl()
{

}




FanCtrl::~FanCtrl()
{
    this->detach();
}




/**
 * @brief Inizializza il pin e l'hardware LEDC (LED Control)
 *
 * @param pin pin del micro a cui è connessa la ventola
 *
 * @param pwm_freq seleziona la frequenza della PWM.
 *
 * @param pwm_resolution_in_bits Range 1-20 bits. Seleziona la risoluzione in bit del canale LEDC.
 *
 * @return Se l'inizializzazione è avvenuta correttamente
 *
 * @attention Viene spenta la PWM alla fine del begin()
 */
bool FanCtrl::begin(uint8_t pin, uint32_t pwm_freq, uint8_t pwm_resolution_in_bits)
{
    /// ERRORE : non è stato assegnato un pin valido
    if(pin == 255)
        return false;

    /// ERRORE : non è stata assegnata una frequenza valida
    if(pwm_freq == 0)
        return false;

    /// ERRORE : non è stata assegnata una risoluzione valida
    if(pwm_resolution_in_bits < 1 || pwm_resolution_in_bits > 20)
        return false;

    /// Disattiva il pin precedente 
    if(!this->detach())
        return false;

    /// Imposta il pin come OUTPUT
    pinMode(this->fan_pin, OUTPUT);

    /// Tenta l'attach del nuovo pin
    bool attach_ok = ledcAttach(pin, pwm_freq, pwm_resolution_in_bits);

    if(!attach_ok)
        return false;

    /// Se è stato fatto l'attach allora assegna il nuovo pin
    this->fan_pin = pin;

    /// Spegne la ventola - duty-cycle = 0
    if(!this->off())
        return false;

    return true;
}





/**
 * @brief Scollega il pin dalla PWM
 */
bool FanCtrl::detach()
{
    bool detach_ok = true;

    if(this->fan_pin != 255)
    {
        detach_ok = ledcDetach(this->fan_pin);
        this->fan_pin = 255;
    }
    
    this->duty = 0;

    return detach_ok;
}




/**
 * @brief Cambia il duty-cycle della PWM
 *
 * @param duty_percentage Range 0-100. Seleziona il duty-cycle della PWM, espresso in percentuale [%].
 *
 * @attention se duty_percentage > 100 allora il duty viene impostato al 100%
 *
 * @return Se è stato cambiato correttamente il duty-cycle
 *
 * @example duty_percentage = 0 ---> ventola spenta
 *
 * @example duty_percentage = 100 ---> ventola accesa
 */
bool FanCtrl::setDuty(uint8_t duty_percentage)
{
    /// ERRORE : non è stato assegnato un pin valido
    if(this->fan_pin == 255)
        return false;

    /// Si assicura che il duty-cycle sia tra 0% e 100%
    if(duty_percentage > 100)
        duty_percentage = 100;
    
    /// Non scrive di nuovo lo stesso valore di duty-cycle
    if(duty_percentage == this->duty)
        return true;

    /// Tenta di fare la scrittura della PWM (Write)
    bool write_ok = ledcWrite(this->fan_pin, this->duty);

    /// Se la scrittura non è fallita assegna il nuovo duty-cycle
    if(write_ok)
        this->duty = duty_percentage;

    return write_ok;
}




/**
 * @brief Fa una rampa a livello hardware (zero CPU load, zero overhead, zero blocking) in un certo tempo 
 *        partendo dal duty cycle precedentemente impostato e arrivando al "target_duty"
 *
 * @param target_duty Range 0-100. Duty-cycle finale espresso in [%]
 *
 * @param max_fade_time_ms Tempo della rampa espresso in [ms]. MAX = 32.767 secondi
 *
 * @return Se la rampa è stata settata corretamente
 * 
 * @example Voglio che la rampa mi faccia andare la ventola da
 *          0% a 100% in 5 secondi allora scrivo
 *          Ventola.ramp(100, 5000);
 * 
 * @example Voglio che la rampa mi faccia andare la ventola da
 *          25% a 80% in 2.5 secondi allora scrivo
 *          Ventola.ramp(80, 2500);
 * 
 * @deprecated In realtà ledcFade() lo fa nel tempo minimo possibile 
 *             quindi non rispetta la rampa, avrebbe bisogno di un loop ma
 *             diventa inutile se non lo può fare l'hardware
 */
/*bool FanCtrl::ramp(uint32_t target_duty, int max_fade_time_ms)
{
    /// ERRORE : non è stato assegnato un pin valido
    if(this->fan_pin == 255)
        return false;

    /// Si assicura che il duty-cycle sia tra 0% e 100%
    if(target_duty > 100)
        target_duty = 100;

    /// Tenta di fare la rampa (Fade)
    bool fade_ok = ledcFade(this->fan_pin, this->duty, target_duty, max_fade_time_ms);

    /// Se la rampa non è fallita assegna il nuovo duty-cycle
    if(fade_ok)
        this->duty = target_duty;

    return fade_ok;
}*/




/**
 * @brief Setta la ventola al 100% - Ventola accesa
 */
bool FanCtrl::on()
{
    return this->setDuty(100);
}




/**
 * @brief Setta la ventola al 0% - Ventola spenta
 */
bool FanCtrl::off()
{
    return this->setDuty(0);
}




/**
 * @brief Cambia la frequenza e la risoluzione della PWM
 *
 * @param pwm_freq seleziona la frequenza della PWM.
 *
 * @param pwm_resolution_in_bits Range 1-20 bits. Seleziona la risoluzione in bit del canale LEDC.
 *
 * @return Se è stata cambiata correttamente la frequenza e la risoluzione
 */
bool FanCtrl::changeFrequency(uint32_t pwm_freq, uint8_t pwm_resolution_in_bits)
{
    /// ERRORE : non è stato assegnato un pin valido
    if(this->fan_pin == 255)
        return false;

    /// ERRORE : non è stata assegnata una frequenza valida
    if(pwm_freq == 0)
        return false;

    /// ERRORE : non è stata assegnata una risoluzione valida
    if(pwm_resolution_in_bits < 1 || pwm_resolution_in_bits > 20)
        return false;

    uint32_t errore = ledcChangeFrequency(this->fan_pin, pwm_freq, pwm_resolution_in_bits);

    /// se errore = 0 allora errore avvenuto, non è stata impostata la frequenza desiderata,
    /// altrimenti errore=frequenza_voluta e return=true
    return (errore != 0);
}












