#include "fieldsec.h"

static const FsModule modules[] = {
    {"Wi-Fi Survey", "Wireless", "Inventory nearby WLAN metadata with the official Wi-Fi Developer Board.", "Use only within an authorized lab or engagement. The companion reports SSID, BSSID, RSSI, channel and advertised authentication.", "Compare security mode, signal strength and channel placement. An OPEN/WEP result is an observation, not proof of compromise.", "Prefer modern WPA2/WPA3 configurations, isolate IoT networks and monitor unauthorized infrastructure.", "Correlate interesting hosts with an authorized desktop network inventory.", FsRiskPassive},
    {"RF Environment", "Wireless", "Characterize authorized Sub-GHz activity without disruption.", "Use receive/analyzer workflows on signals you own or have permission to assess.", "Compare repeated transmissions, frequency and protocol family. Static-looking frames require additional validation before drawing conclusions.", "Use authenticated or rolling protocols where appropriate and reduce unnecessary RF exposure.", "Capture multiple legitimate activations and compare variability.", FsRiskPassive},
    {"NFC Tag Inspector", "RFID/NFC", "Inspect tag technology, identifier behavior and exposed records.", "Use test tags or credentials you own. Read metadata using the official NFC application.", "Determine whether access decisions appear to rely only on reusable identifiers or stronger authenticated exchanges.", "Avoid authorization based solely on clonable identifiers; prefer modern authenticated credentials and backend validation.", "Document reader and credential technology in the engagement record.", FsRiskPassive},
    {"LF RFID Review", "RFID/NFC", "Assess legacy low-frequency credential design and trust assumptions.", "Use authorized laboratory credentials with the official LF RFID application.", "Record technology and whether the system appears to use static reusable identifiers.", "Migrate security-sensitive systems to authenticated credential technologies and layered physical controls.", "Compare observations with access-control configuration and logs.", FsRiskPassive},
    {"UART Discovery", "Hardware/IoT", "Identify exposed serial diagnostics on authorized hardware.", "Identify ground first. Begin receive-only. Connect Flipper RX to suspected target TX and try documented/common baud rates.", "Readable boot output can disclose firmware, board identity, debug state or configuration. Do not assume shell access or privilege from boot text alone.", "Disable unnecessary production debug output, authenticate maintenance interfaces and protect test pads.", "Save representative boot text and determine whether the interface remains active after boot.", FsRiskLocal},
    {"I2C/SPI Assessment", "Hardware/IoT", "Teach board-bus identification and associated trust boundaries.", "Power down before wiring. Confirm voltage and pinout from documentation whenever possible.", "Look for externally reachable memories, sensors or controllers whose data is trusted without authentication.", "Protect secrets with secure elements or cryptographic controls and restrict physical debug access.", "Map each device on the bus to the security decision it influences.", FsRiskLocal},
    {"GPIO Logic Check", "Hardware/IoT", "Document exposed digital states without injecting destructive signals.", "Confirm voltage compatibility before connecting. Prefer observation first.", "Accessible control pins may disclose device state or expose bypass paths if critical logic trusts external wiring.", "Remove unnecessary test points and enforce security in trusted controllers rather than external signal paths.", "Record the pin, idle state and state change associated with normal operation.", FsRiskLocal},
    {"USB HID Lab", "USB", "Demonstrate HID trust using a harmless, visible lab action.", "Use a designated workstation. The exercise should only open a benign text surface and type a marker.", "Success demonstrates that a newly attached keyboard can issue input under the current endpoint policy; it does not itself demonstrate privilege escalation.", "Use device-control policy, least privilege, locked sessions and user awareness controls.", "Check endpoint logs and document whether the device was identified or blocked.", FsRiskActiveSafe},
    {"IR Control Review", "Physical/IoT", "Assess whether unauthenticated infrared commands can alter security-relevant state.", "Use equipment you own and the official Infrared application.", "Determine whether IR can change alarm, service, power or configuration state that affects security.", "Require authenticated control for sensitive functions and control physical access to receivers.", "Identify which IR-accessible functions cross a trust boundary.", FsRiskActiveSafe},
    {"1-Wire/iButton", "Physical", "Review token technology and maintenance/access interfaces.", "Use authorized tokens/readers with the official iButton application.", "Document identifier behavior and whether the reader trusts static reusable data.", "Use authenticated tokens and monitor anomalous physical-access activity.", "Compare reader behavior with backend event logs where available.", FsRiskPassive},
    {"Endpoint Policy", "USB", "Validate whether endpoint controls handle unexpected devices as intended.", "Use a dedicated test endpoint with rollback and written scope.", "Observe device installation, policy prompts and privilege boundaries without bypassing protections.", "Apply allowlisting, EDR/device control, least privilege and workstation locking.", "Record both the endpoint response and relevant defensive telemetry.", FsRiskActiveSafe},
    {"IoT Threat Model", "Analysis", "Walk through assets, entry points, trust boundaries and abuse cases.", "Select one authorized device and list network, radio, physical, cloud and mobile interfaces.", "Prioritize realistic impact and prerequisites rather than novelty.", "Apply defense in depth, secure update, strong identity, segmentation and useful logging.", "Choose the least invasive test that can validate the highest-priority hypothesis.", FsRiskPassive},
    {"Crypto Reference", "Reference", "Provide field reminders for hashing, encryption, encoding, entropy and key management.", "Use the reference while reviewing designs and observations.", "Encoding is not encryption; hashes do not provide confidentiality; key protection is often the critical trust anchor.", "Use reviewed cryptography, authenticated encryption and protected key material.", "Identify where keys are generated, stored, rotated and revoked.", FsRiskPassive},
    {"Red-Team Workflow", "Training", "Teach scope, reconnaissance, hypothesis, validation, evidence, reporting and retesting.", "Start from a defined engagement and known-safe target.", "Prefer the least invasive test capable of answering the security question and distinguish observation from inference.", "Convert each validated weakness into a specific defensive control and retest condition.", "Write a finding that another assessor could reproduce from the evidence.", FsRiskPassive},
    {"Laptop Tool Map", "Training", "Map handheld observations to appropriate professional desktop tools.", "Use Wireshark for packet analysis, Burp Suite for web/API testing, Nmap for authorized network inventory and dedicated tools for identity or firmware analysis.", "Correlate physical and IoT observations with application, network and defensive telemetry.", "Keep tooling proportional to scope and preserve reproducible evidence.", "Move only the evidence needed for the next hypothesis into the desktop workflow.", FsRiskPassive},
};

const FsModule* fs_catalog_get(size_t* count) {
    if(count) *count = COUNT_OF(modules);
    return modules;
}

const char* fs_risk_name(FsRisk risk) {
    switch(risk) {
    case FsRiskPassive: return "PASSIVE";
    case FsRiskLocal: return "LOCAL";
    case FsRiskActiveSafe: return "ACTIVE-LAB";
    default: return "UNKNOWN";
    }
}

const char* fs_severity_name(FsSeverity severity) {
    switch(severity) {
    case FsSeverityInfo: return "Informational";
    case FsSeverityLow: return "Low";
    case FsSeverityMedium: return "Medium";
    case FsSeverityHigh: return "High";
    case FsSeverityCritical: return "Critical";
    default: return "Unknown";
    }
}
