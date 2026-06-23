#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ============================================================
// 1. WiFi VA SUPABASE SOZLAMALARI
// ============================================================
const char* ssid     = "wi id";
const char* password = "wi pas";

const String SUPABASE_URL = "your url";
const String SUPABASE_KEY = "your key";

// ============================================================
// 2. PIN KONFIGURATSIYASI
// ============================================================
#define DHTPIN      4
#define DHTTYPE     DHT11
#define SOIL_PIN    34
#define LIGHT_PIN   32
#define GAS_PIN     35
#define FAN_PIN     12
#define PUMP_PIN    14

DHT dht(DHTPIN, DHTTYPE);

// ============================================================
// 3. GLOBAL O'ZGARUVCHILAR
// ============================================================
float max_temp          = 30.0;
int   min_soil_moisture = 40;
float min_air_humidity  = 50.0;
float max_gas           = 300.0;
bool  cooler_status     = false;
bool  pump_status       = false;
bool  is_automated      = true;

unsigned long lastTime = 0;
const unsigned long DELAY = 15000;

// ============================================================
// 4. YORDAMCHI: HTTP headerlar
// ============================================================
void addHeaders(HTTPClient &http, bool withContentType = false) {
  http.addHeader("apikey",        SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + SUPABASE_KEY);
  if (withContentType)
    http.addHeader("Content-Type", "application/json");
}

// ============================================================
// 5. SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(FAN_PIN,  OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(FAN_PIN,  LOW);
  digitalWrite(PUMP_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("WiFi-ga ulanmoqda");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi ulandi: " + WiFi.localIP().toString());
}

// ============================================================
// 6. ASOSIY SIKL
// ============================================================
void loop() {
  if ((millis() - lastTime < DELAY) || WiFi.status() != WL_CONNECTED) return;
  lastTime = millis();

  // ==========================================================
  // 6.1 — device_settings dan hamma sozlamalarni o'qish
  // ==========================================================
  {
    HTTPClient http;
    // URL oxiriga is_automated ustuni ham qo'shildi
    String url = SUPABASE_URL +
      "/device_settings?id=eq.1"
      "&select=max_temp,min_soil_moisture,min_air_humidity,max_gas,cooler_status,pump_status,is_automated";

    http.begin(url);
    addHeaders(http);

    int code = http.GET();
    if (code == 200) {
      DynamicJsonDocument doc(512);
      deserializeJson(doc, http.getString());

      max_temp          = doc[0]["max_temp"].as<float>();
      min_soil_moisture = doc[0]["min_soil_moisture"].as<int>();
      min_air_humidity  = doc[0]["min_air_humidity"].as<float>();
      max_gas           = doc[0]["max_gas"].as<float>();
      is_automated      = doc[0]["is_automated"].as<bool>(); // Bazadan o'qilmoqda

      // Agar rejim qo'lda (manual) bo'lsa, relay holatlarini ham bazadan oladi
      if (!is_automated) {
        cooler_status = doc[0]["cooler_status"].as<bool>();
        pump_status   = doc[0]["pump_status"].as<bool>();
      }

      Serial.printf(" Sozlamalar: max_temp=%.1f°C | min_soil=%d%% | min_air_hum=%.0f%% | max_gas=%.0fppm | Auto=%s\n",
                    max_temp, min_soil_moisture, min_air_humidity, max_gas, is_automated ? "YONIQ" : "O'CHIQ");
    } else {
      Serial.printf(" device_settings GET xato: %d\n", code);
    }
    http.end();
  }

  // ==========================================================
  // 6.2 — Datchiklarni o'qish
  // ==========================================================
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println(" DHT11 o'qib bo'lmadi! Qayta uriniladi...");
    return;
  }

  int soil_raw      = analogRead(SOIL_PIN);
  int soil_moisture = constrain(map(soil_raw, 4095, 1500, 0, 100), 0, 100);

  int light_raw   = analogRead(LIGHT_PIN);
  int light_level = constrain(map(light_raw, 0, 4095, 0, 100), 0, 100);

  int gas_raw   = analogRead(GAS_PIN);
  int gas_level = map(gas_raw, 0, 4095, 0, 1000);

  Serial.printf("🌡 T=%.1f°C | H=%.0f%% | Tuproq=%d%% | Yorug'lik=%d%% | Gaz=%dppm\n",
                t, h, soil_moisture, light_level, gas_level);

  // ==========================================================
  // 6.3 — Relay mantiq tizimi
  // ==========================================================
  if (is_automated) {
    cooler_status = (t > max_temp) || (gas_level > max_gas);
    pump_status   = (soil_moisture < min_soil_moisture) || (h < min_air_humidity);
  }

  // Jismoniy pinlarni boshqarish
  digitalWrite(FAN_PIN,  cooler_status ? HIGH : LOW);
  digitalWrite(PUMP_PIN, pump_status   ? HIGH : LOW);

  if (pump_status && is_automated) {
    if (soil_moisture < min_soil_moisture && h < min_air_humidity)
      Serial.println(" Nasos: tuproq va havo namligi past bo'lgani uchun yoqildi.");
    else if (soil_moisture < min_soil_moisture)
      Serial.println(" Nasos: tuproq qurib ketgani uchun yoqildi.");
    else
      Serial.println(" Nasos: havo namligi tushib ketgani uchun yoqildi.");
  }

  Serial.printf("🔌 Kuller=%s | Nasos=%s\n",
                cooler_status ? "ON" : "OFF",
                pump_status   ? "ON" : "OFF");

  // ==========================================================
  // 6.4 — sensor_logs ga ma'lumotlarni yuborish (INSERT)
  // ==========================================================
  {
    HTTPClient http;
    http.begin(SUPABASE_URL + "/sensor_logs");
    addHeaders(http, true);
    http.addHeader("Prefer", "return=minimal");

    StaticJsonDocument<256> doc;
    doc["temperature"]   = t;
    doc["humidity"]      = h;
    doc["soil_moisture"] = soil_moisture;
    doc["light_level"]   = light_level;
    doc["gas_level"]     = gas_level;

    String body;
    serializeJson(doc, body);

    int code = http.POST(body);
    if (code >= 200 && code < 300)
      Serial.println(" Datchik ma'lumotlari bazaga yozildi (sensor_logs).");
    else
      Serial.printf(" sensor_logs POST xato: %d\n", code);
    http.end();
  }

  // ==========================================================
  // 6.5 — Faqat Avtomatik rejimda holatni bazaga yozish (PATCH)
  // ==========================================================
  if (is_automated) {
    HTTPClient http;
    http.begin(SUPABASE_URL + "/device_settings?id=eq.1");
    addHeaders(http, true);
    http.addHeader("Prefer", "return=minimal");

    StaticJsonDocument<128> doc;
    doc["cooler_status"] = cooler_status;
    doc["pump_status"]   = pump_status;

    String body;
    serializeJson(doc, body);

    int code = http.PATCH(body);
    if (code >= 200 && code < 300)
      Serial.println(" Relay holatlari bazada yangilandi.");
    else
      Serial.printf(" device_settings PATCH xato: %d\n", code);
    http.end();
  }
}
