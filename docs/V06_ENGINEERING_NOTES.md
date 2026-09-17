# FieldSec v0.6 Engineering Notes

## Release theme: Field Operations & Evidence Correlation

FieldSec v0.6 moves from independent observations toward a traceable assessment record. The primary design requirement is that the same evidence chain must support both professional field use and instruction.

## Stable record identifiers

New records receive persistent identifiers:

- `OBS-######` — observation/evidence record
- `FND-######` — structured finding
- `RT-######` — remediation/retest record

Identifiers are allocated from `counters.ini` in the FieldSec app-data directory. They are monotonic within the FieldSec installation. They are not cryptographic identifiers and are not intended to provide tamper evidence.

## Evidence-to-finding traceability

The operator should cite one or more `OBS-######` identifiers in the Evidence Reference field of a finding. This creates an explicit reasoning chain:

`Observation -> Finding -> Remediation -> Retest`

The desktop companion detects valid observation references and flags references absent from the exported dataset.

## Retest records

Retests are stored separately from findings in `retests.csv`. A retest cites a finding ID and records the observed result. This avoids rewriting the original finding after remediation and preserves assessment history.

## Target timeline

The handheld can render the latest observations associated with the active target. The timeline is intentionally evidence-centric; it does not infer causal relationships between observations.

## Capstone workflow

The v0.6 capstone score is a workflow-completion indicator with seven objectives:

1. Engagement named.
2. Scope entered and authorization acknowledged.
3. Target selected.
4. At least three observations recorded.
5. At least one structured finding recorded.
6. At least one retest recorded.
7. At least six Academy lessons completed.

The score measures assessment discipline, not offensive success, vulnerability count, or target compromise.

## Safety posture

The live hardware features remain observation-oriented. UART capture remains receive-only; I2C discovery performs address-presence probes; Wi-Fi Developer Board reconnaissance remains explicitly passive. FieldSec does not add automated credential theft, persistence, indiscriminate disruption, or broad exploitation in this milestone.
