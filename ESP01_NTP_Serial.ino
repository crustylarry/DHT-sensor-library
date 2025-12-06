/*
 * ESP-01 (ESP8266) NTP Serial Clock - Eastern Time (EST/EDT)
 *
 * This sketch connects the ESP-01 to WiFi, retrieves the time from an NTP server,
 * and outputs the current date and time via the Serial port (USB).
 *
 * HARDWARE:
 * - ESP-01 (or any ESP8266 module)
 * - USB-to-Serial Adapter (FTDI, CH340, etc.)
 *
 * INSTRUCTIONS:
 * 1. Set your WiFi SSID and Password below.
 * 2. Select "Generic ESP8266 Module" in Arduino IDE.
 * 3. Upload to the ESP-01.
 * 4. Open Serial Monitor at 115200 baud to see the time.
 *
 * TIMEZONE:
 * This sketch is configured for Eastern Time (New York, etc.).
 * It handles Standard Time (EST) and Daylight Saving Time (EDT) automatically.
 */

#include <ESP8266WiFi.h>
#include <time.h>

// ----------------------------------------------------------------------------
// CONFIGURATION
// ----------------------------------------------------------------------------
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Timezone string for Eastern Time (USA/Canada)
// EST = UTC-5, EDT = UTC-4
// DST starts 2nd Sunday in March, ends 1st Sunday in November
const char* TZ_INFO  = "EST5EDT,M3.2.0,M11.1.0";

void setup() {
  // Initialize Serial port
  // ESP-01 transmits on the TX pin, which goes to RX on the USB adapter.
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\nESP-01 NTP Serial Clock (Eastern Time)");

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Configure Time
  // 1. Set timezone using setenv (Standard POSIX way, works on ESP8266)
  setenv("TZ", TZ_INFO, 1);
  tzset();

  // 2. Configure NTP servers
  // We use 0 offsets here because the TZ environment variable handles the shift.
  Serial.println("Initializing NTP...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  // Wait for time to be set
  Serial.print("Waiting for NTP time sync");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nTime synchronized!");
}

void loop() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  // Buffer for formatted string
  char timeString[50];

  // Format: YYYY-MM-DD HH:MM:SS
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeinfo);

  Serial.println(timeString);

  // Wait 1 second before next update
  delay(1000);
}
