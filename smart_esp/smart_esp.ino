#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Realme 9 Pro +";
const char* password = "12345678";
  
// Supabase details
const char* supabaseUrl =
  "https://kathcwjxdklbdcuewiiw.supabase.co/rest/v1/cart";
const char* supabaseKey =
  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImthdGhjd2p4ZGtsYmRjdWV3aWl3Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjczMzY1ODUsImV4cCI6MjA4MjkxMjU4NX0.mcgeqyyh2SHwnyddcieiZ1__whio8H_wG_uiHKnPgGQ";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  fetchAndPrintCart();
}

void loop() {
  // optional: refresh every 10 seconds
  delay(10000);
  fetchAndPrintCart();
}

void fetchAndPrintCart() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  HTTPClient http;
  http.begin(supabaseUrl);

  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", String("Bearer ") + supabaseKey);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Received JSON:");
    Serial.println(payload);

    // Parse JSON
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.println("JSON parse failed");
      return;
    }

    Serial.println("\n--- CART DATA ---");

    for (JsonObject item : doc.as<JsonArray>()) {
      Serial.print("Barcode: ");
      Serial.println(item["barcode"].as<String>());

      Serial.print("Name: ");
      Serial.println(item["name"].as<String>());

      Serial.print("Price: ");
      Serial.println(item["price"].as<int>());

      Serial.print("Qty: ");
      Serial.println(item["qty"].as<int>());

      Serial.print("Source: ");
      Serial.println(item["source"].as<String>());

      Serial.println("----------------");
    }

  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
  }

  http.end();
}
