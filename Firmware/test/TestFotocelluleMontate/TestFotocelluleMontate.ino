
#include "DRV8825_Decapsulator_2.hpp"

/// QUI SI DECIDE SE SI USA RALLA O PUNZONE
#define USE_RALLA

#define VENTOLA_PIN            12

/// PIN DELLA RALLA
#define RALLA_DIRECTION_PIN    39
#define RALLA_STEP_PIN         38
#define RALLA_ENABLE_PIN       48
#define RALLA_RESET_PIN       255
#define RALLA_SLEEP_PIN       255
#define RALLA_FAULT_PIN        40
/// PIN DEL PUNZONE
#define PUNZ_DIRECTION_PIN     47
#define PUNZ_STEP_PIN          21
#define PUNZ_ENABLE_PIN        14
#define PUNZ_RESET_PIN        255
#define PUNZ_SLEEP_PIN        255
#define PUNZ_FAULT_PIN         13

#define PUNZ_MAX_POS_PIN       18
#define PUNZ_MIN_POS_PIN        8

#define PIECE_PRESENCE_PIN      1
#define PIECE_PASSED_PIN        2

enum SerialState { ATTESA_OK, ATTESA_NUMERO };
SerialState statoCorrente = ATTESA_OK;

// Buffer per la lettura
char buffer[32];
int indice = 0;


DRV8825 Motore;

void setup() {
  Serial.begin(115200);
  //esp_log_level_set("*", ESP_LOG_NONE);
  pinMode(18, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(VENTOLA_PIN, OUTPUT);
  digitalWrite(VENTOLA_PIN, HIGH);
  delay(5000);
  Serial.println("Started");

  #ifdef USE_RALLA
    Serial.printf("Begin error : %s\n", drv_err_to_name(Motore.begin(RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN, 200)));
    
    DRV8825 DisattivaPunzone;
    Serial.printf("Begin error : %s\n", drv_err_to_name(DisattivaPunzone.begin(PUNZ_DIRECTION_PIN, PUNZ_STEP_PIN, PUNZ_ENABLE_PIN, PUNZ_RESET_PIN, PUNZ_SLEEP_PIN, 200)));
    DisattivaPunzone.disable();
  #else
    Serial.printf("Begin error : %s\n", drv_err_to_name(Motore.begin(PUNZ_DIRECTION_PIN, PUNZ_STEP_PIN, PUNZ_ENABLE_PIN, PUNZ_RESET_PIN, PUNZ_SLEEP_PIN, 200)));
    DRV8825 DisattivaRalla;
    Serial.printf("Begin error : %s\n", drv_err_to_name(DisattivaRalla.begin(RALLA_DIRECTION_PIN, RALLA_STEP_PIN, RALLA_ENABLE_PIN, RALLA_RESET_PIN, RALLA_SLEEP_PIN, 200)));
    DisattivaRalla.disable();
  #endif

  Motore.enable();
  
  delay(200);
  //Serial.printf("Enable error : %s\n", drv_err_to_name(Motore.enable()));

  //Serial.printf("setDirection error : %s\n", drv_err_to_name(Motore.setDirection(DRV8825_COUNTERCLOCK_WISE)));
  //while(1);
  /*#ifdef USE_RALLA
    /// Gradi da fare PER RALLA
    const float gradi_da_fare = 1440.0;
    Motore.step((gradi_da_fare / 360.0)*200 * 3, 5500);
  #else
    /// PER PUNZONE
    const float mm_da_fare = 15.0;
    Motore.step((mm_da_fare / 2)*200, 2000);
  #endif*/
  
/*
  pinMode(PIECE_PRESENCE_PIN, INPUT);
  pinMode(PIECE_PASSED_PIN, INPUT);
  analogReadResolution(12);*/

  /*Motore.setDirection(DRV8825_COUNTERCLOCK_WISE);
  Motore.stepContinuous(10000);
  while(1)
    if(digitalRead(18) == LOW)
    {
      Motore.abortCurrentMovement();
      break;
    }*/

  
}


drv_direction_t direction;

uint8_t lastPresenceState = LOW;
uint8_t lastCadutaState = LOW;
void loop()
{
  Motore.update();

  if(Motore.isStepDone())
  {
    char c;
    Serial.print("\nInserisci a = avanti, i = indietro, e = enable, d = disable : ");
    do
    {
      c = Serial.read();
      
      switch(c)
      {
        case 'a' : Motore.setDirection(DRV8825_COUNTERCLOCK_WISE); Serial.println("avanti"); break;
        case 'i' : Motore.setDirection(DRV8825_CLOCK_WISE); Serial.println("indietro"); break;
        case 'e' : Motore.enable(); Serial.println("enable"); break;
        case 'd' : Motore.disable();  Serial.println("disable"); break;
      }
    }
    while(c != 'a' && c != 'i');

    Serial.print("\nMetti steps : ");
    uint64_t steps;
    do{
      steps = Serial.parseInt();
    }
    while(steps == 0);

    Serial.print("\nMetti velocita' (default 2500 [us/step] se premi invio) : ");
    uint64_t speed;
    do{
      speed = Serial.parseInt();
    }
    while(speed == 0);

    Serial.printf("Steps inseriti : %d\nSpeed inserita : %d\n", steps, speed);
    Motore.step(steps, speed);  
  }
  /*
  double presenceMillivolts = 0;
  double passedMillivolts = 0;

  uint16_t i = 0;
  while(i < 2000)
  {
    presenceMillivolts += (double)analogReadMilliVolts(PIECE_PRESENCE_PIN);
    passedMillivolts += (double)analogReadMilliVolts(PIECE_PASSED_PIN);
    i++;
  }

  presenceMillivolts /= i*1000.0; /// Fa la media e converte da milliVolt a Volt
  passedMillivolts /= i*1000.0;   /// Fa la media e converte da milliVolt a Volt
  Serial.printf("Piece Presence : %.4fV\n\n", presenceMillivolts);
  Serial.printf("Piece Passed : %.4fV\n\n\n\n\n\n", passedMillivolts);
  */
}
