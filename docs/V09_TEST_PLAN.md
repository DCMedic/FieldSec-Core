# FieldSec v0.9 Test Plan

1. Fresh install opens setup/settings with Beginner selected and Adaptive Guidance enabled.
2. Each of four skill levels persists across restart.
3. Adaptive Guidance persists independently of skill.
4. Legacy v0.8 state loads without forcing setup and retains all prior engagement settings.
5. Dashboard displays skill profile.
6. Adaptive Learning Path renders with and without a classified target.
7. UART/I2C/Wi-Fi instruments remain present and unchanged in root workbench menus.
8. All 15 assessment modules and 8 Academy lessons remain present.
9. OBS/FND/RT/HYP/ANN/JRN/IFC/SVC formats remain readable.
10. Desktop report reads state.ini skill/adaptive fields but remains compatible when state.ini is absent.
11. Receive-only UART, I2C address-presence scanning, and passive Wi-Fi behavior regressions remain unchanged.
