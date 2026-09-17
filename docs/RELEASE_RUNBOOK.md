# FieldSec 1.0 First-Release Runbook

This is the authoritative step-by-step procedure for the remaining owner-operated release gates.

## A. Prepare the workstation

1. Update the Flipper Zero to the official firmware version you intend to support. Record the firmware version.
2. Install Python 3.11+ and Git if needed.
3. Install uFBT using the current official Flipper developer instructions. Confirm `ufbt` runs.
4. Install ESP-IDF with ESP32-S2 support for the official Wi-Fi Developer Board. Confirm `idf.py --version` runs.
5. Extract the FieldSec v1.0 source package into a short path with no unusual permissions.

## B. Run source-release checks

From the FieldSec root:

```bash
python tools/preflight.py
python tools/validate_release.py
```

Both should complete without a FieldSec validation failure. Preflight may report missing optional build tools until they are installed.

## C. Confirm FAP API compatibility

1. Obtain `targets/f7/api_symbols.csv` from the exact official firmware revision you intend to run.
2. Run:

```bash
python tools/check_fap_api.py /path/to/api_symbols.csv
```

3. Do not continue if a required FieldSec symbol is absent or marked unavailable. Build against that firmware revision or revise the affected call.

## D. Build the Flipper application

1. Enter the `fieldsec` source directory or copy it into the location expected by your uFBT workflow.
2. Run:

```bash
ufbt
```

3. Resolve every compiler error and every unresolved FAP import. Do not ignore unresolved-symbol warnings.
4. Locate the generated `fieldsec.fap`.
5. Record the FAP SHA-256 hash for the release notes.

## E. Install and smoke-test FieldSec

1. Connect the Flipper Zero by USB.
2. Use the current uFBT deployment command supported by your environment, or copy the built FAP into the external-apps Tools location on the microSD card.
3. Launch FieldSec.
4. Confirm the title reports `FieldSec 1.0.0`.
5. On a fresh test SD card/profile, confirm first-run skill setup appears.
6. Change overall and domain skills, exit, relaunch, and confirm persistence.
7. Create a test engagement and target and acknowledge authorization.
8. Confirm Dashboard, Target Intelligence, Adaptive Curriculum, Competency Engine, Assessment Planner, Training Academy, and reporting menus open without crash.

## F. Validate the UART workbench

Use only a 3.3 V logic target designed for UART testing. Never connect an unknown-voltage signal directly to the Flipper.

1. Connect common ground.
2. Connect target TX to Flipper RX only.
3. Leave Flipper TX electrically unconnected for the initial validation.
4. Start with the known target baud rate.
5. Run a 3-second receive-only capture.
6. Confirm bytes are captured and `uart_capture.bin` is created.
7. Confirm an OBS record appears.
8. With a logic analyzer or oscilloscope on Flipper TX, repeat the test and verify FieldSec does not transmit during receive-only capture.
9. Exit the workbench and verify other Expansion/serial functions still work, demonstrating Expansion service restoration.

## G. Validate I2C discovery

Use a known 3.3 V I2C development target.

1. Connect ground, SDA, and SCL correctly.
2. Run I2C Workbench against a known device.
3. Confirm its address appears.
4. Remove/power down the device and repeat.
5. Confirm the topology comparison reports the missing address.
6. Reconnect it and verify the address returns.
7. Confirm FieldSec never writes registers during this workflow.

## H. Validate GPIO Snapshot

The v1.0 GPIO tool is intentionally non-mutating.

1. Disconnect unknown external hardware.
2. Open GPIO Snapshot and record the baseline.
3. Connect a known 3.3 V digital source through an appropriate safe test setup to one supported external GPIO pin.
4. Drive LOW, run snapshot, and record the displayed state.
5. Drive HIGH, run snapshot, and record the displayed state.
6. Verify unrelated pins were not reconfigured.
7. If you have a debugger/logic analyzer, verify FieldSec does not drive the observed pin.
8. Remember that this tool is not a voltage meter or timing analyzer.

## I. Build and flash the official Wi-Fi Developer Board

1. Connect the board to the workstation using its supported programming connection.
2. From `devboard/` run the appropriate ESP-IDF target/setup commands for ESP32-S2.
3. Build with:

```bash
idf.py build
```

4. Flash and monitor using the board's supported port, for example:

```bash
idf.py -p <PORT> flash monitor
```

5. Confirm the console reports `FieldSec-DevBoard` version `1.0.0`.
6. Confirm the FS2 telemetry stream completes.

## J. Validate passive Wi-Fi behavior

1. Place a second independent Wi-Fi capture system in monitor mode in a controlled lab.
2. Run the Developer Board survey.
3. Verify the board does not emit active probe requests as part of the FieldSec scan workflow.
4. Attach the board to the Flipper and open Wi-Fi Workbench.
5. Confirm AP records populate and an OBS record is written.
6. Change the lab environment (for example power a test AP on/off) and verify BSSID baseline comparison behaves as expected.

## K. Validate evidence/reasoning/reporting

1. Produce at least three OBS records with the live tools.
2. Create a HYP record with expected result written before interpretation.
3. Add an ANN record referencing a valid OBS.
4. Create a structured FND citing valid OBS IDs.
5. Add a remediation RT record.
6. Export/copy the FieldSec data directory to the workstation.
7. Run:

```bash
python tools/fieldsec_report.py <data-directory> -o fieldsec_report.md
```

8. Confirm the report contains the target profile, hypotheses, findings, retests, traceability matrix, domain skill matrix, and competency context.
9. Intentionally create a test annotation/finding reference to a nonexistent OBS in a disposable copy and verify the report warns about the missing reference.

## L. First-release acceptance criteria

Do not call a particular hardware build validated until all of the following are true:

- FAP compiles against the intended official firmware API with no unresolved imports.
- App launches and survives basic navigation/relaunch persistence.
- UART receive-only behavior is electrically confirmed.
- I2C discovery matches known lab topology.
- GPIO snapshot reads known HIGH/LOW states without driving/reconfiguring pins.
- Developer Board builds/flashes and emits v1.0 FS2 telemetry.
- Independent capture confirms the Wi-Fi survey remains passive.
- OBS/HYP/ANN/FND/RT chain is created and exported successfully.
- Desktop report generation succeeds.
- Existing v0.9/v0.10 data survives an upgrade test.

When all items pass, record the tested Flipper firmware version, API version, FAP hash, Developer Board firmware hash, and test date in your release record.
