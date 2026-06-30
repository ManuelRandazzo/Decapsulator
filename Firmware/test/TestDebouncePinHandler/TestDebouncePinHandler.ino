#include "DebouncePinHandler.hpp"

#define PIN 4

uint32_t tmr;
DebPinHandler* hallEffect = nullptr;

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Program started");
  
  hallEffect = new DebPinHandler(POLL, PIN, "Hall Effect Sensor Test", 10000, RISING, INPUT);

  tmr = millis();
}



void loop()
{
  if(hallEffect->IsInterrupt())
  {
    hallEffect->intrUpdate();
  }
  else
  {
    hallEffect->pollUpdate();
  }


  if(hallEffect->event())
  {
    tmr = millis();
    Serial.printf("Evento avvenuto a %ums\nHall Effect Status : %s\n\n", tmr, hallEffect->toString());
  }

  if(millis() - tmr >= 2000)
  {
    tmr = millis();
    Serial.printf("Hall Effect Global Status : %ums\n%s\n\n", tmr, hallEffect->toString().c_str());
  }
}
