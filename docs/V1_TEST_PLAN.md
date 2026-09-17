# FieldSec v1.0 Test Plan

The v1.0 acceptance test is intentionally split into source validation and physical validation. Source checks are automated by `tools/validate_release.py`. Physical tests are detailed in `RELEASE_RUNBOOK.md`.

Key regressions: all 15 modules, all 8 Academy lessons, v0.10 skill matrix, adaptive curriculum, target graph, UART/I2C/Wi-Fi workbenches, all record families, capstone, reporting, and migration behavior must remain present.

New v1.0 checks: Competency Engine, Guided Assessment Planner, GPIO passive non-mutating monitor, first-release docs/tooling, current version metadata, API compatibility checker, and release-package manifest.
