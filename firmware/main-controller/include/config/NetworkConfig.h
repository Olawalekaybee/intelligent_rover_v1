#pragma once

// =============================================================================
// Network Configuration — WiFi · ThingSpeak · OTA
//
// Sensitive credentials (SSID, passwords, API keys) stay in Secrets.h.
// All structural / timing / addressing constants are defined here so every
// network-layer number has one authoritative definition.
// =============================================================================

// ── WiFi ──────────────────────────────────────────────────────────────────────
// Time to wait for association before giving up on a single attempt.
#define WIFI_CONNECT_TIMEOUT_MS     6000

// Time between reconnect cycles when the link is down.
#define WIFI_RECONNECT_INTERVAL_MS  20000

// Number of auth attempts per reconnect cycle.
// With Classic BT active, AUTH_EXPIRE on attempt 1 is common; attempt 2 wins.
#define WIFI_MAX_ATTEMPTS           3

// Pause between failed attempts — lets BT radio traffic subside.
#define WIFI_RETRY_DELAY_MS         1500

// ── ThingSpeak ────────────────────────────────────────────────────────────────
// Channel field mapping (8 fields on free plan):
//   Field 1 → Temperature (°C)       AHT20
//   Field 2 → Humidity (%)           AHT20
//   Field 3 → eCO2 (ppm)            ENS160
//   Field 4 → TVOC (ppb)            ENS160
//   Field 5 → MQ135 raw ADC         analog
//   Field 6 → Battery Voltage (V)   voltage divider
//   Field 7 → GPS Latitude
//   Field 8 → GPS Longitude
#define THINGSPEAK_HOST     "api.thingspeak.com"
#define THINGSPEAK_PORT     80

// ── OTA ───────────────────────────────────────────────────────────────────────
#define OTA_PORT            3232