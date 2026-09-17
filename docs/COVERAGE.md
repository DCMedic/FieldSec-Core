# Professional Red-Team Coverage Map

FieldSec is designed as a field and teaching companion. It does not claim that the Flipper replaces a full offensive-security workstation.

| Engagement phase | FieldSec handheld role | Developer Board role | Typical desktop handoff |
|---|---|---|---|
| Scope / ROE | Engagement profile and authorization gate | - | Formal ROE / ticketing / notes |
| Recon | Target notes, RF/NFC/hardware workflows | Passive Wi-Fi survey | Authorized Nmap inventory, asset tools |
| Physical / credential tech | NFC, LF RFID, iButton guidance | - | Access-control logs/vendor tools |
| Embedded / IoT | live receive-only UART, I2C discovery, GPIO/SPI guidance | WLAN metadata | Logic analyzer, firmware tools, Wireshark |
| Endpoint physical access | Benign USB-HID / policy labs | - | EDR and device-control telemetry |
| Web/API | Discovery/handoff guidance | Network context | Burp Suite or equivalent |
| Packet analysis | Observation/handoff | Wi-Fi metadata | Wireshark/tshark |
| Identity / enterprise | Workflow education only | - | Dedicated AD/cloud identity tooling |
| Evidence | CSV evidence tied to engagement/target | Survey output | Screenshots, pcaps, photos, logs |
| Reporting | Finding builder + summary export | - | Full report generation and QA |

## Training topics represented without one-button compromise automation

- Social-engineering methodology and controls
- Credential attack concepts and defenses
- Persistence concepts and detection
- Wireless disruption/deauthentication concepts and telemetry
- Exploitation lifecycle and validation
- Post-exploitation evidence handling
- Detection engineering and retest

These are best taught in isolated vulnerable labs with desktop tooling and complete logging rather than embedded as indiscriminate handheld actions.
