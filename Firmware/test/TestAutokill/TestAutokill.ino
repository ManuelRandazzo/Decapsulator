
#include "hal/gpio_ll.h"

#define AK_PIN 45
#define AUTOKILL_SHUTDOWN_PIN 17
#define DEBOUNCE_MS 10  // Dovrebbe essere ok dimensionato così

#define digitalWriteFast(gpio_pin, level) gpio_ll_set_level(&GPIO, gpio_pin, level)
#define digitalReadFast(gpio_pin)         gpio_ll_get_level(&GPIO, gpio_pin)

volatile bool flag = false;

void autoKillISR()
{
  flag = true;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");
  pinMode(AK_PIN, INPUT);
  attachInterrupt(AK_PIN, autoKillISR, FALLING);
  pinMode(AUTOKILL_SHUTDOWN_PIN, OUTPUT);
}

uint8_t sw = 0;
uint32_t tmr = 0;
bool prevLevel = false;
uint8_t out_level = LOW;
void loop()
{
  switch(sw)
  {
    case 0:
      if(flag)
      {
        tmr = millis();
        prevLevel = digitalReadFast(AK_PIN);
        sw++;
      }
    break;

    case 1:
      if(millis() - tmr >= DEBOUNCE_MS)
        if(prevLevel == digitalReadFast(AK_PIN))
          sw++;
        else
        {
          flag = false;
          sw = 0;
        }
    break;

    case 2:
      /// Attiva il mosfet così che possa scorrere la corrente della batteria
      digitalWriteFast(AUTOKILL_SHUTDOWN_PIN, prevLevel ? LOW : HIGH);
      Serial.println("BAT_SUPPORT");
      sw = 0;
      flag = false;
    break;
  }
}
