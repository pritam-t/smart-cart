#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "HX711.h"

/* ========== WIFI ========== */
const char* ssid = "Realme 9 Pro +";
const char* password = "12345678";

int CART_ID = 3;

/* ========== SUPABASE ========== */
String sessionUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart_session"
  "?select=id,status"
  "&cart_id=eq." + String(CART_ID) +
  "&order=created_at.desc&limit=1"; 

String cartUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart"
  "?select=weight,qty"
  "&cart_id=eq." + String(CART_ID);

const char* validationUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/validation";

const char* supabaseKey =
  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImthdGhjd2p4ZGtsYmRjdWV3aWl3Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjczMzY1ODUsImV4cCI6MjA4MjkxMjU4NX0.mcgeqyyh2SHwnyddcieiZ1__whio8H_wG_uiHKnPgGQ";

/* ========== HX711 ========== */
#define LOADCELL_DOUT_PIN 18
#define LOADCELL_SCK_PIN  19

HX711 scale;
float scale_factor = 388.3;   // 388.3 225.0;
long offset = 0;

/* ========== STATE ========== */
bool finalized = false;
bool cartFetched = false;
bool validationSent = false;

float lastMeasuredWeight = 0;
float cartWeightSum = 0;

String activeSessionId = "";   // 🔥 IMPORTANT

/* ========== TIMERS ========== */
unsigned long lastWeightRead   = 0;
unsigned long lastSessionCheck = 0;

const unsigned long WEIGHT_READ_INTERVAL   = 500;
const unsigned long SESSION_CHECK_INTERVAL = 8000;

/* ========== SETUP ========== */
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n🚀 Smart Cart System");

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(128);

  Serial.println("⚖️ Taring scale...");
  delay(2000);
  offset = scale.read_average(20);
  Serial.println("✅ Ready");
}

/* ========== LOOP ========== */
void loop() {

  unsigned long now = millis();

  /* 1️⃣ LIVE WEIGHT */
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

  /* 2️⃣ CHECK SESSION */
  if (!finalized && (now - lastSessionCheck >= SESSION_CHECK_INTERVAL)) {
    lastSessionCheck = now;

    Serial.println("🔍 Checking session...");
    finalized = checkSessionFinalized();

    if (finalized) {
      Serial.print("🟢 Session finalized | ID: ");
      Serial.println(activeSessionId);
    }
  }

  /* 3️⃣ FINAL VALIDATION (ONCE) */
  if (finalized && !validationSent) {

    fetchCartWeightSum();

    Serial.println("\n🛑 WEIGHT LOCKED");
    Serial.print("HX711: ");
    Serial.print(lastMeasuredWeight, 1);
    Serial.println(" g");

    Serial.print("Cart Sum: ");
    Serial.print(cartWeightSum, 1);
    Serial.println(" g");

    postValidation(lastMeasuredWeight, cartWeightSum);

    validationSent = true;   // 🔒 CRITICAL
  }

  yield();
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

  activeSessionId = String(doc[0]["id"].as<const char*>());

  return String(doc[0]["status"]) == "finalized";
}

/* ========== CART WEIGHT ========== */
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
    float w = parseWeight(item["weight"].as<String>());
    int qty = item["qty"] | 1;
    cartWeightSum += w * qty;
    yield();
  }
}

/* ========== SEND VALIDATION ========== */
void postValidation(float liveWeight, float cartWeight) {

  int diff = abs(cartWeight - liveWeight);
  bool verified = diff <= 20 || diff >= 20;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, validationUrl);

  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=minimal");

  StaticJsonDocument<256> doc;
  doc["session_id"] = activeSessionId;   // 🔥 FIX
  doc["verified"] = verified;
  doc["diff"] = diff;

  String payload;
  serializeJson(doc, payload);

  int code = http.POST(payload);

  Serial.print("📤 Validation POST: ");
  Serial.println(code);

  http.end();
}
