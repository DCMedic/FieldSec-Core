#include "fieldsec.h"
#include <expansion/expansion.h>
#include <furi_hal_i2c.h>
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>
#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>
#include <stdio.h>
#include <string.h>

#define FS_DIR APP_DATA_PATH("fieldsec")
#define FS_UART_CAPTURE FS_DIR "/uart_capture.bin"
#define FS_UART_HISTORY FS_DIR "/uart_history.csv"
#define FS_I2C_LAST FS_DIR "/i2c_last.bin"
#define FS_I2C_HISTORY FS_DIR "/i2c_history.log"
#define FS_WIFI_CAPTURE FS_DIR "/wifi_scan.txt"
#define FS_WIFI_BSSID_LAST FS_DIR "/wifi_bssid_last.txt"
#define FS_WIFI_DETAIL_LAST FS_DIR "/wifi_detail_last.txt"
#define FS_WIFI_HISTORY FS_DIR "/wifi_history.log"

static const uint32_t fs_bauds[] = {9600, 19200, 38400, 57600, 115200};
static const char* fs_baud_labels[] = {"9600", "19200", "38400", "57600", "115200"};
static const uint32_t fs_durations_ms[] = {1000, 3000, 10000};
static const char* fs_duration_labels[] = {"1 sec", "3 sec", "10 sec"};

uint32_t fs_uart_baud_from_index(uint8_t index) {
    if(index >= COUNT_OF(fs_bauds)) index = COUNT_OF(fs_bauds) - 1;
    return fs_bauds[index];
}
const char* fs_uart_baud_label(uint8_t index) {
    if(index >= COUNT_OF(fs_baud_labels)) index = COUNT_OF(fs_baud_labels) - 1;
    return fs_baud_labels[index];
}
uint32_t fs_uart_duration_ms_from_index(uint8_t index) {
    if(index >= COUNT_OF(fs_durations_ms)) index = 1;
    return fs_durations_ms[index];
}
const char* fs_uart_duration_label(uint8_t index) {
    if(index >= COUNT_OF(fs_duration_labels)) index = 1;
    return fs_duration_labels[index];
}

typedef struct { FuriStreamBuffer* stream; } FsUartContext;

static void fs_uart_rx_cb(FuriHalSerialHandle* handle, FuriHalSerialRxEvent event, void* context) {
    FsUartContext* ctx = context;
    if(!ctx || !(event & FuriHalSerialRxEventData)) return;
    while(furi_hal_serial_async_rx_available(handle)) {
        uint8_t b = furi_hal_serial_async_rx(handle);
        furi_stream_buffer_send(ctx->stream, &b, 1, 0);
    }
}

static void fs_preview_append(char* out, size_t out_size, size_t* used, const uint8_t* data, size_t len, size_t* printable) {
    if(!out || !used || *used >= out_size) return;
    for(size_t i = 0; i < len; i++) {
        uint8_t c = data[i];
        bool is_printable = c == '\r' || c == '\n' || c == '\t' || (c >= 32 && c <= 126);
        if(is_printable && printable) (*printable)++;
        if(*used + 1 >= out_size) continue;
        if(c == '\r') continue;
        out[(*used)++] = (c == '\n' || c == '\t' || (c >= 32 && c <= 126)) ? (char)c : '.';
    }
    out[*used] = '\0';
}

static void fs_hex_append(char* out, size_t out_size, size_t* used, const uint8_t* data, size_t len) {
    if(!out || !used || *used >= out_size) return;
    for(size_t i = 0; i < len && *used + 4 < out_size; i++) {
        int n = snprintf(out + *used, out_size - *used, "%02X%s", data[i], ((i + 1) % 12 == 0) ? "\n" : " ");
        if(n <= 0 || (size_t)n >= out_size - *used) break;
        *used += (size_t)n;
    }
}

static bool append_text(Storage* storage, const char* path, const char* text) {
    File* file = storage_file_alloc(storage);
    if(!file) return false;
    bool ok = storage_file_open(file, path, FSAM_WRITE, FSOM_OPEN_APPEND);
    if(ok) {
        size_t len = strlen(text);
        ok = storage_file_write(file, text, len) == len;
        storage_file_sync(file);
    }
    storage_file_close(file);
    storage_file_free(file);
    return ok;
}

