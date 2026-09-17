# FieldSec v0.5 Engineering Notes

## Milestone: Interactive Workbenches & Guided Analysis

FieldSec v0.5 advances the project from one-shot instrumentation toward a handheld cybersecurity learning and field-assessment environment. The design rule for this release is that a live tool and its teaching workflow must share the same evidence.

## UART Workbench

The UART path remains receive-only by design. It now produces:

- printable-text preview;
- bounded hexadecimal preview;
- captured byte count;
- printable-byte ratio as a baud/framing clue;
- raw `uart_capture.bin` evidence;
- `uart_history.csv` session statistics;
- guided analysis questions on-device.

The printable ratio is an observation aid, not automatic baud detection. High printable ratios can support a framing hypothesis; low ratios can represent binary protocols, encryption/compression, wrong framing, or noise.

## I2C Workbench

The external I2C discovery tool still performs address-presence probes only. v0.5 adds a saved baseline and reports:

- current responding addresses;
- addresses newly present since the prior run;
- addresses no longer present since the prior run.

A topology change is not automatically a security event. Power state, bus multiplexers, target firmware, wiring and peripheral state must be considered.

## Wi-Fi Workbench

The official Wi-Fi Developer Board remains passive-scanning firmware. FieldSec compares BSSID presence against the prior survey and shows new/disappeared counts. A `+` prefix on the handheld denotes a BSSID absent from the prior stored baseline.

This is change detection, not intrusion detection. Operators and students must establish ownership and context before converting a wireless observation into a finding.

## Target Intelligence

Each saved target can now retain:

- device/target type;
- IP or network identity;
- MAC/BSSID;
- firmware/version;
- observed interfaces;
- operator notes.

These fields are inventory context, not vulnerability assertions.

## Structured Findings

The on-device finding builder now records distinct fields for:

1. Title
2. Severity
3. Observed condition
4. Evidence reference
5. Validated impact
6. Recommended control
7. Retest success criteria
8. Status (`Open` at creation)

This is deliberately more demanding than a single note. It reinforces the difference between observation, inference and demonstrated impact.

## Training Practice Gate

Paired Academy lessons track practice attempts. A lesson with a paired live instrument cannot be marked complete until the student has launched that instrument at least once. This is a lightweight educational gate, not an exam or competency certification.

## Safety and scope

FieldSec v0.5 continues to prioritize observation-first instrumentation. It does not add credential theft, deauthentication, persistence, destructive actions, broad automated exploitation, covert surveillance, or indiscriminate active scanning.
