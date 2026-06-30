#include "DebouncePinHandler.hpp"

//DebPinHandler pinDeb(INTR, 4, "Pin test", 30, RISING, INPUT);
bool flag = false;
DebPinHandler* pinDeb;
uint32_t timer = 0;


void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Inizio Programma\n");
  pinDeb = new DebPinHandler(INTR, 4, "Pin test", 10, INPUT_PULLUP, RISING);
  pinMode(35, OUTPUT);
}

void loop()
{
  pinDeb->intrUpdate();

  if(pinDeb->event())
  {
    digitalWrite(35, flag);
    flag ^= 1; // toggle
  }

  /*const uint32_t millisecs = millis();
  if(millisecs - timer >= 100)
  {
    timer = millisecs;
    Serial.printf("Pin Status :\n%s\n\n", pinDeb->toString().c_str());
  }*/
}