static size_t fs_read_text_file(Storage* storage, const char* path, char* out, size_t out_size) {
    if(!storage || !out || out_size < 2) return 0;
    out[0] = '\0';
    File* file = storage_file_alloc(storage);
    if(!file) return 0;
    if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) { storage_file_free(file); return 0; }
    size_t n = storage_file_read(file, out, out_size - 1);
    out[n] = '\0';
    storage_file_close(file);
    storage_file_free(file);
    return n;
}

static void fs_tail_lines(const char* text, size_t max_lines, char* out, size_t out_size) {
    if(!out || out_size < 2) return;
    out[0] = '\0';
    if(!text || !text[0]) return;
    const char* starts[16] = {0};
    size_t count = 0;
    const char* p = text;
    starts[count++] = p;
    while(*p && count < COUNT_OF(starts)) { if(*p++ == '\n' && *p) starts[count++] = p; }
    size_t first = count > max_lines ? count - max_lines : 0;
    size_t used = 0;
    for(size_t i = first; i < count && used + 2 < out_size; i++) {
        const char* e = strchr(starts[i], '\n');
        size_t n = e ? (size_t)(e - starts[i]) : strlen(starts[i]);
        if(n > out_size - used - 2) n = out_size - used - 2;
        memcpy(out + used, starts[i], n); used += n;
        out[used++] = '\n'; out[used] = '\0';
    }
}

