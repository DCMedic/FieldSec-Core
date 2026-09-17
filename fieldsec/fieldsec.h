#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>
#include <storage/storage.h>

#define FS_VERSION "1.0.0"
#define FS_NAME_MAX 48
#define FS_TARGET_MAX 48
#define FS_SCOPE_MAX 96
#define FS_NOTE_MAX 192
#define FS_FINDING_MAX 64
#define FS_PROFILE_FIELD_MAX 80
#define FS_MAX_TARGETS 12
#define FS_UART_CAPTURE_MAX 1024
#define FS_UART_HEX_MAX 768
#define FS_MAX_LESSONS 8
#define FS_ID_MAX 20

typedef enum {
    FsRiskPassive,
    FsRiskLocal,
    FsRiskActiveSafe,
} FsRisk;

typedef enum {
    FsModeLearn = 0,
    FsModeField = 1,
} FsMode;

typedef enum {
    FsSkillBeginner = 0,
    FsSkillIntermediate,
    FsSkillAdvanced,
    FsSkillExpert,
    FsSkillCount,
} FsSkillLevel;

typedef enum {
    FsSeverityInfo = 0,
    FsSeverityLow,
    FsSeverityMedium,
    FsSeverityHigh,
    FsSeverityCritical,
    FsSeverityCount,
} FsSeverity;

typedef struct {
    const char* name;
    const char* category;
    const char* purpose;
    const char* setup;
    const char* analysis;
    const char* mitigation;
    const char* next_step;
    FsRisk risk;
} FsModule;

typedef struct {
    char engagement[FS_NAME_MAX];
    char target[FS_TARGET_MAX];
    char scope[FS_SCOPE_MAX];
    bool authorized;
} FsEngagement;

typedef enum {
    FsDomainHardware = 0,
    FsDomainWireless,
    FsDomainCredential,
    FsDomainEvidence,
    FsDomainThreatModeling,
    FsDomainCount,
} FsSkillDomain;

typedef struct {
    FsMode mode;
    FsSkillLevel skill_level;
    FsSkillLevel domain_skill[FsDomainCount];
    bool adaptive_guidance;
    bool setup_complete;
    FsSeverity severity;
    uint8_t uart_baud_index;
    uint8_t uart_duration_index;
} FsSettings;

typedef struct {
    char name[FS_TARGET_MAX];
    char type[FS_PROFILE_FIELD_MAX];
    char ip[FS_PROFILE_FIELD_MAX];
    char mac[FS_PROFILE_FIELD_MAX];
    char firmware[FS_PROFILE_FIELD_MAX];
    char interfaces[FS_PROFILE_FIELD_MAX];
    char notes[FS_NOTE_MAX];
} FsTargetProfile;

typedef enum {
    FsLessonNotStarted = 0,
    FsLessonInProgress = 1,
    FsLessonComplete = 2,
} FsLessonStatus;

typedef struct {
    const char* title;
    const char* domain;
    const char* objective;
    const char* concept;
    const char* setup;
    const char* practice;
    const char* interpret;
    const char* defend;
    const char* checkpoint;
    int8_t paired_instrument;
} FsLesson;

typedef enum {
    FsInputNone = 0,
    FsInputEngagement,
    FsInputTarget,
    FsInputScope,
    FsInputFindingTitle,
    FsInputFindingCondition,
    FsInputFindingEvidence,
    FsInputFindingImpact,
    FsInputFindingRemediation,
    FsInputFindingRetest,
    FsInputNewTarget,
    FsInputTargetType,
    FsInputTargetIp,
    FsInputTargetMac,
    FsInputTargetFirmware,
    FsInputTargetInterfaces,
    FsInputTargetNotes,
    FsInputRetestFindingId,
    FsInputRetestResult,
    FsInputRetestNote,
    FsInputHypStatement,
    FsInputHypTestPlan,
    FsInputHypExpected,
    FsInputHypObserved,
    FsInputHypStatus,
    FsInputJournal,
    FsInputAnnotationObs,
    FsInputAnnotationText,
    FsInputInterfaceMap,
    FsInputServiceMap,
} FsInputPurpose;

