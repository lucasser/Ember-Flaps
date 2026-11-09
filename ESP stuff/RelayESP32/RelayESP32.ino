#include <WiFi.h>
#include <WebServer.h>
#include "web_assets.h"

// Change before uploading. Your phone will connect directly to this Wi-Fi.
const char* AP_NAME = "Relay-ESP32";
const char* AP_PASSWORD = "ChangeThis123!";  // 8+ characters; choose your own

// Safe first test: GPIO23 -> 330 ohm resistor -> LED -> GND.
// NEVER connect motor power directly to this pin.
const uint8_t TEST_OUTPUT_PIN = 23;
WebServer server(80);
bool outputOn = false;

struct Event { String label; String detail; String when; };
Event history[8];
uint8_t historyCount = 0;

String timeLabel() {
  unsigned long s = millis() / 1000UL;
  char buf[22];
  snprintf(buf, sizeof(buf), "uptime %02lu:%02lu:%02lu", s / 3600UL,
           (s / 60UL) % 60UL, s % 60UL);
  return String(buf);
}

void recordEvent(const String& label, const String& detail) {
  for (int i = 7; i > 0; i--) history[i] = history[i - 1];
  history[0] = {label, detail, timeLabel()};
  if (historyCount < 8) historyCount++;
}

// Event strings are all firmware-controlled (no arbitrary user text).
String statusJSON() {
  String result;
  result.reserve(1300);
  result = "{\"mode\":\"esp32\",\"connected\":true,\"motor_on\":";
  result += outputOn ? "true" : "false";
  result += ",\"last_response\":\"" + timeLabel() + "\",\"activity\":[";
  for (uint8_t i = 0; i < historyCount; i++) {
    if (i) result += ',';
    result += "{\"time\":\"" + history[i].when + "\",\"label\":\"";
    result += history[i].label + "\",\"detail\":\"";
    result += history[i].detail + "\"}";
  }
  result += "]}";
  return result;
}

void serveAsset(const char* data, const char* contentType) {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, contentType, data);
}

void handleOutput() {
  // Only accept JSON from same-origin application; no CORS headers are set.
  if (!server.hasHeader("Content-Type") ||
      !server.header("Content-Type").startsWith("application/json")) {
    server.send(415, "application/json", "{\"error\":\"JSON required\"}");
    return;
  }
  String body = server.arg("plain");
  body.replace(" ", ""); body.replace("\n", ""); body.replace("\r", "");
  body.replace("\t", "");
  bool desired;
  if (body == "{\"on\":true}") desired = true;
  else if (body == "{\"on\":false}") desired = false;
  else {
    server.send(400, "application/json", "{\"error\":\"Expected {\\\"on\\\":true/false}\"}");
    return;
  }
  outputOn = desired;
  digitalWrite(TEST_OUTPUT_PIN, desired ? HIGH : LOW);
  recordEvent(desired ? "Output turned on" : "Output turned off",
              "ESP32 GPIO output state updated (not motor feedback)");
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", statusJSON());
}

void setup() {
  Serial.begin(115200);
  pinMode(TEST_OUTPUT_PIN, OUTPUT);
  digitalWrite(TEST_OUTPUT_PIN, LOW);  // Start in known OFF state
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(AP_NAME, AP_PASSWORD)) {
    Serial.println("Failed to start ESP32 Wi-Fi access point");
    return;
  }
  Serial.print("Wi-Fi: "); Serial.println(AP_NAME);
  Serial.print("Open http://"); Serial.println(WiFi.softAPIP());
  static const char* headers[] = {"Content-Type"};
  server.collectHeaders(headers, 1);
  server.on("/", HTTP_GET, []() { serveAsset(HTML_DATA, "text/html; charset=utf-8"); });
  server.on("/style.css", HTTP_GET, []() { serveAsset(CSS_DATA, "text/css; charset=utf-8"); });
  server.on("/script.js", HTTP_GET, []() { serveAsset(JS_DATA, "application/javascript; charset=utf-8"); });
  server.on("/api/status", HTTP_GET, []() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", statusJSON());
  });
  server.on("/api/motor", HTTP_POST, handleOutput);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  recordEvent("Device started", "ESP32 web server ready");
  server.begin();
}

void loop() {
  server.handleClient();
  delay(2);
}
