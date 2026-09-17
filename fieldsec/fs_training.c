#include "fieldsec.h"
#include <stdio.h>
#include <string.h>

#define FS_DIR APP_DATA_PATH("fieldsec")
#define FS_TRAINING FS_DIR "/training.ini"

static const FsLesson lessons[] = {
    {
        "Engagement Fundamentals", "Methodology",
        "Define authorization, scope, target and evidence expectations before touching a system.",
        "Professional testing starts with a question and a boundary. Scope limits what may be touched; evidence records what was actually observed.",
        "Create an engagement, name one target, add a concise scope note and acknowledge authorization only when written permission exists.",
        "Open Dashboard and verify the engagement, active target and authorization state are correct.",
        "Explain the difference between an observation, an inference and a validated finding.",
        "Good scoping prevents accidental impact and makes findings reproducible and defensible.",
        "Can another assessor determine exactly which target and interfaces are in scope?", -1,
    },
    {
        "UART: From Pins to Evidence", "Hardware/IoT",
        "Learn to identify and passively observe an exposed serial interface.",
        "UART is commonly used for boot logs and maintenance consoles. RX/TX are directional and require a shared ground; unknown voltage levels must be verified before connection.",
        "Identify GND and suspected target TX. Connect target TX to Flipper RX only. Select a likely baud rate and begin with a short receive-only capture.",
        "Run UART Capture. If the output is unreadable, change baud and repeat without enabling transmit.",
        "Readable boot strings can reveal platform, firmware and debug state. They do not automatically prove administrative access or exploitability.",
        "Disable unnecessary production debug output, protect maintenance interfaces and remove exposed test pads where practical.",
        "What exact evidence would distinguish harmless boot logging from an unauthenticated maintenance console?", 0,
    },
    {
        "I2C: Discover the Trust Boundary", "Hardware/IoT",
        "Discover devices on an external I2C bus and reason about what they control.",
        "I2C addresses identify responding bus participants, not their identity or security relevance. The important question is what data or control each device contributes.",
        "Power down before wiring. Verify 3.3 V compatibility, common ground and SDA/SCL from documentation or board tracing.",
        "Run I2C Discovery. Record responding 7-bit addresses, then map addresses to known components using documentation or controlled observation.",
        "A sensor, EEPROM or controller is security-relevant only when its data crosses a trust boundary or controls a meaningful function.",
        "Keep secrets in protected storage, authenticate critical data paths and restrict physical/debug access.",
        "For one discovered address, what security decision depends on that component?", 1,
    },
    {
        "Wi-Fi Recon: Metadata to Hypothesis", "Wireless",
        "Use passive WLAN metadata to build a defensible reconnaissance hypothesis.",
        "SSID, BSSID, RSSI, channel and advertised authentication describe the environment. Reconnaissance narrows questions; it does not prove compromise.",
        "Use the official Wi-Fi Developer Board with FieldSec companion firmware in a network you are authorized to assess.",
        "Collect a scan and compare authentication modes, duplicate SSIDs, channel use and signal strength across observations.",
        "An open network, legacy security mode or unusually strong signal is a lead. Validate configuration and ownership before calling it a weakness.",
        "Use WPA2/WPA3 appropriately, segment IoT infrastructure and monitor unauthorized access points.",
        "Which observation is fact, and which security conclusion still requires another test?", 2,
    },
    {
        "GPIO: Observe Before Control", "Hardware/IoT",
        "Learn safe reasoning about exposed digital signals without blindly driving target pins.",
        "A GPIO can represent a status line, enable pin, interrupt or security-relevant control. Driving unknown pins can damage hardware or alter state.",
        "Verify voltage, determine pin ownership and prefer high-impedance observation. Document idle and normal-operation transitions.",
        "Use the GPIO module or external logic analyzer for controlled observation until FieldSec live snapshot/restore is physically validated.",
        "A changing pin matters only when its relationship to a trusted function is established.",
        "Enforce security decisions inside trusted controllers rather than exposed external signal paths.",
        "Can you describe the pin's function without changing its state?", 3,
    },
    {
        "Credential Systems", "RFID/NFC",
        "Evaluate whether a physical-access system relies on reusable identifiers or authenticated credentials.",
        "A readable identifier is not automatically a vulnerability. The trust model depends on what the reader/backend validates.",
        "Use credentials and readers you own or are explicitly authorized to assess. Start with technology identification and metadata inspection.",
        "Compare multiple legitimate reads and document which values remain static. Correlate with reader/backend logs where available.",
        "Static data becomes significant when possession or replay of that data is sufficient for authorization.",
        "Prefer authenticated credentials, backend validation and layered physical controls for sensitive access.",
        "What does the reader verify beyond an identifier?", -1,
    },
    {
        "Evidence to Finding", "Reporting",
        "Convert observations into a reproducible finding without overstating impact.",
        "A strong finding separates condition, evidence, preconditions, impact and remediation. Severity should follow demonstrated risk, not novelty.",
        "Select an engagement observation with retained evidence and identify what was directly observed versus inferred.",
        "Create a FieldSec finding with a precise title and concise evidence/impact note.",
        "Ask whether a second assessor could reproduce the condition from the evidence and whether the impact follows from demonstrated prerequisites.",
        "Recommend a control tied directly to the validated failure and define how to retest it.",
        "Does every claim in the finding trace to evidence or a clearly stated assumption?", -1,
    },
    {
        "Full IoT Assessment", "Capstone",
        "Perform a complete observation-first assessment of one authorized IoT device.",
        "Real assessments correlate physical interfaces, radio/network exposure, application behavior and defensive telemetry rather than treating each tool independently.",
        "Define scope and target. Inventory interfaces. Choose the least-invasive test that answers the highest-priority security question.",
        "Use FieldSec instruments and modules to gather evidence, create at least one justified finding or document why no weakness was validated, and export the summary.",
        "Correlate evidence across interfaces and distinguish attack surface from an exploitable condition.",
        "Apply defense in depth: secure update, strong identity, protected debug, segmentation, least privilege and useful logging.",
        "Could you brief a defender on exactly what was tested, what was found, and how to retest remediation?", -1,
    },
};

