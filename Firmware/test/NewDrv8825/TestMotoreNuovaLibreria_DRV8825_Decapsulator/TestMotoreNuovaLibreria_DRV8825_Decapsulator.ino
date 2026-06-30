
#include "DRV8825_Decapsulator.hpp"

#define RALLA_DIRECTION_PIN    39
#define RALLA_STEP_PIN         38
#define RALLA_ENABLE_PIN       48
#define RALLA_RESET_PIN       255
#define RALLA_SLEEP_PIN       255
#define RALLA_FAULT_PIN        40

DRV8825 motoreRalla;

void setup() {
  esp_log_level_set("*", ESP_LOG_NONE);
  Serial.begin(115200);
  delay(1000);
  Serial.printf("Begin error : %s\n", drv_err_to_name(motoreRalla.begin(RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN, 200)));
  
  delay(200);
  Serial.printf("Enable error : %s\n", drv_err_to_name(motoreRalla.enable()));

  Serial.printf("setDirection error : %s\n", drv_err_to_name(motoreRalla.setDirection(DRV8825_CLOCK_WISE)));
  

  motoreRalla.stepContinuous(1500);
  delay(1500);
  motoreRalla.abortCurrentMovement();
  delay(1500);
}

void loop() {
  motoreRalla.update();

  if(motoreRalla.isStepDone())
  {
    delay(300);
    Serial.printf("setDirection error : %s\n", drv_err_to_name(motoreRalla.setDirection(motoreRalla.getDirection() * -1)));
    Serial.printf("step(50, 2000) error : %s\n", drv_err_to_name(motoreRalla.step(50, 2000)));
  }
}