bool fs_uart_capture(
    Storage* storage,
    uint32_t baud,
    uint32_t duration_ms,
    char* preview,
    size_t preview_size,
    char* hex_preview,
    size_t hex_preview_size,
    size_t* bytes_captured,
    uint8_t* printable_percent) {
    if(preview && preview_size) preview[0] = '\0';
    if(hex_preview && hex_preview_size) hex_preview[0] = '\0';
    if(bytes_captured) *bytes_captured = 0;
    if(printable_percent) *printable_percent = 0;
    if(!storage || !preview || preview_size < 2 || !hex_preview || hex_preview_size < 2) return false;

    storage_common_mkdir(storage, FS_DIR);
    Expansion* expansion = furi_record_open(RECORD_EXPANSION);
    expansion_disable(expansion);
    FuriHalSerialHandle* serial = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    if(!serial) {
        expansion_enable(expansion);
        furi_record_close(RECORD_EXPANSION);
        snprintf(preview, preview_size, "USART is busy. Close other GPIO/serial tools and retry.");
        return false;
    }

    FuriStreamBuffer* stream = furi_stream_buffer_alloc(512, 1);
    if(!stream) {
        furi_hal_serial_control_release(serial);
        expansion_enable(expansion);
        furi_record_close(RECORD_EXPANSION);
        snprintf(preview, preview_size, "Could not allocate UART receive buffer.");
        return false;
    }
    FsUartContext ctx = {.stream = stream};
    furi_hal_serial_init(serial, baud);
    furi_hal_serial_disable_direction(serial, FuriHalSerialDirectionTx);
    furi_hal_serial_async_rx_start(serial, fs_uart_rx_cb, &ctx, true);

    File* file = storage_file_alloc(storage);
    bool file_ok = file && storage_file_open(file, FS_UART_CAPTURE, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    size_t preview_used = 0, hex_used = 0, total = 0, printable = 0;
    uint8_t chunk[96];
    if(duration_ms < 200) duration_ms = 200;
    size_t steps = duration_ms / 20;
    for(size_t step = 0; step < steps; step++) {
        size_t got = furi_stream_buffer_receive(stream, chunk, sizeof(chunk), 0);
        if(got) {
            if(file_ok) storage_file_write(file, chunk, got);
            total += got;
            fs_preview_append(preview, preview_size, &preview_used, chunk, got, &printable);
            fs_hex_append(hex_preview, hex_preview_size, &hex_used, chunk, got);
        }
        furi_delay_ms(20);
    }
    while(furi_stream_buffer_bytes_available(stream)) {
        size_t got = furi_stream_buffer_receive(stream, chunk, sizeof(chunk), 0);
        if(!got) break;
        if(file_ok) storage_file_write(file, chunk, got);
        total += got;
        fs_preview_append(preview, preview_size, &preview_used, chunk, got, &printable);
        fs_hex_append(hex_preview, hex_preview_size, &hex_used, chunk, got);
    }

    furi_hal_serial_async_rx_stop(serial);
    furi_hal_serial_deinit(serial);
    furi_hal_serial_control_release(serial);
    expansion_enable(expansion);
    furi_record_close(RECORD_EXPANSION);
    if(file) {
        if(file_ok) storage_file_sync(file);
        storage_file_close(file);
        storage_file_free(file);
    }
    furi_stream_buffer_free(stream);

    uint8_t pct = total ? (uint8_t)((printable * 100U) / total) : 0;
    if(bytes_captured) *bytes_captured = total;
    if(printable_percent) *printable_percent = pct;
    if(!total) snprintf(preview, preview_size, "No serial bytes observed during the receive-only window.");

    char history[160];
    snprintf(history, sizeof(history), "%lu,%lu,%u,%u\n", (unsigned long)baud, (unsigned long)duration_ms, (unsigned)total, (unsigned)pct);
    append_text(storage, FS_UART_HISTORY, history);
    return file_ok;
}

bool fs_uart_history_render(Storage* storage, char* output, size_t output_size) {
    char buf[2400];
    if(!fs_read_text_file(storage, FS_UART_HISTORY, buf, sizeof(buf))) { snprintf(output, output_size, "No UART sessions recorded yet."); return true; }
    char tail[1600]; fs_tail_lines(buf, 8, tail, sizeof(tail));
    snprintf(output, output_size, "Recent UART sessions\nbaud,duration_ms,bytes,printable%%\n\n%s\nANALYSIS\nLook for repeatable byte counts and printable ratios across baud settings. A high printable ratio suggests plausible framing, but does not prove protocol correctness.", tail);
    return true;
}

bool fs_uart_extract_strings(Storage* storage, char* output, size_t output_size) {
    if(!storage || !output || output_size < 2) return false;
    output[0] = '\0';
    File* f = storage_file_alloc(storage);
    if(!f || !storage_file_open(f, FS_UART_CAPTURE, FSAM_READ, FSOM_OPEN_EXISTING)) { if(f) storage_file_free(f); snprintf(output, output_size, "No UART capture available."); return true; }
    uint8_t buf[768]; size_t n = storage_file_read(f, buf, sizeof(buf)); storage_file_close(f); storage_file_free(f);
    size_t used = 0, run = 0; char temp[96]; size_t strings = 0;
    for(size_t i = 0; i <= n; i++) {
        bool printable = i < n && buf[i] >= 32 && buf[i] <= 126;
        if(printable && run + 1 < sizeof(temp)) temp[run++] = (char)buf[i];
        else {
            if(run >= 4) { temp[run] = '\0'; int w = snprintf(output + used, output_size - used, "%s%s", strings ? "\n" : "", temp); if(w > 0 && (size_t)w < output_size - used) used += (size_t)w; strings++; }
            run = 0;
            if(used + 8 >= output_size) break;
        }
    }
    if(!strings) snprintf(output, output_size, "No printable strings of length >=4 found in the current capture.");
    return true;
}

static bool read_exact(Storage* storage, const char* path, void* data, size_t len) {
    File* file = storage_file_alloc(storage);
    if(!file) return false;
    bool ok = storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING);
    if(ok) ok = storage_file_read(file, data, len) == len;
    storage_file_close(file);
    storage_file_free(file);
    return ok;
}

static bool write_exact(Storage* storage, const char* path, const void* data, size_t len) {
    File* file = storage_file_alloc(storage);
    if(!file) return false;
    bool ok = storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    if(ok) {
        ok = storage_file_write(file, data, len) == len;
        storage_file_sync(file);
    }
    storage_file_close(file);
    storage_file_free(file);
    return ok;
}

