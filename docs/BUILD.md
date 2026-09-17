# Build and Install

## Flipper application

FieldSec v1.0 is an external Flipper application (FAP) targeting official firmware APIs.

### uFBT workflow

1. Install Python and `pipx`.
2. Install uFBT: `pipx install ufbt`.
3. Copy the `fieldsec` directory to a development workspace.
4. Run `ufbt` inside the app directory.
5. With a Flipper connected over USB, use `ufbt launch` to build and start the FAP.
6. For normal installation, place the resulting `.fap` in the SD-card Tools applications directory.

The application uses the official app-data path abstraction for persistent state, targets, training progress, evidence, findings and hardware captures.

## Wi-Fi Developer Board companion

The board firmware is an ESP-IDF project targeting ESP32-S2. Flashing it replaces stock Developer Board firmware until stock firmware is restored.

1. Install an ESP-IDF version compatible with ESP32-S2.
2. Connect the Developer Board directly to the computer over USB-C.
3. Enter bootloader mode according to Flipper documentation.
4. From `devboard/` build and flash the project.
5. Cold-plug the board into the Flipper: power the Flipper off, fully seat the board, then power on.
6. In FieldSec, define an authorized engagement and select Live Instruments -> Wi-Fi Dev Board.

The companion uses the ESP32-S2 UART console at 115200 and explicit passive Wi-Fi scanning. FieldSec listens receive-only on the Flipper USART.

## Validation

Run `python tools/preflight.py` and `python tools/validate_release.py` for source/package checks. Then follow `RELEASE_RUNBOOK.md`. Source validation does not replace ARM compilation, ESP-IDF compilation, electrical verification, or testing on physical hardware.
