#include "fieldsec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FS_DIR APP_DATA_PATH("fieldsec")
#define FS_STATE FS_DIR "/state.ini"
#define FS_COUNTERS FS_DIR "/counters.ini"
#define FS_EVIDENCE FS_DIR "/evidence.csv"
#define FS_FINDINGS FS_DIR "/findings.csv"
#define FS_RETESTS FS_DIR "/retests.csv"
#define FS_SUMMARY FS_DIR "/summary.txt"

static void fs_mkdir(Storage* storage) {
    storage_common_mkdir(storage, FS_DIR);
}

static void csv_copy(char* out, size_t out_size, const char* in) {
    if(!out_size) return;
    size_t j = 0;
    for(size_t i = 0; in && in[i] && j + 1 < out_size; i++) {
        char c = in[i];
        if(c == '\n' || c == '\r') c = ' ';
        if(c == '"') {
            if(j + 2 >= out_size) break;
            out[j++] = '"';
            out[j++] = '"';
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
}

static bool write_all(Storage* storage, const char* path, const char* data, FS_OpenMode mode) {
    File* file = storage_file_alloc(storage);
    if(!file) return false;
    bool ok = storage_file_open(file, path, FSAM_WRITE, mode);
    if(ok) {
        size_t len = strlen(data);
        ok = storage_file_write(file, data, len) == len;
        storage_file_sync(file);
    }
    storage_file_close(file);
    storage_file_free(file);
    return ok;
}

static size_t read_text(Storage* storage, const char* path, char* out, size_t out_size) {
    if(!storage || !out || out_size < 2) return 0;
    out[0] = '\0';
    File* file = storage_file_alloc(storage);
    if(!file) return 0;
    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return 0;
    }
    size_t read = storage_file_read(file, out, out_size - 1);
    out[read] = '\0';
    storage_file_close(file);
    storage_file_free(file);
    return read;
}

static void set_value(char* dst, size_t dst_size, const char* src) {
    if(!dst || !dst_size) return;
    snprintf(dst, dst_size, "%s", src ? src : "");
}

bool fs_state_save(Storage* storage, const FsEngagement* engagement, const FsSettings* settings) {
    if(!storage || !engagement || !settings) return false;
    fs_mkdir(storage);
    char data[512];
    snprintf(
        data,
        sizeof(data),
        "version=%s\nengagement=%s\ntarget=%s\nscope=%s\nauthorized=%u\nmode=%u\nskill=%u\ndomain_hw=%u\ndomain_wireless=%u\ndomain_credential=%u\ndomain_evidence=%u\ndomain_threat=%u\nadaptive=%u\nsetup_complete=%u\nseverity=%u\nuart_baud=%u\nuart_duration=%u\n",
        FS_VERSION,
        engagement->engagement,
        engagement->target,
        engagement->scope,
        engagement->authorized ? 1U : 0U,
        (unsigned)settings->mode,
        (unsigned)settings->skill_level,
        (unsigned)settings->domain_skill[FsDomainHardware],
        (unsigned)settings->domain_skill[FsDomainWireless],
        (unsigned)settings->domain_skill[FsDomainCredential],
        (unsigned)settings->domain_skill[FsDomainEvidence],
        (unsigned)settings->domain_skill[FsDomainThreatModeling],
        settings->adaptive_guidance ? 1U : 0U,
        settings->setup_complete ? 1U : 0U,
        (unsigned)settings->severity,
        (unsigned)settings->uart_baud_index,
        (unsigned)settings->uart_duration_index);
    return write_all(storage, FS_STATE, data, FSOM_CREATE_ALWAYS);
}

bool fs_state_load(Storage* storage, FsEngagement* engagement, FsSettings* settings) {
    if(!storage || !engagement || !settings) return false;
    memset(engagement, 0, sizeof(*engagement));
    settings->mode = FsModeLearn;
    settings->skill_level = FsSkillBeginner;
    for(size_t i=0;i<FsDomainCount;i++) settings->domain_skill[i]=FsSkillBeginner;
    settings->adaptive_guidance = true;
    settings->setup_complete = false;
    settings->severity = FsSeverityInfo;
    settings->uart_baud_index = 4;
    settings->uart_duration_index = 1;

    char buf[512];
    if(!read_text(storage, FS_STATE, buf, sizeof(buf))) return false;
    bool saw_setup = false;
    bool saw_domain = false;
    char* save = NULL;
    char* line = strtok_r(buf, "\n", &save);
    while(line) {
        char* eq = strchr(line, '=');
        if(eq) {
            *eq = '\0';
            const char* key = line;
            const char* val = eq + 1;
            if(strcmp(key, "engagement") == 0) set_value(engagement->engagement, sizeof(engagement->engagement), val);
            else if(strcmp(key, "target") == 0) set_value(engagement->target, sizeof(engagement->target), val);
            else if(strcmp(key, "scope") == 0) set_value(engagement->scope, sizeof(engagement->scope), val);
            else if(strcmp(key, "authorized") == 0) engagement->authorized = atoi(val) != 0;
            else if(strcmp(key, "mode") == 0) settings->mode = (atoi(val) == 1) ? FsModeField : FsModeLearn;
            else if(strcmp(key, "skill") == 0) { int sk=atoi(val); if(sk>=0 && sk<FsSkillCount) settings->skill_level=(FsSkillLevel)sk; }
            else if(strcmp(key, "domain_hw") == 0) { int sk=atoi(val); if(sk>=0&&sk<FsSkillCount) settings->domain_skill[FsDomainHardware]=(FsSkillLevel)sk; saw_domain=true; }
            else if(strcmp(key, "domain_wireless") == 0) { int sk=atoi(val); if(sk>=0&&sk<FsSkillCount) settings->domain_skill[FsDomainWireless]=(FsSkillLevel)sk; saw_domain=true; }
            else if(strcmp(key, "domain_credential") == 0) { int sk=atoi(val); if(sk>=0&&sk<FsSkillCount) settings->domain_skill[FsDomainCredential]=(FsSkillLevel)sk; saw_domain=true; }
            else if(strcmp(key, "domain_evidence") == 0) { int sk=atoi(val); if(sk>=0&&sk<FsSkillCount) settings->domain_skill[FsDomainEvidence]=(FsSkillLevel)sk; saw_domain=true; }
            else if(strcmp(key, "domain_threat") == 0) { int sk=atoi(val); if(sk>=0&&sk<FsSkillCount) settings->domain_skill[FsDomainThreatModeling]=(FsSkillLevel)sk; saw_domain=true; }
            else if(strcmp(key, "adaptive") == 0) settings->adaptive_guidance = atoi(val) != 0;
            else if(strcmp(key, "setup_complete") == 0) { settings->setup_complete = atoi(val) != 0; saw_setup = true; }
            else if(strcmp(key, "severity") == 0) {
                int s = atoi(val);
                if(s >= 0 && s < FsSeverityCount) settings->severity = (FsSeverity)s;
            } else if(strcmp(key, "uart_baud") == 0) {
                int b = atoi(val);
                if(b >= 0 && b < 5) settings->uart_baud_index = (uint8_t)b;
            } else if(strcmp(key, "uart_duration") == 0) {
                int d = atoi(val);
                if(d >= 0 && d < 3) settings->uart_duration_index = (uint8_t)d;
            }
        }
        line = strtok_r(NULL, "\n", &save);
    }
    if(!saw_setup) settings->setup_complete = true; /* Legacy state: preserve prior user workflow. */
    if(!saw_domain) for(size_t i=0;i<FsDomainCount;i++) settings->domain_skill[i]=settings->skill_level; /* v0.9 migration */
    return true;
}

static uint32_t fs_next_counter(Storage* storage, const char* key) {
    uint32_t obs = 0, finding = 0, retest = 0;
    char buf[192];
    if(read_text(storage, FS_COUNTERS, buf, sizeof(buf))) {
        char* save = NULL;
        char* line = strtok_r(buf, "\n", &save);
        while(line) {
            unsigned value = 0;
            if(sscanf(line, "obs=%u", &value) == 1) obs = value;
            else if(sscanf(line, "finding=%u", &value) == 1) finding = value;
            else if(sscanf(line, "retest=%u", &value) == 1) retest = value;
            line = strtok_r(NULL, "\n", &save);
        }
    }
    uint32_t result = 0;
    if(strcmp(key, "obs") == 0) result = ++obs;
    else if(strcmp(key, "finding") == 0) result = ++finding;
    else result = ++retest;
    char out[128];
    snprintf(out, sizeof(out), "obs=%lu\nfinding=%lu\nretest=%lu\n",
        (unsigned long)obs, (unsigned long)finding, (unsigned long)retest);
    write_all(storage, FS_COUNTERS, out, FSOM_CREATE_ALWAYS);
    return result;
}

static void fs_make_id(Storage* storage, const char* key, const char* prefix, char* out, size_t out_size) {
    uint32_t n = fs_next_counter(storage, key);
    snprintf(out, out_size, "%s-%06lu", prefix, (unsigned long)n);
}

bool fs_evidence_append_event_id(
    Storage* storage,
    const FsEngagement* engagement,
    const char* category,
    const char* tool,
    FsRisk risk,
    const char* note,
    char* out_id,
    size_t out_id_size) {
    if(!storage || !engagement || !category || !tool) return false;
    fs_mkdir(storage);
    char id[FS_ID_MAX];
    fs_make_id(storage, "obs", "OBS", id, sizeof(id));
    char e[128], t[128], c[96], m[96], n[256];
    csv_copy(e, sizeof(e), engagement->engagement);
    csv_copy(t, sizeof(t), engagement->target);
    csv_copy(c, sizeof(c), category);
    csv_copy(m, sizeof(m), tool);
    csv_copy(n, sizeof(n), note ? note : "Observed");
    char line[960];
    snprintf(line, sizeof(line), "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
        id, e, t, c, m, fs_risk_name(risk), n);
    bool ok = write_all(storage, FS_EVIDENCE, line, FSOM_OPEN_APPEND);
    if(ok && out_id && out_id_size) snprintf(out_id, out_id_size, "%s", id);
    return ok;
}

bool fs_evidence_append(Storage* storage, const FsEngagement* engagement, const FsModule* module, const char* note) {
    if(!module) return false;
    return fs_evidence_append_event_id(storage, engagement, module->category, module->name, module->risk, note, NULL, 0);
}

bool fs_evidence_append_event(Storage* storage, const FsEngagement* engagement, const char* category, const char* tool, FsRisk risk, const char* note) {
    return fs_evidence_append_event_id(storage, engagement, category, tool, risk, note, NULL, 0);
}

bool fs_finding_append_structured_id(
    Storage* storage,
    const FsEngagement* engagement,
    const char* title,
    FsSeverity severity,
    const char* condition,
    const char* evidence,
    const char* impact,
    const char* remediation,
    const char* retest,
    char* out_id,
    size_t out_id_size) {
    if(!storage || !engagement || !title) return false;
    fs_mkdir(storage);
    char id[FS_ID_MAX];
    fs_make_id(storage, "finding", "FND", id, sizeof(id));
    char e[128], t[128], ti[160], c[256], ev[256], im[256], rem[256], rt[256];
    csv_copy(e, sizeof(e), engagement->engagement);
    csv_copy(t, sizeof(t), engagement->target);
    csv_copy(ti, sizeof(ti), title);
    csv_copy(c, sizeof(c), condition ? condition : "");
    csv_copy(ev, sizeof(ev), evidence ? evidence : "");
    csv_copy(im, sizeof(im), impact ? impact : "");
    csv_copy(rem, sizeof(rem), remediation ? remediation : "");
    csv_copy(rt, sizeof(rt), retest ? retest : "");
    char line[1900];
    snprintf(line, sizeof(line), "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
        id, e, t, fs_severity_name(severity), ti, c, ev, im, rem, rt, "Open");
    bool ok = write_all(storage, FS_FINDINGS, line, FSOM_OPEN_APPEND);
    if(ok && out_id && out_id_size) snprintf(out_id, out_id_size, "%s", id);
    return ok;
}

bool fs_finding_append_structured(
    Storage* storage,
    const FsEngagement* engagement,
    const char* title,
    FsSeverity severity,
    const char* condition,
    const char* evidence,
    const char* impact,
    const char* remediation,
    const char* retest) {
    return fs_finding_append_structured_id(storage, engagement, title, severity, condition, evidence, impact, remediation, retest, NULL, 0);
}

bool fs_retest_append(
    Storage* storage,
    const FsEngagement* engagement,
    const char* finding_id,
    const char* result,
    const char* note,
    char* out_id,
    size_t out_id_size) {
    if(!storage || !engagement || !finding_id || !finding_id[0] || !result || !result[0]) return false;
    fs_mkdir(storage);
    char id[FS_ID_MAX];
    fs_make_id(storage, "retest", "RT", id, sizeof(id));
    char f[64], e[128], t[128], r[96], n[256];
    csv_copy(f, sizeof(f), finding_id);
    csv_copy(e, sizeof(e), engagement->engagement);
    csv_copy(t, sizeof(t), engagement->target);
    csv_copy(r, sizeof(r), result);
    csv_copy(n, sizeof(n), note ? note : "");
    char line[800];
    snprintf(line, sizeof(line), "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n", id, f, e, t, r, n);
    bool ok = write_all(storage, FS_RETESTS, line, FSOM_OPEN_APPEND);
    if(ok && out_id && out_id_size) snprintf(out_id, out_id_size, "%s", id);
    return ok;
}

static bool row_contains_target(const char* line, const char* target) {
    if(!line || !target || !target[0]) return false;
    char needle[FS_TARGET_MAX + 4];
    snprintf(needle, sizeof(needle), "\"%s\"", target);
    return strstr(line, needle) != NULL;
}

static size_t count_rows_for_target(Storage* storage, const char* path, const char* target) {
    char buf[8192];
    if(!read_text(storage, path, buf, sizeof(buf))) return 0;
    size_t count = 0;
    char* save = NULL;
    char* line = strtok_r(buf, "\n", &save);
    while(line) {
        if(row_contains_target(line, target)) count++;
        line = strtok_r(NULL, "\n", &save);
    }
    return count;
}

bool fs_target_stats(Storage* storage, const char* target, size_t* evidence, size_t* findings, size_t* retests) {
    if(!storage || !target || !target[0]) return false;
    if(evidence) *evidence = count_rows_for_target(storage, FS_EVIDENCE, target);
    if(findings) *findings = count_rows_for_target(storage, FS_FINDINGS, target);
    if(retests) *retests = count_rows_for_target(storage, FS_RETESTS, target);
    return true;
}

bool fs_timeline_render(Storage* storage, const char* target, char* output, size_t output_size) {
    if(!storage || !output || output_size < 2) return false;
    output[0] = '\0';
    char buf[8192];
    if(!read_text(storage, FS_EVIDENCE, buf, sizeof(buf))) {
        snprintf(output, output_size, "No traceable observation records yet.");
        return true;
    }
    const size_t keep_max = 8;
    char keep[keep_max][220];
    size_t kept = 0;
    char* save = NULL;
    char* line = strtok_r(buf, "\n", &save);
    while(line) {
        if(!target || !target[0] || row_contains_target(line, target)) {
            char id[FS_ID_MAX] = {0}, tool[72] = {0}, note[120] = {0};
            char copy[960];
            snprintf(copy, sizeof(copy), "%s", line);
            char* fields[7] = {0};
            size_t nf = 0;
            char* p = copy;
            while(*p && nf < 7) {
                if(*p == '"') p++;
                fields[nf++] = p;
                char* end = strstr(p, "\",\"");
                if(end) { *end = '\0'; p = end + 3; }
                else { char* last = strrchr(p, '"'); if(last) *last='\0'; break; }
            }
            if(nf >= 7) {
                snprintf(id, sizeof(id), "%s", fields[0]);
                snprintf(tool, sizeof(tool), "%s", fields[4]);
                snprintf(note, sizeof(note), "%s", fields[6]);
                size_t slot = 0;
                if(kept < keep_max) {
                    slot = kept++;
                } else {
                    for(size_t i = 1; i < keep_max; i++) snprintf(keep[i - 1], sizeof(keep[i - 1]), "%s", keep[i]);
                    slot = keep_max - 1;
                }
                snprintf(keep[slot], sizeof(keep[slot]), "%s | %s | %.100s", id, tool, note);
            }
        }
        line = strtok_r(NULL, "\n", &save);
    }
    if(!kept) {
        snprintf(output, output_size, "No observations for the active target.");
        return true;
    }
    size_t used = 0;
    for(size_t i = 0; i < kept && used + 2 < output_size; i++) {
        int n = snprintf(output + used, output_size - used, "%s%s", i ? "\n\n" : "", keep[i]);
        if(n <= 0 || (size_t)n >= output_size - used) break;
        used += (size_t)n;
    }
    return true;
}

uint8_t fs_capstone_score(Storage* storage, const FsEngagement* engagement, const uint8_t lesson_status[], size_t lesson_count, char* detail, size_t detail_size) {
    if(!storage || !engagement) return 0;
    size_t evidence = 0, findings = 0, retests = 0;
    if(engagement->target[0]) fs_target_stats(storage, engagement->target, &evidence, &findings, &retests);
    size_t done = 0;
    for(size_t i = 0; lesson_status && i < lesson_count; i++) if(lesson_status[i] == FsLessonComplete) done++;
    uint8_t score = 0;
    if(engagement->engagement[0]) score++;
    if(engagement->authorized && engagement->scope[0]) score++;
    if(engagement->target[0]) score++;
    if(evidence >= 3) score++;
    if(findings >= 1) score++;
    if(retests >= 1) score++;
    if(done >= 6) score++;
    if(detail && detail_size) {
        snprintf(detail, detail_size,
            "Capstone objectives: %u/7\n\n%s Engagement named\n%s Scope + authorization\n%s Active target\n%s >=3 observations (%u)\n%s >=1 finding (%u)\n%s >=1 retest (%u)\n%s >=6 lessons complete (%u)\n\nThis score measures workflow completion, not offensive effectiveness.",
            score,
            engagement->engagement[0] ? "[x]" : "[ ]",
            (engagement->authorized && engagement->scope[0]) ? "[x]" : "[ ]",
            engagement->target[0] ? "[x]" : "[ ]",
            evidence >= 3 ? "[x]" : "[ ]", (unsigned)evidence,
            findings >= 1 ? "[x]" : "[ ]", (unsigned)findings,
            retests >= 1 ? "[x]" : "[ ]", (unsigned)retests,
            done >= 6 ? "[x]" : "[ ]", (unsigned)done);
    }
    return score;
}

bool fs_target_workspace_render(Storage* storage, const char* target, char* output, size_t output_size) {
    if(!storage || !output || output_size < 2) return false;
    if(!target || !target[0]) { snprintf(output, output_size, "Select an active target first."); return true; }
    size_t e=0,f=0,r=0; fs_target_stats(storage,target,&e,&f,&r);
    char timeline[1300]; fs_timeline_render(storage,target,timeline,sizeof(timeline));
    snprintf(output, output_size, "TARGET WORKSPACE\n%s\n\nRecords: %u OBS / %u FND / %u RT\n\nRECENT OBSERVATIONS\n%s\n\nANALYST CHECK\nWhat is directly observed? What changed from baseline? Which hypothesis is still untested? Which finding, if any, is supported by retained OBS IDs?", target, (unsigned)e,(unsigned)f,(unsigned)r,timeline);
    return true;
}

bool fs_after_action_render(Storage* storage, const FsEngagement* engagement, const uint8_t lesson_status[], size_t lesson_count, char* output, size_t output_size) {
    if(!storage || !engagement || !output || output_size < 2) return false;
    size_t e=0,f=0,r=0; if(engagement->target[0]) fs_target_stats(storage,engagement->target,&e,&f,&r);
    size_t done=0; for(size_t i=0; lesson_status && i<lesson_count; i++) if(lesson_status[i]==FsLessonComplete) done++;
    snprintf(output, output_size, "AFTER-ACTION REVIEW\nEngagement: %s\nTarget: %s\nRecords: %u OBS / %u FND / %u RT\nAcademy: %u/%u complete\n\nREVIEW\n1. Which conclusions are direct observations?\n2. Which conclusions depend on inference?\n3. Did every finding cite retained OBS IDs?\n4. Did any baseline deviation have a benign alternative?\n5. Was the least-invasive test used first?\n6. Did remediation receive an objective retest?\n7. What would you do differently on the next authorized engagement?\n\nInstructor note: quality is measured by defensible reasoning and traceability, not number of vulnerabilities found.", engagement->engagement[0]?engagement->engagement:"(not set)", engagement->target[0]?engagement->target:"(not set)",(unsigned)e,(unsigned)f,(unsigned)r,(unsigned)done,(unsigned)lesson_count);
    return true;
}

bool fs_export_summary(Storage* storage, const FsEngagement* engagement, const FsSettings* settings) {
    if(!storage || !engagement || !settings) return false;
    fs_mkdir(storage);
    size_t evidence = 0, findings = 0, retests = 0;
    if(engagement->target[0]) fs_target_stats(storage, engagement->target, &evidence, &findings, &retests);
    char out[1024];
    snprintf(
        out,
        sizeof(out),
        "FieldSec %s Engagement Summary\n\nEngagement: %s\nTarget: %s\nScope: %s\nAuthorization acknowledged: %s\nOperator mode: %s\nDefault finding severity: %s\nUART capture baud: %s\nUART capture window: %s\n\nActive target records\nObservations: %u\nFindings: %u\nRetests: %u\n\nTraceability\nObservations use OBS-###### IDs. Findings use FND-###### IDs and should cite supporting OBS IDs. Retests use RT-###### IDs and cite the finding they evaluate.\n\nWorkflow\n1. Define scope\n2. Reconnaissance\n3. Form hypothesis\n4. Use least-invasive validation\n5. Capture evidence\n6. Assess impact\n7. Write finding\n8. Recommend mitigation\n9. Retest\n",
        FS_VERSION,
        engagement->engagement[0] ? engagement->engagement : "(not set)",
        engagement->target[0] ? engagement->target : "(not set)",
        engagement->scope[0] ? engagement->scope : "(not set)",
        engagement->authorized ? "YES" : "NO",
        settings->mode == FsModeField ? "Field" : "Learn",
        fs_severity_name(settings->severity),
        fs_uart_baud_label(settings->uart_baud_index),
        fs_uart_duration_label(settings->uart_duration_index),
        (unsigned)evidence, (unsigned)findings, (unsigned)retests);
    return write_all(storage, FS_SUMMARY, out, FSOM_CREATE_ALWAYS);
}
