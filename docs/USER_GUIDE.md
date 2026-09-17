# FieldSec v1.0 User Guide

## First use

1. Create an Engagement.
2. Set a primary Target.
3. Write a concise Scope note.
4. Acknowledge Authorization only when written permission exists.
5. Choose Learn or Field mode in Settings.

## Training Academy

Open Training Academy from the root menu. `[ ]` means Not started, `[>]` means In progress and `[x]` means Complete.

Open a lesson and use the center button to Start, mark Done, or Reset progress. Lessons paired with a live instrument show a Practice button.

## UART

Verify voltage and ground before connection. Connect the target TX signal to the Flipper RX path and share ground. FieldSec disables its transmit direction during capture. Set baud and capture window in Settings.

## I2C

Power down before wiring. Verify 3.3 V compatibility and correct SDA/SCL/ground connections. FieldSec performs address-presence checks only and does not access device registers.

## Wi-Fi Developer Board

Flash the included FieldSec Developer Board companion firmware. Fully seat the board with the Flipper powered off before connection. The board performs passive Wi-Fi scans and emits FS2 telemetry via its UART console. FieldSec listens at 115200 baud and records a survey to `wifi_scan.txt` plus a structured evidence row.

An SSID, open advertised authentication mode or strong RSSI is a reconnaissance observation, not proof of compromise. Validate ownership and actual configuration before writing a finding.

## Evidence and findings

Live UART, I2C and Wi-Fi runs append structured evidence automatically. Assessment Modules can also be logged manually. Findings should separate direct observation from inference and describe impact only when the required preconditions are demonstrated.

## Desktop report

Copy the FieldSec app-data files to a computer and run:

`python tools/fieldsec_report.py <data-directory> -o fieldsec_report.md`

The report includes evidence, findings, educational review prompts and retest questions.


## Traceability

FieldSec assigns observation (`OBS-######`), finding (`FND-######`), and retest (`RT-######`) IDs. Cite OBS IDs in findings and FND IDs in retests. Use **Evidence Timeline** for recent target observations and **Capstone Status** to track a complete authorized assessment workflow.

## Guided Hypothesis Workflow
Use **+ New Hypothesis** after selecting an authorized target. Record a falsifiable statement, the least-invasive adequate test, and the expected result before running the test. After testing, record only what was observed and select a status: UNTESTED, TESTING, SUPPORTED, NOT SUPPORTED, or INCONCLUSIVE.

Use **+ Annotate OBS** to add analyst context to an existing observation without changing the original evidence record. Use **+ Map Interface** and **+ Map Service** for attack-surface inventory. Inventory records do not establish vulnerability or impact by themselves.

The **Instructor Review** screen summarizes hypotheses and annotations and presents reasoning-quality checkpoints suitable for classroom, self-study, or peer review.


## Skill profile
On a fresh install, select Beginner, Intermediate, Advanced, or Expert/Field in Settings. Adaptive Guidance changes presentation depth and recommended sequencing only; it does not remove capabilities.


## GPIO Monitor

FieldSec v1.0 passively samples the supported external GPIO connector pins for one second without changing their configuration. It reports starting state, ending state, and sampled edge count. This is useful for observation and teaching, but it is not a replacement for a logic analyzer, oscilloscope, or voltage meter.

## Competency Engine

The Competency Engine distinguishes the skill level the learner selected from assessment evidence they have actually produced. It never locks tools. Use it as a self-study/instructor aid, not as a formal certification score.

## Assessment Planner

The Guided Assessment Planner uses target/interface context to recommend a least-invasive sequence. It does not authorize activity and it does not convert inventory into findings.

## First-release validation

Before treating a hardware installation as validated, follow every applicable step in `RELEASE_RUNBOOK.md`.
