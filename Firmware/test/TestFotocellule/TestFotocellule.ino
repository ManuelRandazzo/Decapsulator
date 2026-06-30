#define D_PIN 4
#define A_PIN 5

volatile bool intOccured = false;
volatile bool fallOrRise;
volatile uint16_t analogVal;



void IRAM_ATTR ISR()
{
  intOccured = true;
  fallOrRise ^= 1;
  analogVal = analogRead(A_PIN);
}

void setup()
{
  Serial.begin(115200);
  pinMode(D_PIN, INPUT);
  pinMode(A_PIN, INPUT);
  attachInterrupt(D_PIN, ISR, CHANGE);
  delay(1000);
  Serial.println("-------------------------------------------------------------------------------------------------------------");
  Serial.println("Program Initialized!!");
  Serial.println("Waiting for Interrupt to occour.\n\n");

}

void loop()
{
  static uint8_t state = 0;
  static bool value;
  static uint32_t startInterrupt;
  switch(state)
  {
    case 0:
      if(intOccured)
      {
        startInterrupt = millis();
        value = fallOrRise;
        state++;
      }
    break;
    case 1:
      if(millis() - startInterrupt >= 50)
        if(fallOrRise == value)
          state++;
        else
        {
          intOccured = false;
          state = 0;
        }
    break;
    case 2:
      Serial.printf("%s, soglia : %fV\n\n", (fallOrRise ? "Rising" : "Falling"), (3.3 * float(analogVal) / 4095.0));
      intOccured = false;
      state = 0;
    break;
  }
}









