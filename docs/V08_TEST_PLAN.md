# FieldSec v0.8 Test Plan

1. Confirm v0.8 version strings in FAP and Developer Board companion.
2. Create an engagement and target; verify authorization gating remains intact.
3. Create a HYP record and verify statement, test plan, expected result, observed result, and status persist.
4. Verify hypothesis IDs remain unique across restarts.
5. Create JRN entries and confirm target filtering.
6. Create ANN records against valid and invalid OBS IDs; verify desktop report warns on missing OBS references.
7. Add IFC and SVC inventory entries and verify target map rendering.
8. Generate desktop report and confirm Hypothesis Register, Target Inventory Maps, Evidence Annotations, Engagement Journal, findings, retests, and traceability matrix are all represented.
9. Regression-test UART receive-only behavior and Expansion service restoration.
10. Regression-test I2C address discovery without writes.
11. Verify Developer Board continues `WIFI_SCAN_TYPE_PASSIVE` operation.
12. Verify legacy v0.4-v0.7 exports remain readable by the report companion.
