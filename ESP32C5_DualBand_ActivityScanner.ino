/*
 * ESP32-C5 Dual Band Activity Scanner
 *
 * This sketch scans for WiFi networks on both 2.4GHz and 5GHz bands
 * and prints the results to the Serial Monitor.
 *
 * REQUIREMENTS:
 * - ESP32-C5 development board
 * - Arduino IDE with ESP32 board support package installed (version that supports C5)
 * - Select "ESP32C5 Dev Module" (or similar) as the board
 *
 * NOTE:
 * The ESP32-C5 supports dual-band WiFi. The standard WiFi.scanNetworks()
 * function should scan all supported channels defined by the board's
 * regulatory configuration.
 */

#include "WiFi.h"

void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done. Starting Dual Band Activity Scanner...");
}

void loop() {
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  // It scans all channels available to the device configuration.
  // Note: Scanning 5GHz channels might take longer.
  int n = WiFi.scanNetworks();

  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");

    // Print header
    Serial.printf("%-32s | %-4s | %-7s | %-6s | %s\n", "SSID", "RSSI", "Channel", "Band", "Encrypt");
    Serial.println("--------------------------------------------------------------------------------------");

    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      int32_t rssi = WiFi.RSSI(i);
      int32_t channel = WiFi.channel(i);
      String encryption = getEncryptionType(WiFi.encryptionType(i));

      // Determine Band based on channel
      // 2.4GHz channels are typically 1-14
      // 5GHz channels are typically 36 and above (e.g., 36, 40, ..., 165)
      String band = (channel > 14) ? "5GHz" : "2.4GHz";

      // Print details
      Serial.printf("%-32s | %-4d | %-7d | %-6s | %s\n",
                    ssid.c_str(), rssi, channel, band.c_str(), encryption.c_str());

      delay(10);
    }
  }
  Serial.println("");

  // Wait a bit before scanning again
  delay(5000);
}

String getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return "OPEN";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2_ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3_PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2_WPA3_PSK";
    case WIFI_AUTH_WAPI_PSK:
      return "WAPI_PSK";
    default:
      return "UNKNOWN";
  }
}