const FsLesson* fs_lessons_get(size_t* count) {
    if(count) *count = COUNT_OF(lessons);
    return lessons;
}

const char* fs_lesson_status_name(uint8_t status) {
    switch(status) {
    case FsLessonInProgress: return "In progress";
    case FsLessonComplete: return "Complete";
    default: return "Not started";
    }
}

bool fs_training_load(Storage* storage, uint8_t status[], uint8_t practice[], size_t max_lessons) {
    if(!storage || !status || !practice || !max_lessons) return false;
    memset(status, 0, max_lessons);
    memset(practice, 0, max_lessons);
    File* file = storage_file_alloc(storage);
    if(!file) return false;
    if(!storage_file_open(file, FS_TRAINING, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return false;
    }
    char buf[512];
    size_t read = storage_file_read(file, buf, sizeof(buf) - 1);
    buf[read] = '\0';
    storage_file_close(file);
    storage_file_free(file);
    char* save = NULL;
    char* line = strtok_r(buf, "\n", &save);
    while(line) {
        unsigned idx = 0, val = 0;
        if(sscanf(line, "s%u=%u", &idx, &val) == 2 && idx < max_lessons && val <= FsLessonComplete) {
            status[idx] = (uint8_t)val;
        } else if(sscanf(line, "p%u=%u", &idx, &val) == 2 && idx < max_lessons) {
            practice[idx] = (uint8_t)(val > 255 ? 255 : val);
        } else if(sscanf(line, "%u=%u", &idx, &val) == 2 && idx < max_lessons && val <= FsLessonComplete) {
            /* Backward-compatible v0.4 status format. */
            status[idx] = (uint8_t)val;
        }
        line = strtok_r(NULL, "\n", &save);
    }
    return true;
}

bool fs_training_save(Storage* storage, const uint8_t status[], const uint8_t practice[], size_t count) {
    if(!storage || !status || !practice) return false;
    storage_common_mkdir(storage, FS_DIR);
    File* file = storage_file_alloc(storage);
    if(!file) return false;
    bool ok = storage_file_open(file, FS_TRAINING, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    if(ok) {
        for(size_t i = 0; i < count; i++) {
            char line[40];
            int n = snprintf(line, sizeof(line), "s%u=%u\np%u=%u\n", (unsigned)i, (unsigned)status[i], (unsigned)i, (unsigned)practice[i]);
            if(n <= 0 || storage_file_write(file, line, (size_t)n) != (size_t)n) { ok = false; break; }
        }
        storage_file_sync(file);
    }
    storage_file_close(file);
    storage_file_free(file);
    return ok;
}
