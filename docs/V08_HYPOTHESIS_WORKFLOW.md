# Guided Hypothesis Workflow

A good FieldSec hypothesis is testable and falsifiable.

Example:

**Statement:** Production UART may expose unauthenticated diagnostic metadata.

**Least-invasive test:** Receive-only boot capture at the documented logic level. Do not transmit data.

**Expected result:** If diagnostic output is exposed, readable boot or firmware-identification strings should appear during startup.

**Observed result:** `OBS-000021` retained a readable firmware version and board identifier.

**Status:** SUPPORTED.

This still does not automatically establish material security impact. The analyst must determine whether the exposed information crosses a security boundary, facilitates an attack, discloses protected information, or otherwise creates a validated risk before creating a `FND-######` record.
