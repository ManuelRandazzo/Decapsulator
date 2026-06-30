#include <Servo.h>
Servo s;
void setup() {
  s.attach(3);
  pinMode(2, INPUT_PULLUP);
  delay(350); //VCC=5V --> 1400=360°   1050=270°   700=180°   350=90°
  s.write(90);



}
void loop()
{
 // while(digitalRead(2) != LOW);

  s.write(180);   
  delay(2000); //VCC=5V --> 1400=360°   1050=270°   700=180°   350=90°
  s.write(90);    
  delay(2000);  

  s.write(0); 
  delay(2000);  

  s.write(90);    
  delay(2000);  
}

