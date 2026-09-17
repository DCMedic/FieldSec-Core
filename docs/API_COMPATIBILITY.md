# FieldSec 1.0 FAP API Compatibility

FieldSec 1.0 targets the official Flipper Zero external-app (FAP) API.

The source release was reviewed against the official F7 `api_symbols.csv` development branch reporting API 88.2 on 2026-09-17. The required serial-control, serial RX, Expansion, I2C, external-I2C handle, GPIO-resource helper, and external GPIO variables used by FieldSec were exposed in that API snapshot. `furi_hal_gpio_read()` is an inline header operation; the corresponding GPIO header is part of the exported FAP header set.

This is not a substitute for checking the exact API file that ships with the firmware revision installed on the user's device. Run:

```bash
python tools/check_fap_api.py /path/to/targets/f7/api_symbols.csv
```

before final release acceptance.
