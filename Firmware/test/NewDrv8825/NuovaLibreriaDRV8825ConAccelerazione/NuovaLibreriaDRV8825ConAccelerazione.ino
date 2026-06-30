
#include "DRV8825_Decapsulator.hpp"

#define RALLA_DIRECTION_PIN    4
#define RALLA_STEP_PIN         5
#define RALLA_ENABLE_PIN       6
#define RALLA_RESET_PIN       255
#define RALLA_SLEEP_PIN       255
#define RALLA_FAULT_PIN        40

DRV8825 motoreRalla;

uint64_t acceleration = 0;
uint64_t deceleration = 0;

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);
  Serial.begin(115200);
  delay(2500);
  Serial.printf("Begin error : %s\n", drv_err_to_name(motoreRalla.begin(RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN, 200)));
  
  delay(200);
  Serial.printf("Enable error : %s\n", drv_err_to_name(motoreRalla.enable()));

  Serial.printf("setDirection error : %s\n", drv_err_to_name(motoreRalla.setDirection(DRV8825_CLOCK_WISE)));
  

  /*motoreRalla.stepContinuous(5000);
  delay(5000);
  motoreRalla.abortCurrentMovement();
  delay(1000);*/
}

void loop()
{
  motoreRalla.update();

  if(motoreRalla.isStepDone())
  {
    motoreRalla.setDirection(motoreRalla.getDirection() == DRV8825_CLOCK_WISE ? DRV8825_COUNTERCLOCK_WISE : DRV8825_CLOCK_WISE);
    delay(2000);

    /// Triangolare
    motoreRalla.step(1600, 800, 5000, 5000);

    /// Trapezoidale
    //motoreRalla.step(200, 5000, 10000, 10000);
  }/*
    if(acceleration >= 100000)
      acceleration = 0;

    if(deceleration >= 100000)
      deceleration = 0;

    acceleration = 15;
    deceleration = 30;*/

    ///*Serial.printf("setDirection error : %s\n", drv_err_to_name(*/motoreRalla.setDirection(motoreRalla.getDirection() * -1)/*))*/;
    ///*Serial.printf("step(50, 2000) error : %s\n", drv_err_to_name(*/motoreRalla.step(200, 5000, acceleration, deceleration)/*))*/;
   // motoreRalla.step(200*40, 5000, acceleration, deceleration);
   // Serial.printf("ACCELERATION = %d\n", acceleration);
   // Serial.printf("DECELERATION = %d\n", deceleration);
   // Serial.printf("\n");

  //}
}
