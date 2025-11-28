/*
 * ESP32-C5 Dual Band Activity Scanner with Display
 *
 * This sketch scans for WiFi networks on both 2.4GHz and 5GHz bands
 * and displays the results on an ILI9341 SPI TFT display.
 *
 * HARDWARE CONNECTIONS (ESP32-C5 to ILI9341):
 * - VCC  -> 3.3V
 * - GND  -> GND
 * - CS   -> GPIO 23
 * - RESET-> GPIO 25
 * - DC   -> GPIO 24
 * - MOSI -> GPIO 8
 * - SCK  -> GPIO 10
 * - LED  -> GPIO 26 (Backlight)
 * - MISO -> Not connected (or standard MISO if needed for read)
 *
 * LIBRARIES REQUIRED:
 * - Arduino_GFX (Install via Library Manager: "GFX Library for Arduino" by Moon On Our Nation)
 *
 * NOTE ON DISPLAY DRIVER:
 * The user requested support for an "ILI9341 4-inch model".
 * Standard ILI9341 displays are usually 2.4", 2.8", or 3.2" with 320x240 resolution.
 * True 4-inch displays often use ILI9488 or ST7796 drivers (480x320).
 * This sketch defaults to ILI9341. If your display looks wrong (white screen, garbage),
 * try uncommenting the ILI9488 or ST7796 constructors below.
 */

#include <WiFi.h>
#include <Arduino_GFX_Library.h>

// ----------------------------------------------------------------------------
// PIN DEFINITIONS
// ----------------------------------------------------------------------------
#define TFT_CS    23
#define TFT_DC    24
#define TFT_RST   25
#define TFT_LED   26
#define TFT_SCK   10
#define TFT_MOSI  8
#define TFT_MISO  GFX_NOT_DEFINED // Use standard or not defined if write-only

// ----------------------------------------------------------------------------
// DISPLAY SETUP
// ----------------------------------------------------------------------------
// Option 1: Create Data Bus (SPI)
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);

// Option 2: Create Display Driver
// Default: ILI9341
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST, 3 /* Rotation: 0-3 */);

// Alternative: ILI9488 (Common for 3.5" - 4.0" displays)
// Arduino_GFX *gfx = new Arduino_ILI9488(bus, TFT_RST, 3);

// Alternative: ST7796 (Common for 4.0" displays)
// Arduino_GFX *gfx = new Arduino_ST7796(bus, TFT_RST, 3);

// ----------------------------------------------------------------------------
// HELPER FUNCTIONS
// ----------------------------------------------------------------------------
String getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK: return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2_ENT";
    case WIFI_AUTH_WPA3_PSK: return "WPA3_PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2_WPA3";
    case WIFI_AUTH_WAPI_PSK: return "WAPI_PSK";
    default: return "UNK";
  }
}

void setup() {
  Serial.begin(115200);

  // Init Backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }

  gfx->fillScreen(BLACK);
  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  gfx->setCursor(0, 0);
  gfx->println("ESP32-C5 Scanner Init...");

  // Init WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done. Starting Dual Band Activity Scanner...");
}

void loop() {
  // 1. Scan
  Serial.println("Scan start...");
  gfx->fillScreen(BLACK);
  gfx->setCursor(0, 0);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->println("Scanning...");

  // Sync scan
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");

  // 2. Display Results
  gfx->fillScreen(BLACK);
  gfx->setCursor(0, 0);

  // Header
  gfx->setTextSize(2);
  gfx->setTextColor(ORANGE);
  gfx->println("WiFi Scanner");
  gfx->drawLine(0, 16, gfx->width(), 16, ORANGE);

  gfx->setTextSize(1);
  gfx->setCursor(0, 20);

  if (n == 0) {
    gfx->setTextColor(RED);
    gfx->println("No networks found.");
  } else {
    // Column Headers
    gfx->setTextColor(LIGHTGREY);
    gfx->println("SSID             RSSI  CH");
    gfx->drawLine(0, 30, gfx->width(), 30, LIGHTGREY);

    // List Networks
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      int32_t rssi = WiFi.RSSI(i);
      int32_t channel = WiFi.channel(i);

      // Determine Band
      bool is5GHz = (channel > 14);

      // Color coding: Green for 2.4G, Cyan for 5G
      if (is5GHz) {
        gfx->setTextColor(CYAN);
      } else {
        gfx->setTextColor(GREEN);
      }

      // Formatting
      // SSID truncated to fit (approx 16 chars for nice fit with RSSI/CH)
      // Display width 320px. Font size 1 is 6px wide. ~53 chars total.
      // Layout: "SSID(20)  RSSI(4) CH(3)"

      String ssidDisplay = ssid;
      if (ssidDisplay.length() > 20) {
        ssidDisplay = ssidDisplay.substring(0, 20);
      }

      // Pad SSID to align RSSI
      while (ssidDisplay.length() < 21) {
        ssidDisplay += " ";
      }

      gfx->print(ssidDisplay);

      // RSSI
      gfx->print(rssi);
      if (rssi > -100) gfx->print(" "); // Padding
      if (rssi > -10) gfx->print(" ");

      gfx->print("  ");

      // Channel
      if (channel < 10) gfx->print(" ");
      if (channel < 100) gfx->print(" ");
      gfx->println(channel);

      // Simple pagination: stop if screen full
      // Font height ~8px. Screen 240px.
      // y start 32. Space left 208. Max lines ~25.
      if (gfx->getCursorY() > gfx->height() - 10) {
        break;
      }
    }
  }

  // Footer status
  gfx->setCursor(0, gfx->height() - 10);
  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  gfx->print("Networks: ");
  gfx->print(n);

  // Wait before next scan
  delay(5000);
}
