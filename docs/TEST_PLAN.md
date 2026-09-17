# FieldSec v0.4 Validation Plan

## Static release checks

- Manifest and source version are both 0.4.0.
- Required v0.4 source files are present.
- 15+ assessment modules and exactly 8 academy lessons are present.
- UART TX is disabled during receive-only capture.
- Wi-Fi companion explicitly uses passive scanning.
- Direct board parser and report companion are present.

## Flipper device validation

1. Build with current uFBT against official firmware.
2. Launch on a Flipper with microSD installed.
3. Confirm state persistence, target database and training progress.
4. Confirm 1/3/10-second UART windows at 115200 using a known 3.3 V test source.
5. Verify with a logic analyzer that Flipper TX remains idle during UART capture.
6. Verify I2C scan against a known 3.3 V device and confirm no register writes occur.
7. Verify evidence.csv is appended after successful UART and I2C runs.

## Wi-Fi Developer Board validation

1. Build and flash the included ESP32-S2 project.
2. Confirm FS2 READY/CAPS output at 115200 baud.
3. Use a monitor-mode sniffer or second radio to verify the companion scan does not emit active probe requests.
4. Cold-plug the Developer Board into the Flipper.
5. Run Live Instruments -> Wi-Fi Dev Board.
6. Confirm at least one complete FS2 scan is parsed within the 17-second window.
7. Confirm wifi_scan.txt contains raw telemetry and evidence.csv contains a survey observation.
8. Confirm the Flipper USART is released and Expansion service is restored after the run.

## Training validation

1. Open each of the eight lessons.
2. Verify Start -> Done -> Reset state transitions.
3. Restart FieldSec and verify training.ini persistence.
4. Verify Practice links open the intended UART, I2C, Wi-Fi or GPIO workflow.
5. Confirm a learner can move from concept -> real observation -> evidence -> finding without losing engagement context.

## Desktop companion

Run `python tools/fieldsec_report.py <sample-data-directory> -o report.md` against synthetic evidence/findings and inspect the generated Markdown.
