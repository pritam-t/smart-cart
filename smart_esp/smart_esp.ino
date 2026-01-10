#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

/* ========== WIFI CREDENTIALS ========== */
const char* ssid = "Realme 9 Pro +";
const char* password = "12345678";

/* ========== SUPABASE DETAILS ========== */
// Example:
// https://abcd1234.supabase.co/rest/v1/cart
const char* supabaseUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart";

// Use ONLY the public anon key
const char* supabaseKey =
  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImthdGhjd2p4ZGtsYmRjdWV3aWl3Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjczMzY1ODUsImV4cCI6MjA4MjkxMjU4NX0.mcgeqyyh2SHwnyddcieiZ1__whio8H_wG_uiHKnPgGQ";

/* ========== FUNCTION DECLARATIONS ========== */
void connectToWiFi();
void setClock();
void fetchAndPrintCart();

/* ========== SETUP ========== */
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nStarting NodeMCU...");

  connectToWiFi();   // Connect to WiFi
  setClock();        // 🔥 REQUIRED for HTTPS
  fetchAndPrintCart();
}

/* ========== LOOP ========== */
void loop() {
  // Fetch cart every 10 seconds
  delay(10000);
  fetchAndPrintCart();
}

/* ========== WIFI CONNECTION ========== */
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

/* ========== SET SYSTEM TIME (CRITICAL) ========== */
void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync");
  time_t now = time(nullptr);

  while (now < 100000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println("\nTime synchronized");
}

/* ========== FETCH & PRINT CART DATA ========== */
void fetchAndPrintCart() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();   // Skip certificate validation

  HTTPClient http;
  http.setTimeout(10000); // 10 seconds timeout

  Serial.println("\nConnecting to Supabase...");
  http.begin(client, supabaseUrl);

  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", String("Bearer ") + supabaseKey);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();

  if (httpCode < 0) {
    Serial.print("HTTP request failed: ");
    Serial.println(http.errorToString(httpCode));
    http.end();
    return;
  }

  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  String payload = http.getString();
  Serial.println("\n===== RAW JSON FROM SUPABASE =====");
  Serial.println(payload);

  StaticJsonDocument<4096> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    http.end();
    return;
  }

  Serial.println("\n===== CART DATA =====");

  for (JsonObject item : doc.as<JsonArray>()) {
    Serial.print("Barcode : ");
    Serial.println(item["barcode"].as<String>());

    Serial.print("Name    : ");
    Serial.println(item["name"].as<String>());

    Serial.print("Price   : ");
    Serial.println(item["price"].as<int>());

    Serial.print("Qty     : ");
    Serial.println(item["qty"].as<int>());

    Serial.print("Source  : ");
    Serial.println(item["source"].as<String>());

    Serial.println("-----------------------------");
  }

  http.end();
}
