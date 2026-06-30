/*
  HX711 - calibrazione guidata (no EEPROM)
  DT -> pin 3
  SCK-> pin 2
  Calibrazione con peso noto: 275 g
  Questo sketch non dipende da set_scale/set_offset/get_units della libreria:
  calcola offset e fattore di scala internamente usando letture raw.
*/

#include "HX711.h"

#define DOUT_PIN 3
#define SCK_PIN  2

HX711 scale;             // oggetto HX711 corretto

const int avgSamples = 16;     // numero di letture per media
const int calibWeightGrams = 275; // peso di calibrazione in grammi (modificato a 275 g)
float calibrationFactor = 1.0; // counts per gram (verrà calcolato)
long offsetBaseline = 0;       // raw a vuoto (tare)

void waitForEnter(const char *msg) {
  Serial.println(msg);
  while (!Serial.available()) {
    delay(10);
  }
  // svuota il buffer
  while (Serial.available()) Serial.read();
  delay(200);
}

// Legge una media di "samples" letture raw dallo HX711
long readAverageRaw(int samples) {
  long long sum = 0;
  for (int i = 0; i < samples; ++i) {
    // aspetta che il convertitore sia pronto, se disponibile
    unsigned long tstart = millis();
    while (!scale.is_ready() && (millis() - tstart) < 2000) {
      delay(1);
    }
    long val = scale.read(); // metodo read() disponibile nella maggior parte delle librerie
    sum += val;
    delay(10);
  }
  return (long)(sum / samples);
}

void performCalibration() {
  Serial.println();
  Serial.println("=== CALIBRAZIONE ===");
  char buf[120];
  snprintf(buf, sizeof(buf), "1) Rimuovere qualsiasi peso dalla cella e premere INVIO per misurare il vuoto.");
  waitForEnter(buf);
  offsetBaseline = readAverageRaw(avgSamples);
  Serial.print("Valore medio a vuoto (raw): ");
  Serial.println(offsetBaseline);

  snprintf(buf, sizeof(buf), "2) Posizionare un peso noto di %d g sulla cella e premere INVIO.", calibWeightGrams);
  waitForEnter(buf);
  long rawWith = readAverageRaw(avgSamples);
  Serial.print("Valore medio con ");
  Serial.print(calibWeightGrams);
  Serial.print(" g (raw): ");
  Serial.println(rawWith);

  long diff = rawWith - offsetBaseline;
  if (diff == 0) {
    Serial.println("Errore: nessuna differenza rilevata tra vuoto e carico. Controllare collegamenti.");
    calibrationFactor = 1.0;
  } else {
    // counts per gram (counts per 1 g) -> dividiamo per il peso di calibrazione
    calibrationFactor = (float)diff / (float)calibWeightGrams;
  }

  // Se le letture risultano con segno invertito, correggiamo:
  long rawTest = readAverageRaw(5);
  float testWeight = (rawTest - offsetBaseline) / (calibrationFactor == 0 ? 1.0f : calibrationFactor);
  if (testWeight < 0) {
    calibrationFactor = -calibrationFactor;
    testWeight = -testWeight;
  }

  Serial.print("Fattore di scala (counts per gram): ");
  Serial.println(calibrationFactor, 6);
  Serial.print("Verifica lettura (dovrebbe essere circa ");
  Serial.print(calibWeightGrams);
  Serial.print(" g): ");
  Serial.println(testWeight, 2);
  Serial.println("=== FINE CALIBRAZIONE ===");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* attesa se necessario */ }

  Serial.println();
  Serial.print("HX711 - calibrazione guidata per cella (senza EEPROM) - calibrazione a ");
  Serial.print(calibWeightGrams);
  Serial.println(" g");
  Serial.print("DT pin: "); Serial.print(DOUT_PIN);
  Serial.print("   SCK pin: "); Serial.println(SCK_PIN);
  Serial.println();

  // inizializza HX711
  scale.begin(DOUT_PIN, SCK_PIN);
  // se la tua libreria ha set_gain, puoi abilitare: scale.set_gain(128);

  // breve attesa e check
  unsigned long tstart = millis();
  while (!scale.is_ready() && (millis() - tstart) < 2000) {
    delay(10);
  }

  Serial.println("Pronto. Avviando calibrazione interattiva...");
  performCalibration();

  Serial.println();
  Serial.println("Calibrazione completata. Nessun salvataggio in EEPROM.");
  Serial.println("Stampa peso (g) ogni secondo.");
  Serial.println();
}

void loop() {
  long raw = readAverageRaw(10);
  float weight_g = (raw - offsetBaseline) / (calibrationFactor == 0 ? 1.0f : calibrationFactor);
  Serial.print("Peso: ");
  Serial.print(weight_g, 2);
  Serial.println(" g");
  delay(1000);
}