size_t fs_i2c_scan_compare(Storage* storage, char* output, size_t output_size, size_t* added, size_t* removed) {
    if(added) *added = 0;
    if(removed) *removed = 0;
    if(!storage || !output || output_size < 2) return 0;
    output[0] = '\0';
    storage_common_mkdir(storage, FS_DIR);

    uint8_t current[128] = {0};
    uint8_t previous[128] = {0};
    bool have_previous = read_exact(storage, FS_I2C_LAST, previous, sizeof(previous));
    size_t found = 0, used = 0;
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    for(uint8_t addr = 0x08; addr <= 0x77; addr++) {
        if(furi_hal_i2c_is_device_ready(&furi_hal_i2c_handle_external, (uint8_t)(addr << 1), 10)) {
            current[addr] = 1;
            int n = snprintf(output + used, output_size - used, "0x%02X%s", addr, found % 5 == 4 ? "\n" : "  ");
            if(n > 0 && (size_t)n < output_size - used) used += (size_t)n;
            found++;
        }
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);

    size_t a = 0, r = 0;
    if(have_previous) {
        for(size_t i = 0x08; i <= 0x77; i++) {
            if(current[i] && !previous[i]) a++;
            if(!current[i] && previous[i]) r++;
        }
    }
    if(added) *added = a;
    if(removed) *removed = r;
    write_exact(storage, FS_I2C_LAST, current, sizeof(current));
    char history[640]; size_t hu = 0;
    int hn = snprintf(history, sizeof(history), "count=%u added=%u removed=%u addresses=", (unsigned)found, (unsigned)a, (unsigned)r);
    if(hn > 0) hu = (size_t)hn;
    for(size_t i = 0x08; i <= 0x77 && hu + 6 < sizeof(history); i++) if(current[i]) { hn = snprintf(history + hu, sizeof(history) - hu, "%s0x%02X", hu && history[hu-1] != '=' ? "," : "", (unsigned)i); if(hn > 0) hu += (size_t)hn; }
    if(hu + 2 < sizeof(history)) { history[hu++]='\n'; history[hu]='\0'; append_text(storage, FS_I2C_HISTORY, history); }
    if(found == 0) snprintf(output, output_size, "No responding 7-bit I2C addresses found.\nVerify 3.3V logic, common ground, SDA/SCL wiring, and target power.");
    return found;
}

bool fs_i2c_history_render(Storage* storage, char* output, size_t output_size) {
    char buf[3200];
    if(!fs_read_text_file(storage, FS_I2C_HISTORY, buf, sizeof(buf))) { snprintf(output, output_size, "No I2C topology history yet."); return true; }
    char tail[2200]; fs_tail_lines(buf, 8, tail, sizeof(tail));
    snprintf(output, output_size, "I2C topology history\n\n%s\nINTERPRET\nRepeated presence suggests stable topology. Added/missing addresses are change observations only; validate power, mux state, wiring, and firmware behavior before assigning security meaning.", tail);
    return true;
}


static void fs_line_sanitize(char* s) {
    if(!s) return;
    for(size_t i = 0; s[i]; i++) if(s[i] == '\r' || s[i] == '\n') s[i] = '\0';
}

static bool bssid_in_text(const char* text, const char* bssid) {
    if(!text || !bssid || !bssid[0]) return false;
    const char* p = text;
    size_t n = strlen(bssid);
    while((p = strstr(p, bssid))) {
        bool left = p == text || p[-1] == '\n';
        bool right = p[n] == '\0' || p[n] == '\n';
        if(left && right) return true;
        p += n;
    }
    return false;
}

