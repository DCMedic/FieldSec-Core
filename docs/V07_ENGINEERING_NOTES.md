# FieldSec v0.8 Engineering Notes

## Milestone
Field Analysis & Session Intelligence. v0.8 preserves the v0.6 OBS/FND/RT traceability backbone and adds analytical history views rather than creating a second evidence system.

## UART session intelligence
- Review recent baud/duration/byte-count/printable-ratio sessions.
- Extract printable strings (minimum length 4) from the most recent raw capture.
- Raw evidence remains authoritative; extracted strings are an analyst convenience and may omit binary structure.

## I2C topology intelligence
Each scan appends a bounded textual topology record containing responding-address count, additions/removals relative to the previous scan, and address list. Address presence is not device identity.

## Wi-Fi history
Passive surveys append AP/open/new/disappeared/security-change counts and channel occupancy. A separate detail snapshot permits advertised-security change detection for a persistent BSSID. Baseline deviation remains an observation, not proof of compromise.

## Target workspace
The target workspace correlates record counts and recent observations, then explicitly prompts the operator to distinguish observation, change, hypothesis, and validated finding.

## GPIO
Live GPIO monitoring remains gated pending physical validation of exact pin-configuration snapshot/restore behavior.
