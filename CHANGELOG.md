# FieldSec Changelog

## 1.0.0 — First Release Milestone

- Promoted the project to the first public release baseline.
- Preserved all v0.10 capabilities and legacy data compatibility.
- Added Competency Engine and least-invasive Guided Assessment Planner.
- Closed the GPIO placeholder with a non-mutating external-pin digital snapshot and history log.
- Added release/API compatibility checks, preflight tooling, installation and physical-validation runbooks.
- Updated the official Wi-Fi Developer Board companion to 1.0.0 while retaining passive Wi-Fi survey behavior.
- Remaining release gates are physical compilation/device validation steps documented in `docs/RELEASE_RUNBOOK.md`.

# FieldSec Changelog

## 0.10.0
- Added per-domain skill matrix across five cybersecurity learning domains.
- Added Adaptive Curriculum view.
- Added Target Relationship Graph and desktop cross-domain graph section.
- Preserved all v0.9 capabilities and legacy data compatibility.

# Changelog

## 0.9.0
- Added persistent Beginner, Intermediate, Advanced, and Expert/Field skill profiles.
- Added first-run skill/adaptive setup behavior.
- Added independent Adaptive Guidance toggle.
- Added target-aware Adaptive Learning Path.
- Added skill-aware module and lesson presentation without removing underlying capabilities.
- Added explicit Capability Preservation Contract.
- Preserved legacy v0.8 state and report compatibility.
- Bumped Wi-Fi Developer Board companion to 0.9.0 while retaining passive scan posture.


## 0.8.0

- Added UART session review and printable-string extraction from retained raw captures.
- Added persistent I2C topology history.
- Added passive Wi-Fi survey history with channel occupancy and advertised-security-change observations.
- Added Target Workspace correlating observations, findings, retests and analyst prompts.
- Added After-Action Review for instructional reflection and professional workflow review.
- Added desktop Executive Summary and OBS → FND → RT Traceability Matrix.
- Preserved v0.6 record IDs and backward-compatible report ingestion.
- Preserved receive-only UART, I2C presence-only discovery, and passive Wi-Fi defaults.


## 0.6.0

- Added persistent `OBS-######`, `FND-######`, and `RT-######` record identifiers.
- Added evidence timeline for the active target.
- Added evidence/finding/retest counts to target intelligence and dashboard.
- Added dedicated remediation retest logger.
- Added workflow-based capstone status (7 objectives).
- Added evidence-to-finding traceability in the desktop report, including missing-reference warnings.
- Added retest correlation in desktop reports.
- Added v0.6 capstone, engineering notes, and test plan.
- Preserved receive-only UART, I2C presence-only discovery, and passive Wi-Fi reconnaissance defaults.


## 0.5.0
- Added UART Workbench with text + hex preview, printable ratio and capture history.
- Added I2C baseline comparison with new/missing address counts.
- Added passive Wi-Fi BSSID baseline comparison using the official Wi-Fi Developer Board.
- Added persistent Target Intelligence profiles.
- Replaced single-note findings with structured condition/evidence/impact/remediation/retest records.
- Added Academy practice counters and a practice-before-completion gate for paired lessons.
- Updated desktop reporting to understand structured findings and target profiles while retaining v0.4 finding compatibility.
- Kept Wi-Fi companion scans explicitly passive.

## 0.4.0
- Added Training Academy and persistent lesson progress.
- Added direct passive Wi-Fi Developer Board telemetry path.
- Added configurable UART observation windows.
- Added desktop Markdown report utility.

## 0.8.0 - Target Intelligence & Guided Hypothesis Management
- Added persistent `HYP-######` hypothesis records with statement, least-invasive test plan, expected result, observed result, and status.
- Added `JRN-######` engagement journal records.
- Added `ANN-######` evidence annotations linked to `OBS-######` without modifying raw observations.
- Added persistent target interface (`IFC`) and service (`SVC`) inventory records.
- Added on-device Hypothesis Workspace, Engagement Journal, Evidence Annotations, Interface/Service Map, and Instructor Review.
- Extended desktop reports with Hypothesis Register, target inventory maps, annotations, journal entries, and missing-OBS-reference checks.
- Preserved v0.7 OBS/FND/RT traceability and conservative live-instrument behavior.
