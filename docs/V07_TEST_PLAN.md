# FieldSec v0.8 Test Plan

1. Verify v0.6 state, targets, evidence, findings, retests and lesson progress load unchanged.
2. Run three UART captures with different baud settings; verify session review order and string extraction against known test text.
3. Run repeated I2C scans while adding/removing a known lab peripheral; confirm topology history reports changes without assigning identity.
4. Run repeated passive Wi-Fi scans; confirm BSSID baseline, channel occupancy and advertised-security-change history. Verify with a monitor interface that the ESP32 companion remains passive.
5. Verify Target Workspace filters to the active target and shows current OBS/FND/RT counts.
6. Verify After-Action Review reflects current engagement, target, lesson completion and record counts.
7. Generate a desktop report from v0.6 and v0.8 data; verify Executive Summary and Traceability Matrix render without breaking legacy rows.
8. Regression-test UART TX inactivity, Expansion service restore, I2C bus release, microSD-write failure handling and Developer Board disconnect behavior.
