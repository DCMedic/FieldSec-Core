# FieldSec v0.8 Engineering Notes

## Milestone
v0.8 adds Target Intelligence & Guided Hypothesis Management without replacing the v0.7 OBS/FND/RT traceability model.

## New record classes
- `HYP-######`: target-specific hypothesis, least-invasive test plan, expected result recorded before testing, observed result, analyst-selected status.
- `JRN-######`: engagement journal decisions, context, and analyst notes.
- `ANN-######`: annotations linked to immutable `OBS-######` evidence records.
- `IFC-######`: observed hardware/network/radio interface inventory.
- `SVC-######`: observed service/endpoint inventory.

Hypothesis status vocabulary is intentionally human-entered and instructional: `UNTESTED`, `TESTING`, `SUPPORTED`, `NOT SUPPORTED`, or `INCONCLUSIVE`. A supported hypothesis does not become a finding automatically.

## Reasoning model
FieldSec v0.8 trains an assessment loop:

1. Inventory attack surface.
2. Record direct observations.
3. State a falsifiable hypothesis.
4. Define the least-invasive sufficient test.
5. Predict the expected result before the test.
6. Run the authorized test using the appropriate workbench.
7. Record the observed result separately from interpretation.
8. Mark the hypothesis according to evidence.
9. Create a finding only when impact has been validated and supporting OBS IDs are retained.
10. Retest the remediation against explicit success criteria.

## Safety posture
v0.8 does not add credential theft, wireless disruption/deauthentication, persistence, destructive actions, broad exploit automation, or covert collection. UART remains receive-only by default, I2C remains address-presence discovery, and Wi-Fi scanning remains explicit passive scanning on the official Developer Board companion.
