/*
 * Alexa Voice Control for ESP32 / ESP8266 (FauxmoESP)
 *
 * This sketch allows you to control a light (or any device connected to a GPIO)
 * using Amazon Alexa voice commands. It emulates a Belkin WeMo device.
 *
 * HARDWARE:
 * - ESP32 or ESP8266 Development Board
 * - LED or Relay Module connected to the defined RELAY_PIN
 *
 * REQUIRED LIBRARIES (Install via Arduino Library Manager):
 * 1. "FauxmoESP" by Xose Pérez (v3.1.0 or higher)
 * 2. "AsyncTCP" by Me-No-Dev (for ESP32) OR "ESPAsyncTCP" (for ESP8266)
 *
 * INSTRUCTIONS:
 * 1. Update WIFI_SSID and WIFI_PASS with your credentials.
 * 2. Upload the code.
 * 3. Say "Alexa, discover devices".
 * 4. Say "Alexa, turn on Light".
 */

#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include "fauxmoESP.h"

// ----------------------------------------------------------------------------
// CONFIGURATION
// ----------------------------------------------------------------------------
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

#define DEVICE_NAME "Light"
#define RELAY_PIN   2  // GPIO 2 is usually the Built-in LED on ESP32

fauxmoESP fauxmo;

// ----------------------------------------------------------------------------
// SETUP
// ----------------------------------------------------------------------------
void wifiSetup() {
  WiFi.mode(WIFI_STA);
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n",
                WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // LED / Relay setup
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Start OFF

  // WiFi
  wifiSetup();

  // Fauxmo setup
  fauxmo.createServer(true); // Create the webserver
  fauxmo.setPort(80);        // Required for Gen3 devices
  fauxmo.enable(true);       // Enable the library

  // Add virtual device
  fauxmo.addDevice(DEVICE_NAME);

  // Define callback for Alexa commands
  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

    if (strcmp(device_name, DEVICE_NAME) == 0) {
      digitalWrite(RELAY_PIN, state ? HIGH : LOW);
    }
  });

  Serial.println("Ready! Ask Alexa to discover devices.");
}

void loop() {
  // fauxmoESP uses an async TCP server but a sync UDP server
  // Therefore, we have to manually poll for UDP packets
  fauxmo.handle();

  static unsigned long last = millis();
  if (millis() - last > 5000) {
    last = millis();
    // Heartbeat to serial
    // Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
  }
}
