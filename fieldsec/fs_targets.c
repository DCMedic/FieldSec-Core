#include "fieldsec.h"
#include <stdio.h>
#include <string.h>

#define FS_DIR APP_DATA_PATH("fieldsec")
#define FS_TARGETS FS_DIR "/targets.txt"
#define FS_PROFILES FS_DIR "/target_profiles.tsv"

static void fs_targets_mkdir(Storage* storage) {
    storage_common_mkdir(storage, FS_DIR);
}

static void clean_field(char* dst, size_t dst_size, const char* src) {
    if(!dst || !dst_size) return;
    size_t j = 0;
    for(size_t i = 0; src && src[i] && j + 1 < dst_size; i++) {
        char c = src[i];
        if(c == '\t' || c == '\r' || c == '\n') c = ' ';
        dst[j++] = c;
    }
    dst[j] = '\0';
}

size_t fs_targets_load(Storage* storage, char targets[][FS_TARGET_MAX], size_t max_targets) {
    if(!storage || !targets || !max_targets) return 0;
    File* file = storage_file_alloc(storage);
    if(!file) return 0;
    if(!storage_file_open(file, FS_TARGETS, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return 0;
    }
    char buf[FS_MAX_TARGETS * FS_TARGET_MAX];
    size_t read = storage_file_read(file, buf, sizeof(buf) - 1);
    buf[read] = '\0';
    storage_file_close(file);
    storage_file_free(file);

    size_t count = 0;
    char* save = NULL;
    char* line = strtok_r(buf, "\n", &save);
    while(line && count < max_targets) {
        size_t len = strlen(line);
        while(len && (line[len - 1] == '\r' || line[len - 1] == '\n')) line[--len] = '\0';
        if(len) {
            snprintf(targets[count], FS_TARGET_MAX, "%s", line);
            count++;
        }
        line = strtok_r(NULL, "\n", &save);
    }
    return count;
}

bool fs_target_add(Storage* storage, const char* target) {
    if(!storage || !target || !target[0]) return false;
    fs_targets_mkdir(storage);
    char safe[FS_TARGET_MAX];
    clean_field(safe, sizeof(safe), target);
    File* file = storage_file_alloc(storage);
    if(!file) return false;
    bool ok = storage_file_open(file, FS_TARGETS, FSAM_WRITE, FSOM_OPEN_APPEND);
    if(ok) {
        char line[FS_TARGET_MAX + 2];
        int n = snprintf(line, sizeof(line), "%s\n", safe);
        ok = n > 0 && storage_file_write(file, line, (size_t)n) == (size_t)n;
        storage_file_sync(file);
    }
    storage_file_close(file);
    storage_file_free(file);
    if(ok) {
        FsTargetProfile profile = {0};
        snprintf(profile.name, sizeof(profile.name), "%s", safe);
        fs_target_profile_save(storage, &profile);
    }
    return ok;
}

static bool parse_profile_line(char* line, FsTargetProfile* profile) {
    if(!line || !profile) return false;
    char* fields[7] = {0};
    size_t count = 0;
    fields[count++] = line;
    for(char* p = line; *p && count < COUNT_OF(fields); p++) {
        if(*p == '\t') {
            *p = '\0';
            fields[count++] = p + 1;
        }
    }
    if(!fields[0] || !fields[0][0]) return false;
    memset(profile, 0, sizeof(*profile));
    clean_field(profile->name, sizeof(profile->name), fields[0]);
    if(count > 1) clean_field(profile->type, sizeof(profile->type), fields[1]);
    if(count > 2) clean_field(profile->ip, sizeof(profile->ip), fields[2]);
    if(count > 3) clean_field(profile->mac, sizeof(profile->mac), fields[3]);
    if(count > 4) clean_field(profile->firmware, sizeof(profile->firmware), fields[4]);
    if(count > 5) clean_field(profile->interfaces, sizeof(profile->interfaces), fields[5]);
    if(count > 6) clean_field(profile->notes, sizeof(profile->notes), fields[6]);
    return true;
}

bool fs_target_profile_load(Storage* storage, const char* target, FsTargetProfile* profile) {
    if(!storage || !target || !profile) return false;
    memset(profile, 0, sizeof(*profile));
    snprintf(profile->name, sizeof(profile->name), "%s", target);
    File* file = storage_file_alloc(storage);
    if(!file) return false;
    if(!storage_file_open(file, FS_PROFILES, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_free(file);
        return false;
    }
    char buf[8192];
    size_t read = storage_file_read(file, buf, sizeof(buf) - 1);
    buf[read] = '\0';
    storage_file_close(file);
    storage_file_free(file);
    char* save = NULL;
    char* line = strtok_r(buf, "\n", &save);
    while(line) {
        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);
        FsTargetProfile candidate;
        if(parse_profile_line(copy, &candidate) && strcmp(candidate.name, target) == 0) {
            *profile = candidate;
            return true;
        }
        line = strtok_r(NULL, "\n", &save);
    }
    return false;
}

bool fs_target_profile_save(Storage* storage, const FsTargetProfile* profile) {
    if(!storage || !profile || !profile->name[0]) return false;
    fs_targets_mkdir(storage);

    char existing[8192] = {0};
    File* read_file = storage_file_alloc(storage);
    if(read_file && storage_file_open(read_file, FS_PROFILES, FSAM_READ, FSOM_OPEN_EXISTING)) {
        size_t read = storage_file_read(read_file, existing, sizeof(existing) - 1);
        existing[read] = '\0';
        storage_file_close(read_file);
    }
    if(read_file) storage_file_free(read_file);

    File* out = storage_file_alloc(storage);
    if(!out || !storage_file_open(out, FS_PROFILES, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        if(out) storage_file_free(out);
        return false;
    }

    char* save = NULL;
    char* line = strtok_r(existing, "\n", &save);
    while(line) {
        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);
        FsTargetProfile candidate;
        bool same = parse_profile_line(copy, &candidate) && strcmp(candidate.name, profile->name) == 0;
        if(!same && line[0]) {
            storage_file_write(out, line, strlen(line));
            storage_file_write(out, "\n", 1);
        }
        line = strtok_r(NULL, "\n", &save);
    }

    char name[FS_TARGET_MAX], type[FS_PROFILE_FIELD_MAX], ip[FS_PROFILE_FIELD_MAX], mac[FS_PROFILE_FIELD_MAX];
    char firmware[FS_PROFILE_FIELD_MAX], interfaces[FS_PROFILE_FIELD_MAX], notes[FS_NOTE_MAX];
    clean_field(name, sizeof(name), profile->name);
    clean_field(type, sizeof(type), profile->type);
    clean_field(ip, sizeof(ip), profile->ip);
    clean_field(mac, sizeof(mac), profile->mac);
    clean_field(firmware, sizeof(firmware), profile->firmware);
    clean_field(interfaces, sizeof(interfaces), profile->interfaces);
    clean_field(notes, sizeof(notes), profile->notes);
    char record[768];
    int n = snprintf(record, sizeof(record), "%s\t%s\t%s\t%s\t%s\t%s\t%s\n", name, type, ip, mac, firmware, interfaces, notes);
    bool ok = n > 0 && storage_file_write(out, record, (size_t)n) == (size_t)n;
    storage_file_sync(out);
    storage_file_close(out);
    storage_file_free(out);
    return ok;
}