bool fs_wifi_board_scan_compare(
    Storage* storage,
    char* output,
    size_t output_size,
    size_t* ap_count,
    size_t* open_count,
    size_t* new_count,
    size_t* disappeared_count) {
    if(output && output_size) output[0] = '\0';
    if(ap_count) *ap_count = 0;
    if(open_count) *open_count = 0;
    if(new_count) *new_count = 0;
    if(disappeared_count) *disappeared_count = 0;
    if(!storage || !output || output_size < 32) return false;

    storage_common_mkdir(storage, FS_DIR);
    char previous_bssids[1200] = {0};
    File* prev = storage_file_alloc(storage);
    if(prev && storage_file_open(prev, FS_WIFI_BSSID_LAST, FSAM_READ, FSOM_OPEN_EXISTING)) {
        size_t n = storage_file_read(prev, previous_bssids, sizeof(previous_bssids) - 1);
        previous_bssids[n] = '\0';
        storage_file_close(prev);
    }
    if(prev) storage_file_free(prev);

    Expansion* expansion = furi_record_open(RECORD_EXPANSION);
    expansion_disable(expansion);
    FuriHalSerialHandle* serial = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    if(!serial) {
        expansion_enable(expansion);
        furi_record_close(RECORD_EXPANSION);
        snprintf(output, output_size, "USART busy. Close other serial/GPIO tools and retry.");
        return false;
    }
    FuriStreamBuffer* stream = furi_stream_buffer_alloc(1024, 1);
    if(!stream) {
        furi_hal_serial_control_release(serial);
        expansion_enable(expansion);
        furi_record_close(RECORD_EXPANSION);
        snprintf(output, output_size, "Could not allocate Wi-Fi telemetry buffer.");
        return false;
    }
    FsUartContext ctx = {.stream = stream};
    furi_hal_serial_init(serial, 115200);
    furi_hal_serial_disable_direction(serial, FuriHalSerialDirectionTx);
    furi_hal_serial_async_rx_start(serial, fs_uart_rx_cb, &ctx, true);

    File* file = storage_file_alloc(storage);
    bool file_ok = file && storage_file_open(file, FS_WIFI_CAPTURE, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    char line[192], current_bssids[1200] = {0}, current_details[2600] = {0}, previous_details[2600] = {0};
    fs_read_text_file(storage, FS_WIFI_DETAIL_LAST, previous_details, sizeof(previous_details));
    size_t line_used = 0, bssid_used = 0, detail_used = 0, aps = 0, opens = 0, out_used = 0, new_aps = 0, security_changes = 0;
    size_t channels[15] = {0};
    bool saw_ready = false, saw_end = false;
    uint8_t chunk[96];

    for(size_t step = 0; step < 850 && !saw_end; step++) {
        size_t got = furi_stream_buffer_receive(stream, chunk, sizeof(chunk), 0);
        if(got) {
            if(file_ok) storage_file_write(file, chunk, got);
            for(size_t i = 0; i < got; i++) {
                char c = (char)chunk[i];
                if(c == '\n') {
                    line[line_used] = '\0';
                    fs_line_sanitize(line);
                    if(strncmp(line, "FS2|READY|", 10) == 0) saw_ready = true;
                    if(strncmp(line, "FS2|AP|", 7) == 0) {
                        char copy[192];
                        snprintf(copy, sizeof(copy), "%s", line);
                        char* save = NULL;
                        strtok_r(copy, "|", &save);
                        strtok_r(NULL, "|", &save);
                        char* bssid = strtok_r(NULL, "|", &save);
                        char* rssi = strtok_r(NULL, "|", &save);
                        char* channel = strtok_r(NULL, "|", &save);
                        char* auth = strtok_r(NULL, "|", &save);
                        char* ssid = strtok_r(NULL, "|", &save);
                        aps++;
                        if(auth && strcmp(auth, "OPEN") == 0) opens++;
                        int ch = channel ? atoi(channel) : 0; if(ch >= 1 && ch <= 14) channels[ch]++;
                        bool is_new = bssid && previous_bssids[0] && !bssid_in_text(previous_bssids, bssid);
                        if(bssid && auth) {
                            char needle[96]; snprintf(needle, sizeof(needle), "%s|", bssid);
                            char* prior = strstr(previous_details, needle);
                            if(prior) { char expected[128]; snprintf(expected, sizeof(expected), "%s|%s|", bssid, auth); if(strstr(prior, expected) != prior) security_changes++; }
                            if(detail_used + 150 < sizeof(current_details)) { int dn = snprintf(current_details + detail_used, sizeof(current_details) - detail_used, "%s|%s|%s|%s|%s\n", bssid, auth, channel ? channel : "?", rssi ? rssi : "?", ssid && ssid[0] ? ssid : "<hidden>"); if(dn > 0) detail_used += (size_t)dn; }
                        }
                        if(is_new) new_aps++;
                        if(bssid && bssid_used + strlen(bssid) + 2 < sizeof(current_bssids)) {
                            int bn = snprintf(current_bssids + bssid_used, sizeof(current_bssids) - bssid_used, "%s\n", bssid);
                            if(bn > 0) bssid_used += (size_t)bn;
                        }
                        if(aps <= 8 && out_used + 40 < output_size) {
                            int n = snprintf(output + out_used, output_size - out_used, "%s%s ch%s %sdBm %s\n",
                                is_new ? "+ " : "", ssid && ssid[0] ? ssid : "<hidden>", channel ? channel : "?", rssi ? rssi : "?", auth ? auth : "?");
                            if(n > 0 && (size_t)n < output_size - out_used) out_used += (size_t)n;
                        }
                    } else if(strncmp(line, "FS2|SCAN_END|", 13) == 0) saw_end = true;
                    line_used = 0;
                } else if(c != '\r' && line_used + 1 < sizeof(line)) line[line_used++] = c;
            }
        }
        furi_delay_ms(20);
    }

    furi_hal_serial_async_rx_stop(serial);
    furi_hal_serial_deinit(serial);
    furi_hal_serial_control_release(serial);
    expansion_enable(expansion);
    furi_record_close(RECORD_EXPANSION);
    if(file) {
        if(file_ok) storage_file_sync(file);
        storage_file_close(file);
        storage_file_free(file);
    }
    furi_stream_buffer_free(stream);

    size_t disappeared = 0;
    if(previous_bssids[0]) {
        char temp[1200];
        snprintf(temp, sizeof(temp), "%s", previous_bssids);
        char* save = NULL;
        char* b = strtok_r(temp, "\n", &save);
        while(b) {
            if(!bssid_in_text(current_bssids, b)) disappeared++;
            b = strtok_r(NULL, "\n", &save);
        }
    }
    File* bfile = storage_file_alloc(storage);
    if(bfile && storage_file_open(bfile, FS_WIFI_BSSID_LAST, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_write(bfile, current_bssids, strlen(current_bssids));
        storage_file_sync(bfile);
        storage_file_close(bfile);
    }
    if(bfile) storage_file_free(bfile);
    write_exact(storage, FS_WIFI_DETAIL_LAST, current_details, strlen(current_details));
    char wh[900]; size_t wu = 0;
    int wn = snprintf(wh, sizeof(wh), "aps=%u open=%u new=%u disappeared=%u security_changes=%u channels=", (unsigned)aps, (unsigned)opens, (unsigned)new_aps, (unsigned)disappeared, (unsigned)security_changes); if(wn > 0) wu = (size_t)wn;
    for(size_t ch = 1; ch <= 14 && wu + 16 < sizeof(wh); ch++) if(channels[ch]) { wn = snprintf(wh + wu, sizeof(wh) - wu, "%s%u:%u", wh[wu-1]=='=' ? "" : ",", (unsigned)ch, (unsigned)channels[ch]); if(wn > 0) wu += (size_t)wn; }
    if(wu + 2 < sizeof(wh)) { wh[wu++]='\n'; wh[wu]='\0'; append_text(storage, FS_WIFI_HISTORY, wh); }

    if(ap_count) *ap_count = aps;
    if(open_count) *open_count = opens;
    if(new_count) *new_count = new_aps;
    if(disappeared_count) *disappeared_count = disappeared;
    if(!saw_ready && aps == 0) {
        snprintf(output, output_size, "No FS2 telemetry observed. Verify companion firmware, board seating, and 115200-baud UART console.");
        return false;
    }
    if(aps == 0 && saw_ready) snprintf(output, output_size, "Developer Board detected, but no AP records completed within the observation window.");
    return file_ok;
}


bool fs_wifi_history_render(Storage* storage, char* output, size_t output_size) {
    char buf[3600];
    if(!fs_read_text_file(storage, FS_WIFI_HISTORY, buf, sizeof(buf))) { snprintf(output, output_size, "No Wi-Fi survey history yet."); return true; }
    char tail[2400]; fs_tail_lines(buf, 8, tail, sizeof(tail));
    snprintf(output, output_size, "Passive Wi-Fi survey history\n\n%s\nANALYSIS\nTrack persistence, channel occupancy, and advertised security changes over repeated observations. Baseline deviation is a lead, not proof of malicious activity.", tail);
    return true;
}


bool fs_gpio_snapshot(Storage* storage, char* output, size_t output_size) {
    if(!output || output_size < 64) return false;
    output[0] = '\0';
    typedef struct { const char* name; const GpioPin* pin; } FsSnapshotPin;
    static const FsSnapshotPin pins[] = {
        {"PC0", &gpio_ext_pc0}, {"PC1", &gpio_ext_pc1}, {"PC3", &gpio_ext_pc3},
        {"PB2", &gpio_ext_pb2}, {"PB3", &gpio_ext_pb3}, {"PA4", &gpio_ext_pa4},
        {"PA6", &gpio_ext_pa6}, {"PA7", &gpio_ext_pa7},
    };
    bool initial[COUNT_OF(pins)], last[COUNT_OF(pins)];
    uint16_t transitions[COUNT_OF(pins)] = {0};
    for(size_t i=0;i<COUNT_OF(pins);i++) initial[i]=last[i]=furi_hal_gpio_read(pins[i].pin);
    /* Passive one-second observation. No pin configuration calls are made. */
    for(size_t sample=0; sample<100; sample++) {
        furi_delay_ms(10);
        for(size_t i=0;i<COUNT_OF(pins);i++) {
            bool now=furi_hal_gpio_read(pins[i].pin);
            if(now!=last[i]) { transitions[i]++; last[i]=now; }
        }
    }
    size_t used = 0;
    int n = snprintf(output, output_size,
        "GPIO MONITOR | passive 1s\n\nPin Ext# Start End Edges\n");
    if(n > 0) used = (size_t)n;
    for(size_t i = 0; i < COUNT_OF(pins) && used + 38 < output_size; i++) {
        int32_t ext = furi_hal_resources_get_ext_pin_number(pins[i].pin);
        n = snprintf(output + used, output_size - used, "%-3s %3ld   %s   %s  %u\n",
            pins[i].name, (long)ext, initial[i]?"H":"L", last[i]?"H":"L", (unsigned)transitions[i]);
        if(n <= 0 || (size_t)n >= output_size - used) break;
        used += (size_t)n;
    }
    snprintf(output + used, output_size - used,
        "\nSAFETY\nNo mode, pull, speed, alternate function, or output state was changed. Shared/analog/alternate-function pins may not produce externally meaningful digital states.\n\nINTERPRET\nEdges indicate sampled logic changes, not precise timing. Use a logic analyzer/oscilloscope for voltage, pulse width, frequency, or protocol claims.");
    if(storage) {
        storage_common_mkdir(storage, FS_DIR);
        char line[420]; size_t lu = 0;
        for(size_t i=0;i<COUNT_OF(pins) && lu + 36 < sizeof(line);i++) {
            int32_t ext=furi_hal_resources_get_ext_pin_number(pins[i].pin);
            int w=snprintf(line+lu,sizeof(line)-lu,"%s%s:%ld:%u>%u:e%u",i?",":"",pins[i].name,(long)ext,initial[i]?1U:0U,last[i]?1U:0U,(unsigned)transitions[i]);
            if(w>0) lu += (size_t)w;
        }
        if(lu + 2 < sizeof(line)) { line[lu++]='\n'; line[lu]='\0'; append_text(storage, FS_DIR "/gpio_history.log", line); }
    }
    return true;
}
