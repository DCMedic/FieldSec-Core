# FieldSec v0.3 Engineering Notes

## What became live in this release

### UART Capture

- Uses official USART ownership through `furi_hal_serial_control_acquire`.
- Temporarily disables the Expansion service before serial acquisition and restores it after release.
- Initializes the selected baud rate with default 8-N-1 framing.
- Explicitly disables the TX direction after initialization, making the FieldSec workflow receive-only.
- Uses an interrupt-safe Furi stream buffer between the UART callback and application thread.
- Captures a three-second observation window.
- Stores raw received bytes as `uart_capture.bin` and presents a printable preview.

### I2C Discovery

- Uses the official external I2C bus handle.
- Acquires and releases the bus for every scan.
- Probes the normal 7-bit address range 0x08 through 0x77.
- Displays responder addresses only; it does not read registers or write configuration.

### Target Database

- Stores up to 12 target names in the in-memory quick-select list.
- Persists targets in `targets.txt`.
- Selecting a target updates the engagement's active target used by findings and evidence.

## Current official-FAP API verification

Development was checked against the current official F7 API symbol table on 2026-09-17. The APIs used by the new live instrumentation are currently exported to FAPs, including Expansion enable/disable, serial control acquire/release, async serial receive, stream buffers, external I2C handle, bus acquire/release, and device-ready probing.

## Deliberate hardware gates

Live GPIO monitoring is not enabled yet because a professional field tool must preserve existing pin ownership/configuration. Blindly switching all expansion pins to input could interfere with UART, I2C, SPI, SWD, or an attached expansion module. The next implementation must snapshot ownership/configuration, monitor only selected safe pins, and restore them exactly.

Direct Flipper-to-Wi-Fi-Developer-Board smart-module transport is also still gated until the physical board revision and connector routing are validated. The passive board firmware remains independently testable over its console.

## Physical test sequence recommended before calling v0.3 device-validated

1. Compile the FAP against current official firmware/uFBT.
2. Launch with no external hardware connected and exercise every menu/navigation path.
3. Verify Target Database persistence after reboot.
4. Connect a known 3.3 V UART source that emits a harmless repeating string.
5. Confirm with a logic analyzer that Flipper TX remains inactive during FieldSec UART capture.
6. Test all five baud presets.
7. Confirm `uart_capture.bin` byte-for-byte against the source output.
8. Confirm the Expansion service resumes after capture.
9. Connect a known I2C lab device and confirm its documented 7-bit address is discovered.
10. Repeat with two I2C devices on the same bus.
11. Power-cycle and verify engagement/settings persistence.
12. Only after these pass, proceed to GPIO snapshot/restore development and direct Developer Board transport.
