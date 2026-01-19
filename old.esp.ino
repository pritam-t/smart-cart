#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "HX711.h"

/* ================= WIFI ================= */
const char* ssid = "Realme 9 Pro +";
const char* password = "12345678";

/* ================= SUPABASE ================= */
const char* cartUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart";

const char* sessionUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart_session?select=status&order=created_at.desc&limit=1";

const char* validationUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/validation";

const char* supabaseKey =
  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImthdGhjd2p4ZGtsYmRjdWV3aWl3Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjczMzY1ODUsImV4cCI6MjA4MjkxMjU4NX0.mcgeqyyh2SHwnyddcieiZ1__whio8H_wG_uiHKnPgGQ";

/* ================= HX711 ================= */
#define LOADCELL_DOUT_PIN D6
#define LOADCELL_SCK_PIN  D7
HX711 scale;

/* ================= BUZZER ================= */
#define BUZZER_PIN D5

/* ================= CALIBRATION ================= */
float scale_factor = 398.856;
long offset = 0;

/* ================= LOGIC ================= */
float tolerancePercent = 5.0;
float expectedTotalWeight = 0;

bool cartFetched = false;
bool fetchingInProgress = false;
bool lastValidationState = true;

bool sessionChecked = false;


/* ================= UTILS ================= */
float parseWeight(String w) {
  w.toLowerCase();
  w.replace("ml", "");
  w.replace("g", "");
  w.trim();
  return w.toFloat();
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("\n🚀 Starting Smart Cart System");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    yield();
  }
  Serial.println("\n✅ WiFi Connected");

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000) {
    delay(500);
    yield();
  }

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(128);
  Serial.println("⚖️ Taring scale (NO LOAD)");
  delay(2000);
  offset = scale.read_average(20);

  Serial.print("Offset = ");
  Serial.println(offset);
}

/* ================= LOOP ================= */
void loop() {

  if (!sessionChecked) {
    sessionChecked = checkIfSessionFinalized();
    delay(2000);   // 🔥 VERY IMPORTANT
    return;
  }

  if (sessionChecked && !cartFetched) {
    fetchExpectedWeight();
    cartFetched = true;
    delay(2000);   // 🔥 VERY IMPORTANT
    return;
  }

  if (cartFetched) {
    checkWeight();     // NO HTTP HERE
    delay(1000);
  }
}


/* ================= SESSION CHECK ================= */
bool checkIfSessionFinalized() {

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  http.begin(client, sessionUrl);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));

  int code = http.GET();

  Serial.print("Session HTTP code: ");
  Serial.println(code);

  if (code != 200) {
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  Serial.println(payload);

  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload);

  if (doc.size() == 0) return false;

  return String(doc[0]["status"]) == "finalized";
}

/* ================= FETCH CART ================= */
void fetchExpectedWeight() {

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  http.begin(client, cartUrl);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));

  int code = http.GET();
  yield();

  if (code != 200) {
    Serial.println("❌ Failed to fetch cart");
    fetchingInProgress = false;
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();
  yield();

  StaticJsonDocument<2048> doc;
  if (deserializeJson(doc, payload)) {
    Serial.println("❌ JSON parse error");
    fetchingInProgress = false;
    return;
  }

  expectedTotalWeight = 0;

  for (JsonObject item : doc.as<JsonArray>()) {
    yield();
    float w = parseWeight(item["weight"].as<String>());
    int q = item["qty"] | 1;
    expectedTotalWeight += w * q;
  }

  Serial.print("Expected TOTAL weight = ");
  Serial.print(expectedTotalWeight);
  Serial.println(" g");

  cartFetched = true;
  fetchingInProgress = false;
}

/* ================= WEIGHT CHECK ================= */
void checkWeight() {

  long raw = scale.read_average(10);
  yield();

  float actual = (raw - offset) / scale_factor;
  if (actual < 0) actual = 0;

  float diff = abs(actual - expectedTotalWeight);
  float allowed = (tolerancePercent / 100.0) * expectedTotalWeight;
  bool valid = diff <= allowed;

  if (!valid) buzzerAlert();

  if (valid != lastValidationState) {
    sendValidationResult(valid, (int)diff);
    lastValidationState = valid;
  }
}

/* ================= SEND VALIDATION ================= */
void sendValidationResult(bool verified, int diff) {

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  http.begin(client, validationUrl);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + String(supabaseKey));
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["verified"] = verified;
  doc["diff"] = diff;

  String payload;
  serializeJson(doc, payload);

  http.POST(payload);
  http.end();
}

/* ================= BUZZER ================= */
void buzzerAlert() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
}
