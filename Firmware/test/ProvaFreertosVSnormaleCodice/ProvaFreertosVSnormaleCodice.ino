///*
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <MPU6050.h>
#include <U8g2lib.h>

// ==== DHT11 Setup ====
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ==== MPU6050 Setup ====
MPU6050 mpu;

// ==== OLED Setup (SH1106 I2C) ====
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ==== LED_BUILTIN Setup ====
#define LED_BUILTIN 2

// ==== Task Handles ====
TaskHandle_t TaskDHT;
TaskHandle_t TaskMPU;
TaskHandle_t TaskBlink;
TaskHandle_t TaskOLED;  // <-- Nuova task

// ==== Variabile condivisa ====
volatile float blinkIntervalMs = 0;

void setup() {
  Serial.begin(921600);
  pinMode(LED_BUILTIN, OUTPUT);

  dht.begin();
  Wire.begin(21, 22); // SDA, SCL

  mpu.initialize();
  if (!mpu.testConnection())
    Serial.println("MPU6050 non connesso!");
  else
    Serial.println("MPU6050 connesso!");

  u8g2.begin();  // Inizializza display

  // Task DHT
  xTaskCreatePinnedToCore(TaskReadDHT, "Task DHT", 2048, NULL, 1, &TaskDHT, 0);

  // Task MPU
  xTaskCreatePinnedToCore(TaskReadMPU, "Task MPU", 4096, NULL, 1, &TaskMPU, 1);

  // Task Blink
  xTaskCreatePinnedToCore(TaskBlinkLED, "Task Blink", 2048, NULL, 10, &TaskBlink, 1);

  // Task OLED
  xTaskCreatePinnedToCore(TaskDisplayOLED, "Task OLED", 4096, NULL, 1, &TaskOLED, 0);
}

// === Task DHT ===
void TaskReadDHT(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t))
      Serial.println("Errore lettura DHT11");

    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

// === Task MPU ===
void TaskReadMPU(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

// === Task LED Blink + tempo ===
void TaskBlinkLED(void *pvParameters) {
  (void) pvParameters;
  uint32_t lastRun = micros();

  for (;;) {
    uint32_t now = micros();
    uint32_t delta = now - lastRun;
    lastRun = now;

    blinkIntervalMs = delta / 1000.0;  // aggiorna la variabile condivisa (ms)

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.printf("Blink Task - Tempo tra esecuzioni: %.3f ms\n", blinkIntervalMs);

    vTaskDelay(1 / portTICK_PERIOD_MS);  // ogni 10 sec
  }
}

// === Task OLED ===
void TaskDisplayOLED(void *pvParameters) {
  (void) pvParameters;
  char buffer[32];

  for (;;) {
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 15, "ESP32 FreeRTOS");
    u8g2.drawStr(0, 30, "Blink Interval:");

    snprintf(buffer, sizeof(buffer), "%.3f ms", blinkIntervalMs);
    u8g2.drawStr(0, 45, buffer);

    u8g2.sendBuffer();

    vTaskDelay(0.001 / portTICK_PERIOD_MS);  // aggiorna ogni 1 microsecondo
  }
}

void loop() {
  // Tutto gestito da task
}
//*/

/*
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <MPU6050.h>
#include <U8g2lib.h>

// ==== DHT11 Setup ====
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ==== MPU6050 Setup ====
MPU6050 mpu;

// ==== OLED SH1106 Setup ====
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ==== LED ====
#define LED_BUILTIN 2

// ==== Timing ====
unsigned long lastDHTTime = 0;
unsigned long lastMPUTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastOLEDTime = 0;
unsigned long blinkInterval = 1;  // 10 microsecondo in ms
float lastBlinkDuration = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Inizializzazione
  dht.begin();
  Wire.begin(21, 22);
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 non connesso!");
  } else {
    Serial.println("MPU6050 connesso!");
  }

  u8g2.begin();
}

void loop() {
  unsigned long now = millis();

  // === Lettura DHT ogni 2s ===
  if (now - lastDHTTime >= 1) {
    lastDHTTime = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("Errore lettura DHT11");
    }
  }

  // === Lettura MPU ogni 0.5s ===
  if (now - lastMPUTime >= 1) {
    lastMPUTime = now;
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  }

  // === Blink LED ogni 1us e calcolo intervallo ===
  if (now - lastBlinkTime >= blinkInterval) {
    unsigned long currentTime = micros();
    static unsigned long previousBlinkMicros = 0;

    lastBlinkTime = now;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    // Calcolo intervallo in millisecondi
    lastBlinkDuration = (currentTime - previousBlinkMicros) / 1000.0;
    previousBlinkMicros = currentTime;

    Serial.printf("Blink - Intervallo: %.3f ms\n", lastBlinkDuration);
  }

  // === Display OLED ogni 1ms ===
  if (now - lastOLEDTime >= 1) {
    lastOLEDTime = now;

    char buffer[32];
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    u8g2.drawStr(0, 15, "ESP32 - NO RTOS");
    u8g2.drawStr(0, 30, "Blink Interval:");

    snprintf(buffer, sizeof(buffer), "%.3f ms", lastBlinkDuration);
    u8g2.drawStr(0, 45, buffer);

    u8g2.sendBuffer();
  }
}
//*/
