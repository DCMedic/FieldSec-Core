# FieldSec 1.0

FieldSec is a purpose-built handheld cybersecurity learning and authorized field-assessment environment for the official Flipper Zero and official Wi-Fi Developer Board.

FieldSec 1.0 pairs real observation-first workbenches with guided analysis and first-release workflow controls:

- UART Workbench: receive-only raw/text/hex capture and session statistics.
- I2C Workbench: address discovery with prior-baseline comparison.
- Wi-Fi Workbench: passive WLAN metadata with BSSID change comparison.
- Target Intelligence: target type, network identifiers, firmware, interfaces and notes.
- Structured Findings: condition, evidence, impact, remediation and retest criteria.
- Training Academy: persistent progress with paired real-tool practice requirements.
- Desktop Companion: Markdown reporting from the same evidence and findings.

FieldSec is not a replacement for a full workstation toolchain. It is designed to teach and support the observe -> hypothesize -> validate -> document -> defend workflow in the field.


## Traceability

FieldSec assigns observation (`OBS-######`), finding (`FND-######`), and retest (`RT-######`) IDs. Cite OBS IDs in findings and FND IDs in retests. Use **Evidence Timeline** for recent target observations and **Capstone Status** to track a complete authorized assessment workflow.


## Skill profile and v1.0 release
On a fresh install, select Beginner, Intermediate, Advanced, or Expert/Field in Settings. Adaptive Guidance changes presentation depth and recommended sequencing only; it does not remove capabilities.


v1.0 also adds a passive one-second GPIO Monitor, Competency Engine, Guided Assessment Planner, release preflight/API compatibility tooling, and a physical acceptance runbook. See `RELEASE_RUNBOOK.md`.
