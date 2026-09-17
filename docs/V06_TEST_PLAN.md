# FieldSec v0.6 Test Plan

## Static / package validation

- Verify FAP and Developer Board versions are 0.6.0.
- Verify all v0.6 source and documentation files are present.
- Verify Python companion tools compile.
- Verify report generation handles v0.6, v0.5, and legacy evidence/finding rows.
- Verify ZIP integrity.

## Identifier persistence

1. Record three observations.
2. Confirm IDs increment without duplication.
3. Restart FieldSec.
4. Record another observation and confirm the counter continues.
5. Repeat for finding and retest IDs.

## Evidence timeline

- Select a target and create more than eight observations.
- Confirm the handheld timeline displays the latest eight matching records.
- Switch targets and confirm unrelated observations do not appear.

## Traceability

- Create a finding citing two valid OBS IDs.
- Export data and run `fieldsec_report.py`.
- Confirm the report lists the linked IDs.
- Create a finding citing a nonexistent OBS ID and confirm the desktop report flags it.

## Retest workflow

- Create a finding and note its FND ID.
- Record Pass, Fail, and Partial retest examples.
- Confirm each produces a unique RT ID and retains the FND link.
- Confirm the desktop report groups retests under the referenced finding.

## Capstone

- Start from a new engagement and verify score 0/7 or expected partial score.
- Complete each objective independently and verify the score increases appropriately.
- Confirm the score does not depend on severity, number of vulnerabilities, or successful exploitation.

## Existing hardware regression tests

Repeat v0.5 physical checks for UART receive-only electrical inactivity, I2C address discovery, passive Developer Board Wi-Fi scans, expansion-service restoration, and microSD failure handling.
