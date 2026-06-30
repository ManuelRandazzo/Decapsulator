

#define PIECE_PRESENCE_PIN 1
#define PIECE_PASSED_PIN   2


void setup()
{
  Serial.begin(115200);
  pinMode(PIECE_PRESENCE_PIN, INPUT);
  pinMode(PIECE_PASSED_PIN, INPUT);
  delay(5000);
  Serial.println("Program Start");
}

void loop()
{
  int presence = digitalRead(PIECE_PRESENCE_PIN);
  int passed = digitalRead(PIECE_PASSED_PIN);

  Serial.printf("Presence : %s\n", presence == LOW ? "LOW" : "HIGH");
  Serial.printf("passed : %s\n\n", passed == LOW ? "LOW" : "HIGH");

  delay(1000);
}