typedef struct {
    Gui* gui;
    ViewDispatcher* dispatcher;
    Submenu* submenu;
    Widget* widget;
    TextInput* text_input;
    VariableItemList* settings_list;
    Storage* storage;

    uint32_t selected;
    uint32_t current_menu;
    uint32_t active_view;
    FsInputPurpose input_purpose;
    FsEngagement engagement;
    FsSettings settings;

    char input_buffer[FS_NOTE_MAX];
    char finding_title[FS_FINDING_MAX];
    char finding_condition[FS_NOTE_MAX];
    char finding_evidence[FS_NOTE_MAX];
    char finding_impact[FS_NOTE_MAX];
    char finding_remediation[FS_NOTE_MAX];
    char finding_retest[FS_NOTE_MAX];
    char retest_finding_id[FS_ID_MAX];
    char retest_result[FS_PROFILE_FIELD_MAX];
    char retest_note[FS_NOTE_MAX];
    char hyp_statement[FS_NOTE_MAX];
    char hyp_test_plan[FS_NOTE_MAX];
    char hyp_expected[FS_NOTE_MAX];
    char hyp_observed[FS_NOTE_MAX];
    char hyp_status[FS_PROFILE_FIELD_MAX];
    char annotation_obs[FS_ID_MAX];

    char targets[FS_MAX_TARGETS][FS_TARGET_MAX];
    size_t target_count;
    FsTargetProfile target_profile;

    uint8_t lesson_status[FS_MAX_LESSONS];
    uint8_t lesson_practice[FS_MAX_LESSONS];
    size_t selected_lesson;
} FieldSecApp;

const FsModule* fs_catalog_get(size_t* count);
const char* fs_risk_name(FsRisk risk);
const char* fs_severity_name(FsSeverity severity);
const char* fs_skill_name(FsSkillLevel level);
const char* fs_domain_name(FsSkillDomain domain);
bool fs_curriculum_render(const FsSettings* settings, const FsTargetProfile* profile, char* output, size_t output_size);
bool fs_target_graph_render(Storage* storage, const FsEngagement* engagement, const FsTargetProfile* profile, char* output, size_t output_size);

bool fs_state_load(Storage* storage, FsEngagement* engagement, FsSettings* settings);
bool fs_state_save(Storage* storage, const FsEngagement* engagement, const FsSettings* settings);

bool fs_evidence_append(Storage* storage, const FsEngagement* engagement, const FsModule* module, const char* note);
bool fs_evidence_append_event(Storage* storage, const FsEngagement* engagement, const char* category, const char* tool, FsRisk risk, const char* note);

bool fs_finding_append_structured(
    Storage* storage,
    const FsEngagement* engagement,
    const char* title,
    FsSeverity severity,
    const char* condition,
    const char* evidence,
    const char* impact,
    const char* remediation,
    const char* retest);

bool fs_export_summary(Storage* storage, const FsEngagement* engagement, const FsSettings* settings);

bool fs_evidence_append_event_id(
    Storage* storage,
    const FsEngagement* engagement,
    const char* category,
    const char* tool,
    FsRisk risk,
    const char* note,
    char* out_id,
    size_t out_id_size);

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
    size_t out_id_size);

bool fs_retest_append(
    Storage* storage,
    const FsEngagement* engagement,
    const char* finding_id,
    const char* result,
    const char* note,
    char* out_id,
    size_t out_id_size);

bool fs_target_stats(Storage* storage, const char* target, size_t* evidence, size_t* findings, size_t* retests);
bool fs_timeline_render(Storage* storage, const char* target, char* output, size_t output_size);
uint8_t fs_capstone_score(Storage* storage, const FsEngagement* engagement, const uint8_t lesson_status[], size_t lesson_count, char* detail, size_t detail_size);

