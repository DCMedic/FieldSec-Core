# FieldSec Developer Board Telemetry

## FS2 schema

FieldSec v0.4 retains the stable newline-delimited FS2 telemetry schema:

- `FS2|READY|FieldSec-DevBoard|0.4.0`
- `FS2|CAPS|WIFI_SCAN|PASSIVE_METADATA_ONLY`
- `FS2|SCAN_BEGIN|<count>`
- `FS2|AP|<BSSID>|<RSSI>|<channel>|<auth>|<SSID>`
- `FS2|SCAN_END|<count>`
- `FS2|ERROR|<stage>|<code>`

The companion sanitizes pipe/newline characters in SSIDs before transmission. FieldSec saves raw telemetry to `wifi_scan.txt` and displays a bounded AP summary on the Flipper.

## Transport in v0.4

The official Wi-Fi Developer Board schematic routes ESP32-S2 UART0 through the module TX/RX nets to the Flipper USART connector. The companion uses the ESP-IDF UART console at 115200. FieldSec temporarily disables the Flipper Expansion service, acquires USART, disables Flipper TX, and listens receive-only for FS2 telemetry.

This transport is intentionally simple for physical validation. The official Flipper Expansion Module Protocol remains the planned long-term smart-module transport because it provides automatic module detection, baud negotiation, framing, checksums, heartbeat handling and RPC integration.

## Safety / behavior

The Wi-Fi companion requests `WIFI_SCAN_TYPE_PASSIVE`. No deauthentication, credential capture, authentication attempts, rogue portal, or frame-injection workflow is implemented.
