#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "HX711.h"

/* ========== WIFI CREDENTIALS ========== */
const char* ssid = "Realme 9 Pro +";
const char* password = "12345678";

/* ========== SUPABASE DETAILS ========== */
const char* cartUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart";

const char* sessionUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart_session?select=status&order=created_at.desc&limit=1";


const char* validationUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/validation";

const char* supabaseKey =
  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImthdGhjd2p4ZGtsYmRjdWV3aWl3Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjczMzY1ODUsImV4cCI6MjA4MjkxMjU4NX0.mcgeqyyh2SHwnyddcieiZ1__whio8H_wG_uiHKnPgGQ";

/* ========== HX711 PINS ========== */
#define LOADCELL_DOUT_PIN D6
#define LOADCELL_SCK_PIN  D7

HX711 scale;

/* ========== BUZZER ========== */
#define BUZZER_PIN D5

/* ========== LOAD CELL CALIBRATION ========== */
float scale_factor = 398.856;   // calibrated value
long offset = 0;

/* ========== VALIDATION SETTINGS ========== */
float tolerancePercent = 5.0;   // 🔥 5% tolerance
float expectedTotalWeight = 0;

/* ========== STATE MANAGEMENT ========== */
bool cartFetched = false;
bool lastValidationState = true;   // prevents spam

/* ========== FUNCTION DECLARATIONS ========== */
void connectToWiFi();
void setClock();
void fetchExpectedWeight();
void checkWeight();
void buzzerAlert();
void sendValidationResult(bool verified, int diffValue);
float parseWeight(String weightStr);

/* ========== SETUP ========== */
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("\n🚀 Starting Smart Cart System");

  connectToWiFi();
  setClock();

  /* HX711 INIT */
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(128);

  Serial.println("⚖️ Taring scale (NO LOAD)");
  delay(2000);
  offset = scale.read_average(20);

  Serial.print("Offset = ");
  Serial.println(offset);

}

/* ========== LOOP ========== */
void loop() {

  if (!cartFetched) {
    if (isSessionFinalized()) {
      Serial.println("🟢 Cart finalized. Fetching expected weight...");
      fetchExpectedWeight();
    }
  } 
  else {
    checkWeight();
  }

  delay(1000);
}

/* ========== WIFI CONNECTION ========== */
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

bool isSessionFinalized() {

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
  deserializeJson(doc, payload);

  if (doc.size() == 0) return false;

  return String(doc[0]["status"]) == "finalized";
}


/* ========== TIME SYNC (HTTPS REQUIRED) ========== */
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("⏱ Syncing time");
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n⏱ Time synchronized");
}

/* ========== PARSE WEIGHT STRING ========== */
float parseWeight(String weightStr) {
  weightStr.toLowerCase();
  weightStr.replace("ml", "");
  weightStr.replace("g", "");
  weightStr.trim();
  return weightStr.toFloat();
}

/* ========== FETCH & SUM EXPECTED WEIGHT ========== */
void fetchExpectedWeight() {

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, cartUrl);

  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));

  Serial.println("\n📡 Fetching cart from Supabase...");

  int httpCode = http.GET();

  if (httpCode != 200) {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
    http.end();
    return;
  }

  String payload = http.getString();

  StaticJsonDocument<4096> doc;
  if (deserializeJson(doc, payload)) {
    Serial.println("❌ JSON parse failed");
    http.end();
    return;
  }

  expectedTotalWeight = 0;

  Serial.println("\n🧾 CART ITEMS:");
  for (JsonObject item : doc.as<JsonArray>()) {
    float weight = parseWeight(item["weight"].as<String>());
    int qty = item["qty"] | 1;
    expectedTotalWeight += weight * qty;

    Serial.print("Item: ");
    Serial.print(weight);
    Serial.print(" g x ");
    Serial.println(qty);
  }

  Serial.print("\nExpected TOTAL Weight = ");
  Serial.print(expectedTotalWeight);
  Serial.println(" g");

  cartFetched = true;
  http.end();
}



/* ========== READ & VALIDATE WEIGHT ========== */
void checkWeight() {

  long raw = scale.read_average(10);
  float actualWeight = (raw - offset) / scale_factor;
  if (actualWeight < 0) actualWeight = 0;

  float diff = abs(actualWeight - expectedTotalWeight);
  float allowedDiff = (tolerancePercent / 100.0) * expectedTotalWeight;

  Serial.print("\nActual Weight = ");
  Serial.print(actualWeight, 1);
  Serial.print(" g | Diff = ");
  Serial.print(diff, 1);
  Serial.print(" g | Allowed = ");
  Serial.print(allowedDiff, 1);
  Serial.println(" g");

  bool currentState = diff <= allowedDiff;

  if (!currentState) {
    buzzerAlert();
  }

  /* 🔥 SEND ONLY IF STATE CHANGED */
  if (currentState != lastValidationState) {
    Serial.println("📤 Sending validation update to Supabase");
    sendValidationResult(currentState, (int)diff);
    lastValidationState = currentState;
  }
}

/* ========== SEND VALIDATION RESULT ========== */
void sendValidationResult(bool verified, int diffValue) {

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, validationUrl);

  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["verified"] = verified;
  doc["diff"] = diffValue;

  String payload;
  serializeJson(doc, payload);

  int httpCode = http.POST(payload);
  Serial.print("Validation POST status: ");
  Serial.println(httpCode);

  http.end();
}

/* ========== BUZZER ALERT ========== */
void buzzerAlert() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1500);
  digitalWrite(BUZZER_PIN, LOW);
}