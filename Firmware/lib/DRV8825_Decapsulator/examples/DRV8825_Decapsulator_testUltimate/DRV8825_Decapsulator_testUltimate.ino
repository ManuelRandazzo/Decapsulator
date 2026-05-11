#include <DRV8825_Decapsulator.hpp>


/// Da cambiare man mano in base ai test
#define MIN_DELAY_MICROSECONDS 800 // gradiSecondo = 2250 °/s con uStepsScelti = 1 e 200 steps per giro
#define MAX_DELAY_MICROSECONDS 250000

DRV8825 stepper;

/// @enum per i dati dell'oggetto del MotionControl
enum motionRallaParams
{ 
  /// Pin Potenziometro
  POT_PIN = 11,



  STEPS = 200,

  /// Driver DRV8825 pins
  DIRECTION_PIN = 4,     /** @warning  Ancora da definire*/
  STEP_PIN = 5,          /** @warning  Ancora da definire*/
  ENABLE_PIN = 15,        /** @warning  Ancora da definire*/
  RESET_PIN = 7,         /** @warning  Ancora da definire*/
  SLEEP_PIN = 6,         /** @warning  Ancora da definire*/
  FAULT_PIN = 16,         /** @warning  Ancora da definire*/
};

uint8_t uStepsScelti = 16;
double gradiSecondo = 1440.0, V, last_V;

void setup()
{
  Serial.begin(115200);

  /// Potenziometro
  pinMode(POT_PIN, INPUT);
  analogReadResolution(12);

  stepper.begin(DIRECTION_PIN, STEP_PIN, ENABLE_PIN, RESET_PIN, SLEEP_PIN);
  if(stepper.isSleeping())
    stepper.wakeup();
  stepper.setDirection(DRV8825_CLOCK_WISE);
  stepper.enable();

  V = 360000000.0 / (gradiSecondo * double(uStepsScelti) * double(STEPS));
  delay(2000);
  stepper.step(1, V);
  Serial.printf("Initial speed : %.3f\n\n\n", V);

}




void getSpeedFromPot()
{
  /// evita calcoli in runtime se la lettura è uguale alla precedente
  static uint16_t lastRead;

  uint16_t read = analogRead(POT_PIN);

  /// evita calcoli in runtime se la lettura è uguale alla precedente
  if(read == lastRead)
    return;

  Serial.printf("Lettura Pot : %.d\n", read);
  V = map(read, 0, 4096, MIN_DELAY_MICROSECONDS, MAX_DELAY_MICROSECONDS);
}


void logInfos()
{
  if(last_V != V) /// Evita di saturare la UART con i logs se la velocità non cambia
  {
    /// Muove il motore
    last_V = V;

    //if(V >= MIN_DELAY_MICROSECONDS && V <= MAX_DELAY_MICROSECONDS)
    //  stepper.stepContinuous(V);

    Serial.printf("Periodo Delay: V = 360000000.0 / (%.5f * %d * %d) = %.5fus\n", gradiSecondo, uStepsScelti, STEPS, V);
    const double errore = V - double(uint32_t(V));
    Serial.printf("Delay reale dovuto all'errore di cast: %.0fus\n", V - errore );
    Serial.printf("Errore delay dovuto al cast: %.5fus\n\n", errore);
    Serial.printf("Frequenza Delay: fdelay = 1/V*10^-6 = %.5fHz\n", 1000000.0 / V);
    Serial.printf("Frequenza reale dovuta all'errore di cast: %.5fHz\n", 1000000.0 / (V - errore) );
    Serial.printf("Errore frequenza dovuto al cast : %.5fHz\n\n\n\n\n\n", (1000000.0 / (V - errore)) - (1000000.0 / V) );
  }
}




bool getStartFromSerial()
{
  if(Serial.available())
  {
    String line = Serial.readStringUntil('\n'); // leggi fino all'Invio
    line.trim();
    if(line.length() > 0)
    {
      line.toLowerCase();
      if(line == "true" || line == "1" || line == "start")
        return true;
    }
  }

  return false;
}





void loop()
{
  //getSpeedFromPot();
  logInfos();


  static uint8_t prgSw;
  static uint32_t timer;
  static uint8_t counter;
  switch(prgSw)
  {
    case 0:
      if(getStartFromSerial())
      {
        /// Fa 10 giri
        stepper.setDirection(DRV8825_CLOCK_WISE);
        //stepper.step(10 * uStepsScelti * STEPS, 750);
        counter = 0;
        prgSw = 3;
      }
    break;
    case 1:
      if(stepper.isStepDone())
      {
        stepper.setDirection(DRV8825_COUNTERCLOCK_WISE);
        stepper.stepContinuous(4000);
        timer = millis();
        prgSw++;
      }
    break;
    case 2:
      if(millis() - timer >= 5000)
      {
        stepper.setDirection(DRV8825_CLOCK_WISE);
        stepper.step(2 * uStepsScelti * STEPS, 10000);
        prgSw++;
      }
    break;
    case 3:
      if(stepper.isStepDone())
      {
        stepper.setDirection(DRV8825_COUNTERCLOCK_WISE);
        stepper.step(uStepsScelti * 150, 1000);
        counter++;
        prgSw++;
      }
    break;
    case 4:
      if(counter == 4)
        prgSw = 0;
      else if(stepper.isStepDone())
      {
        timer = millis();
        prgSw++;
      }
    break;
    case 5:
      if(millis() - timer >= 2000)
        prgSw = 3;
    break;
  }


/*
  if(stepper.isStepDone())
  {
    Serial.println("delay");
    stepper.disable();
    delay(1000);
    stepper.enable();
    stepper.step(25 * 200 * 16, V);
  }*/


}