size_t fs_targets_load(Storage* storage, char targets[][FS_TARGET_MAX], size_t max_targets);
bool fs_target_add(Storage* storage, const char* target);
bool fs_target_profile_load(Storage* storage, const char* target, FsTargetProfile* profile);
bool fs_target_profile_save(Storage* storage, const FsTargetProfile* profile);

bool fs_training_load(Storage* storage, uint8_t status[], uint8_t practice[], size_t max_lessons);
bool fs_training_save(Storage* storage, const uint8_t status[], const uint8_t practice[], size_t count);
const FsLesson* fs_lessons_get(size_t* count);
const char* fs_lesson_status_name(uint8_t status);

uint32_t fs_uart_baud_from_index(uint8_t index);
const char* fs_uart_baud_label(uint8_t index);
uint32_t fs_uart_duration_ms_from_index(uint8_t index);
const char* fs_uart_duration_label(uint8_t index);
bool fs_uart_capture(
    Storage* storage,
    uint32_t baud,
    uint32_t duration_ms,
    char* preview,
    size_t preview_size,
    char* hex_preview,
    size_t hex_preview_size,
    size_t* bytes_captured,
    uint8_t* printable_percent);
size_t fs_i2c_scan_compare(Storage* storage, char* output, size_t output_size, size_t* added, size_t* removed);

bool fs_uart_history_render(Storage* storage, char* output, size_t output_size);
bool fs_uart_extract_strings(Storage* storage, char* output, size_t output_size);
bool fs_i2c_history_render(Storage* storage, char* output, size_t output_size);
bool fs_wifi_history_render(Storage* storage, char* output, size_t output_size);
bool fs_target_workspace_render(Storage* storage, const char* target, char* output, size_t output_size);
bool fs_after_action_render(Storage* storage, const FsEngagement* engagement, const uint8_t lesson_status[], size_t lesson_count, char* output, size_t output_size);

// v1.0 target intelligence and guided hypothesis management
bool fs_hypothesis_append(Storage* storage, const FsEngagement* engagement, const char* statement, const char* test_plan, const char* expected, const char* observed, const char* status, char* out_id, size_t out_id_size);
bool fs_hypothesis_render(Storage* storage, const char* target, char* output, size_t output_size);
bool fs_journal_append(Storage* storage, const FsEngagement* engagement, const char* note, char* out_id, size_t out_id_size);
bool fs_journal_render(Storage* storage, const char* target, char* output, size_t output_size);
bool fs_annotation_append(Storage* storage, const FsEngagement* engagement, const char* obs_id, const char* note, char* out_id, size_t out_id_size);
bool fs_annotation_render(Storage* storage, const char* target, char* output, size_t output_size);
bool fs_interface_map_append(Storage* storage, const FsEngagement* engagement, const char* interface_note);
bool fs_service_map_append(Storage* storage, const FsEngagement* engagement, const char* service_note);
bool fs_target_map_render(Storage* storage, const char* target, char* output, size_t output_size);
bool fs_instructor_review_render(Storage* storage, const FsEngagement* engagement, char* output, size_t output_size);

bool fs_wifi_board_scan_compare(
    Storage* storage,
    char* output,
    size_t output_size,
    size_t* ap_count,
    size_t* open_count,
    size_t* new_count,
    size_t* disappeared_count);

/* v1.0 adaptive instruction / target correlation */
bool fs_adaptive_render(const FsSettings* settings, const FsTargetProfile* profile, const uint8_t lesson_status[], const uint8_t lesson_practice[], size_t lesson_count, char* output, size_t output_size);

/* v1.0 first-release closure */
bool fs_gpio_snapshot(Storage* storage, char* output, size_t output_size);
bool fs_competency_render(Storage* storage, const FsEngagement* engagement, const FsSettings* settings, const uint8_t lesson_status[], const uint8_t lesson_practice[], size_t lesson_count, char* output, size_t output_size);
bool fs_assessment_plan_render(Storage* storage, const FsEngagement* engagement, const FsSettings* settings, const FsTargetProfile* profile, char* output, size_t output_size);
