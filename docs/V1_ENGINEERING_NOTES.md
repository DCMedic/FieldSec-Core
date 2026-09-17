# FieldSec v1.0 Engineering Notes

FieldSec 1.0 is the first release baseline. It preserves the v0.10 capability set and closes software endpoints that can be responsibly completed without physical device access.

## Closed in v1.0

- Version/promotional release metadata moved to 1.0.0.
- Competency Engine added; selected skill remains separate from demonstrated practice evidence.
- Guided Assessment Planner added; it proposes least-invasive next steps from target/interface context.
- GPIO placeholder replaced with a passive non-mutating one-second digital monitor of external connector pins. The snapshot reads existing input data-register state only and does not call GPIO configuration functions.
- Release/API compatibility and environment preflight tooling added.
- Release runbook, migration guidance, acceptance criteria, and first-release checklist added.

## Safety and interpretation

The GPIO snapshot does not claim to be a logic analyzer. Pins configured for alternate, analog, or shared functions can produce digital states that are not meaningful for an external signal. Timing, electrical loading, voltage-level, or protocol claims require appropriate external instrumentation.

UART remains receive-only by default, I2C remains address-presence discovery, and the Wi-Fi board remains explicitly passive.

## Physical gates intentionally left for the owner

This environment cannot attach to the user's Flipper Zero or Wi-Fi Developer Board. Therefore final FAP build/install, ESP-IDF flash, electrical validation, GPIO snapshot sanity checks, passive-scan RF validation, and on-device regression testing remain physical release gates. See `RELEASE_RUNBOOK.md`.
