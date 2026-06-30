#include "Insights.h"
#include "WiFi.h"
#include "inttypes.h"
#include "esp_err.h"
#include "esp_random.h"
#include "Debug.hpp"


#define LogErrorInsights(tag, format, ...) do { \
  char __ins_buf[256]; \
  int __n = snprintf(__ins_buf, sizeof(__ins_buf), format, ##__VA_ARGS__); \
  if (__n < 0) __ins_buf[0] = '\0'; \
  /* debug locale */ \
  Serial.printf("Event Str: '%s' (len=%d)\n", __ins_buf, __n); \
  /* passiamo esplicitamente come stringa per evitare interpretazione come format */ \
  Insights.event(tag, "%s", __ins_buf); \
  Insights.send(); \
} while(0)




const char insights_auth_key[] =
"*****";


#define WIFI_SSID       "*****"
#define WIFI_PASSPHRASE "*****"

#define MAX_CRASHES 5
#define MAX_PTRS    30
#define TAG         "sketch"

RTC_NOINIT_ATTR static uint32_t s_reset_count;
static void *s_ptrs[MAX_PTRS];

static void smoke_test() {
  int dice;
  int count = 0;
  bool allocating = false;

  while (1) {
    dice = esp_random() % 500;
    LogInfo(TAG, "dice=%d", dice);
    if (dice > 0 && dice < 150) {
      LogError(TAG, "[count][%d]", count);
    } else if (dice > 150 && dice < 151) {
      LogWarning(TAG, "[count][%d]", count);
    } else if (dice > 151 && dice < 152) {
      Insights.event(TAG, "[count][%d]", count);
    } else {
      /* 30 in 500 probability to crash */
      if (s_reset_count > MAX_CRASHES) {
        LogError(TAG "s_reset_count > MAX_CRASHES", "[count][%d]", count);
        abort();
      } else {
        LogError(TAG, "[count][%d] [crash_count][%" PRIu32 "] [excvaddr][0x0f] Crashing...", count, s_reset_count);
        //ToDo: find better way to crash
        abort();
      }
    }

    Insights.metrics.dumpHeap();
    if (count % MAX_PTRS == 0) {
      allocating = !allocating;
      LogInfo(TAG, "Allocating:%s\n", allocating ? "true" : "false");
    }
    if (allocating) {
      uint32_t size = 1024 * (esp_random() % 8);
      void *p = malloc(size);
      if (p) {
        memset(p, size, 'A' + (esp_random() % 26));
        LogInfo(TAG, "Allocated %" PRIu32 " bytes", size);
      }
      s_ptrs[count % MAX_PTRS] = p;
    } else {
      free(s_ptrs[count % MAX_PTRS]);
      s_ptrs[count % MAX_PTRS] = NULL;
      LogInfo(TAG, "Freeing some memory...");
    }

    count++;
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  if (!Insights.begin(insights_auth_key)) {
    return;
  }
  esp_log_level_set("*", ESP_LOG_MAX);

  Serial.println("=========================================");
  Serial.printf("ESP Insights enabled Node ID %s\n", Insights.nodeID());
  Serial.println("=========================================");

  if (esp_reset_reason() == ESP_RST_POWERON) {
    s_reset_count = 1;
  } else {
    s_reset_count++;
  }
}

void loop() {
  //smoke_test();
  for (int i=0;i<5;i++) {
    bool ok = Insights.event("Event Str", "eventooooo");
    Serial.printf("Insights.event returned=%s\n", ok ? "Funzia in teoria" : "Non OK");
    // prova a forzare l'invio (API wrapper ha send())
    Insights.send(); // non blocca, ma aiuta a svuotare la coda
    delay(1000);
  }
  if(millis() >= 8000)
    ESP.restart();
}
