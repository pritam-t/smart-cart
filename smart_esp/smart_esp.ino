#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "HX711.h"

/* ========== WIFI ========== */
const char* ssid = "Realme 9 Pro +";
const char* password = "12345678";

/* ========== SUPABASE ========== */
const char* sessionUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart_session"
  "?select=status&order=created_at.desc&limit=1";

const char* cartUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart"
  "?select=weight&limit=10";

const char* supabaseKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImthdGhjd2p4ZGtsYmRjdWV3aWl3Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjczMzY1ODUsImV4cCI6MjA4MjkxMjU4NX0.mcgeqyyh2SHwnyddcieiZ1__whio8H_wG_uiHKnPgGQ";

/* ========== HX711 ========== */
#define LOADCELL_DOUT_PIN D6
#define LOADCELL_SCK_PIN  D7

HX711 scale;
float scale_factor = 388.3;
long offset = 0;

/* ========== CONTROL ========== */
bool finalized = false;
bool cartFetched = false;

float lastMeasuredWeight = 0;
float cartWeightSum = 0;

/* ========== TIMERS ========== */
unsigned long lastWeightRead   = 0;
unsigned long lastSessionCheck = 0;

const unsigned long WEIGHT_READ_INTERVAL   = 500;
const unsigned long SESSION_CHECK_INTERVAL = 8000;

/* ========== SETUP ========== */
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n🚀 Smart Cart System (millis-based)");

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(128);

  Serial.println("Taring scale...");
  delay(2000);
  offset = scale.read_average(20);
  Serial.println("Ready");
}

/* ========== LOOP ========== */
void loop() {

  unsigned long now = millis();

  /* 1️⃣ HX711 weight reading (every 500 ms) */
  if (!finalized && (now - lastWeightRead >= WEIGHT_READ_INTERVAL)) {
    lastWeightRead = now;

    long raw = scale.read_average(10);
    float weight = (raw - offset) / scale_factor;
    if (weight < 0) weight = 0;

    lastMeasuredWeight = weight;

    Serial.print("Live Weight: ");
    Serial.print(weight, 1);
    Serial.println(" g");
  }

  /* 2️⃣ Session check (every 8 seconds) */
  if (!finalized && (now - lastSessionCheck >= SESSION_CHECK_INTERVAL)) {
    lastSessionCheck = now;

    Serial.println("🔍 Checking session...");
    finalized = checkSessionFinalized();

    if (finalized) {
      Serial.println("🟢 Session finalized");
    } else {
      Serial.println("🟡 Not finalized");
    }
  }

  /* 3️⃣ Final action (ONLY ONCE) */
  if (finalized && !cartFetched) {

    fetchCartWeightSum();

    Serial.println("\n🛑 LIVE WEIGHT STOPPED");
    Serial.print("Last HX711 Weight: ");
    Serial.print(lastMeasuredWeight, 1);
    Serial.println(" g");

    Serial.print("Cart Weight Sum: ");
    Serial.print(cartWeightSum, 1);
    Serial.println(" g");

    Serial.println("Posting validation to Supabase...");
    postValidation(lastMeasuredWeight, cartWeightSum);  // <-- NEW

    cartFetched = true;   // 🔒 never run again
  }

  yield();  // ESP8266 safety
}

/* ========== SESSION CHECK ========== */
bool checkSessionFinalized() {

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, sessionUrl);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));

  int code = http.GET();
  if (code != 200) {
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, payload)) return false;
  if (doc.size() == 0) return false;

  return String(doc[0]["status"]) == "finalized";
}

/* ========== CART WEIGHT SUM ========== */
float parseWeight(String w) {
  w.toLowerCase();
  w.replace("kg", "000");
  w.replace("g", "");
  w.replace("ml", "");
  w.trim();
  return w.toFloat();
}

void fetchCartWeightSum() {

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, cartUrl);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));

  int code = http.GET();
  if (code != 200) {
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, payload)) return;

  cartWeightSum = 0;

  for (JsonObject item : doc.as<JsonArray>()) {
    yield();
    cartWeightSum += parseWeight(item["weight"].as<String>());
  }
}

void postValidation(float liveWeight, float cartWeight) {

  bool verified = false;
  int diff = 0;

  diff = abs(cartWeight - liveWeight);

  if (diff <= 10) {
    verified = true;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client,
    "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/validation");

  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=minimal"); // small response

  // Build JSON
  StaticJsonDocument<200> doc;
  doc["verified"] = verified;
  doc["diff"] = diff;

  String payload;
  serializeJson(doc, payload);

  int code = http.POST(payload);

  Serial.print("Validation POST code: ");
  Serial.println(code);

  http.end();
}

