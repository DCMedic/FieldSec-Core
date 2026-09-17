# FieldSec v0.4 Engineering Notes

## Purpose

v0.4 is the Learn & Operate milestone. It makes the educational environment persistent and pairs lessons with live instruments.

## Live UART

UART capture remains receive-only. The TX direction is disabled. The observation window is configurable at 1, 3 or 10 seconds. The Flipper Expansion service is temporarily disabled while FieldSec owns the USART, then restored. Successful captures automatically append an evidence record.

## Live I2C

I2C discovery remains an address-presence probe across normal 7-bit addresses 0x08-0x77. It does not read or write device registers. Results automatically append an evidence record.

## Wi-Fi Developer Board direct telemetry

The official Wi-Fi Developer Board schematic shows the ESP32-S2 UART0 TXD0/RXD0 connected to the board TX/RX nets and the Flipper connector exposes UART on pins 13/14. FieldSec companion firmware is configured for the ESP UART console, allowing the attached Flipper to receive FS2 telemetry on its USART.

FieldSec v0.4 therefore adds a receive-only Developer Board survey path:

1. FieldSec disables the Expansion service temporarily.
2. It acquires the USART at 115200 baud.
3. TX is disabled on the Flipper.
4. It listens for FS2 telemetry for up to 17 seconds.
5. AP observations are parsed into SSID, channel, RSSI and advertised authentication summaries.
6. Raw telemetry is stored in `wifi_scan.txt`.
7. The survey is recorded in `evidence.csv`.
8. The USART is released and the Expansion service is restored.

The ESP32-S2 companion explicitly uses `WIFI_SCAN_TYPE_PASSIVE`; FieldSec does not implement deauthentication, credential capture, authentication attempts, rogue portals or frame injection.

The official Flipper Expansion Module Protocol remains the long-term direction for module negotiation, heartbeats and RPC integration. v0.4 deliberately establishes a simpler receive-only data path first because it is easy to inspect with a logic analyzer and minimizes target-side behavior.

## GPIO

GPIO live monitoring remains validation-gated. The instructional lesson is active, but FieldSec does not blindly reconfigure shared pins. A future release should snapshot ownership/configuration, permit deliberate selection of an observation pin, use high-impedance input behavior where appropriate, and restore the exact prior state.

## Desktop companion

`tools/fieldsec_report.py` turns evidence and findings exported from FieldSec into a Markdown engagement report with analyst checkpoints and retest prompts. It performs no scanning or exploitation.
