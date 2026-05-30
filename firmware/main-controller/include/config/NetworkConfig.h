#pragma once

// =============================================================================
// Network Configuration — WiFi · ThingSpeak · OTA
//
// Sensitive credentials (SSID, passwords, API keys) stay in Secrets.h.
// This file holds only structural / timing / addressing constants so that
// every network-layer number has one authoritative definition.
// =============================================================================

// ── WiFi ──────────────────────────────────────────────────────────────────────
// How long to wait for an association before giving up on a single attempt.
#define WIFI_CONNECT_TIMEOUT_MS     8000

// How long to wait between reconnection attempts when the link is lost.
#define WIFI_RECONNECT_INTERVAL_MS  20000

// ── ThingSpeak ────────────────────────────────────────────────────────────────
// Channel field mapping (8 fields on free plan):
//   Field 1 → Temperature (°C)       AHT20
//   Field 2 → Humidity (%)           AHT20
//   Field 3 → eCO2 (ppm)            ENS160
//   Field 4 → TVOC (ppb)            ENS160
//   Field 5 → MQ135 raw ADC         analog
//   Field 6 → GPS Latitude
//   Field 7 → GPS Longitude

#define THINGSPEAK_HOST     "api.thingspeak.com"
#define THINGSPEAK_PORT     80

// ── OTA ───────────────────────────────────────────────────────────────────────
#define OTA_PORT            3232

// How many connection attempts before giving up (until next reconnect cycle).
// With BT active, AUTH_EXPIRE on attempt 1 is common; attempt 2 usually wins.
#define WIFI_MAX_ATTEMPTS       3

// Pause between failed attempts — gives the BT radio time to go quiet.
#define WIFI_RETRY_DELAY_MS     1500