#include "fieldsec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    FsViewMenu = 0,
    FsViewDetail,
    FsViewTextInput,
    FsViewSettings,
} FsViewId;

typedef enum {
    FsMenuRoot = 0,
    FsMenuEngagement,
    FsMenuModules,
    FsMenuTargets,
    FsMenuInstruments,
    FsMenuTraining,
    FsMenuTargetProfile,
} FsMenuId;

typedef enum {
    FsRootDashboard = 0,
    FsRootEngagement,
    FsRootTargets,
    FsRootTargetIntel,
    FsRootWorkspace,
    FsRootHypotheses,
    FsRootJournal,
    FsRootAnnotations,
    FsRootTargetMap,
    FsRootInstructorReview,
    FsRootAdaptivePath,
    FsRootAdaptiveCurriculum,
    FsRootTargetGraph,
    FsRootCompetency,
    FsRootAssessmentPlan,
    FsRootNewHypothesis,
    FsRootNewJournal,
    FsRootNewAnnotation,
    FsRootMapInterface,
    FsRootMapService,
    FsRootInstruments,
    FsRootModules,
    FsRootTraining,
    FsRootFinding,
    FsRootRetest,
    FsRootTimeline,
    FsRootCapstone,
    FsRootAfterAction,
    FsRootExport,
    FsRootSettings,
    FsRootAbout,
} FsRootItem;

typedef enum {
    FsInstrumentUart = 0,
    FsInstrumentUartReview,
    FsInstrumentUartStrings,
    FsInstrumentI2c,
    FsInstrumentI2cHistory,
    FsInstrumentWifiBoard,
    FsInstrumentWifiHistory,
    FsInstrumentGpioGuide,
} FsInstrumentItem;

static void fs_build_menu(FieldSecApp* app, FsMenuId menu);
static void fs_show_dashboard(FieldSecApp* app);
static void fs_show_module(FieldSecApp* app);
static void fs_show_lesson(FieldSecApp* app);
static void fs_run_instrument(FieldSecApp* app, uint32_t instrument);
static void fs_show_engagement(FieldSecApp* app);
static void fs_show_target_profile(FieldSecApp* app);
static void fs_show_workspace(FieldSecApp* app);
static void fs_show_after_action(FieldSecApp* app);
static void fs_show_timeline(FieldSecApp* app);
static void fs_show_capstone(FieldSecApp* app);
static void fs_show_hypotheses(FieldSecApp* app);
static void fs_show_journal(FieldSecApp* app);
static void fs_show_annotations(FieldSecApp* app);
static void fs_show_target_map(FieldSecApp* app);
static void fs_show_instructor_review(FieldSecApp* app);
static void fs_show_adaptive_path(FieldSecApp* app);
static void fs_show_adaptive_curriculum(FieldSecApp* app);
static void fs_show_target_graph(FieldSecApp* app);
static void fs_show_competency(FieldSecApp* app);
static void fs_show_assessment_plan(FieldSecApp* app);
static void fs_start_text_input(FieldSecApp* app, FsInputPurpose purpose, const char* header, const char* initial);

static void fs_switch_view(FieldSecApp* app, uint32_t view_id) {
    app->active_view = view_id;
    view_dispatcher_switch_to_view(app->dispatcher, view_id);
}

static void fs_notice(FieldSecApp* app, const char* title, const char* body) {
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 3, AlignCenter, AlignTop, FontPrimary, title);
    widget_add_text_scroll_element(app->widget, 3, 15, 122, 36, body);
    fs_switch_view(app, FsViewDetail);
}

static void fs_save_state(FieldSecApp* app) {
    fs_state_save(app->storage, &app->engagement, &app->settings);
}

static void fs_refresh_targets(FieldSecApp* app) {
    memset(app->targets, 0, sizeof(app->targets));
    app->target_count = fs_targets_load(app->storage, app->targets, FS_MAX_TARGETS);
    if(app->engagement.target[0]) fs_target_profile_load(app->storage, app->engagement.target, &app->target_profile);
}

static void fs_settings_mode_changed(VariableItem* item) {
    FieldSecApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.mode = index ? FsModeField : FsModeLearn;
    variable_item_set_current_value_text(item, index ? "Field" : "Learn");
    fs_save_state(app);
}

static void fs_settings_skill_changed(VariableItem* item) {
    FieldSecApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    if(index >= FsSkillCount) index = FsSkillBeginner;
    app->settings.skill_level = (FsSkillLevel)index;
    app->settings.setup_complete = true;
    variable_item_set_current_value_text(item, fs_skill_name(app->settings.skill_level));
    fs_save_state(app);
}

static void fs_domain_set(FieldSecApp* app, VariableItem* item, FsSkillDomain domain) {
    uint8_t index=variable_item_get_current_value_index(item);
    if(index>=FsSkillCount) index=FsSkillBeginner;
    app->settings.domain_skill[domain]=(FsSkillLevel)index;
    variable_item_set_current_value_text(item,fs_skill_name(app->settings.domain_skill[domain]));
    fs_save_state(app);
}
static void fs_settings_hw_changed(VariableItem* item){ FieldSecApp* app=variable_item_get_context(item); fs_domain_set(app,item,FsDomainHardware); }
static void fs_settings_wireless_changed(VariableItem* item){ FieldSecApp* app=variable_item_get_context(item); fs_domain_set(app,item,FsDomainWireless); }
static void fs_settings_credential_changed(VariableItem* item){ FieldSecApp* app=variable_item_get_context(item); fs_domain_set(app,item,FsDomainCredential); }
static void fs_settings_evidence_changed(VariableItem* item){ FieldSecApp* app=variable_item_get_context(item); fs_domain_set(app,item,FsDomainEvidence); }
static void fs_settings_threat_changed(VariableItem* item){ FieldSecApp* app=variable_item_get_context(item); fs_domain_set(app,item,FsDomainThreatModeling); }

static void fs_settings_adaptive_changed(VariableItem* item) {
    FieldSecApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.adaptive_guidance = index != 0;
    variable_item_set_current_value_text(item, app->settings.adaptive_guidance ? "On" : "Off");
    fs_save_state(app);
}

static void fs_settings_severity_changed(VariableItem* item) {
    FieldSecApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    if(index >= FsSeverityCount) index = FsSeverityInfo;
    app->settings.severity = (FsSeverity)index;
    variable_item_set_current_value_text(item, fs_severity_name(app->settings.severity));
    fs_save_state(app);
}

static void fs_settings_baud_changed(VariableItem* item) {
    FieldSecApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    if(index > 4) index = 4;
    app->settings.uart_baud_index = index;
    variable_item_set_current_value_text(item, fs_uart_baud_label(index));
    fs_save_state(app);
}

static void fs_settings_duration_changed(VariableItem* item) {
    FieldSecApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    if(index > 2) index = 1;
    app->settings.uart_duration_index = index;
    variable_item_set_current_value_text(item, fs_uart_duration_label(index));
    fs_save_state(app);
}

static void fs_show_settings(FieldSecApp* app) {
    variable_item_list_reset(app->settings_list);
    VariableItem* skill = variable_item_list_add(app->settings_list, "Skill level", FsSkillCount, fs_settings_skill_changed, app);
    variable_item_set_current_value_index(skill, (uint8_t)app->settings.skill_level);
    variable_item_set_current_value_text(skill, fs_skill_name(app->settings.skill_level));

    VariableItem* hw = variable_item_list_add(app->settings_list, "Hardware skill", FsSkillCount, fs_settings_hw_changed, app);
    variable_item_set_current_value_index(hw,(uint8_t)app->settings.domain_skill[FsDomainHardware]); variable_item_set_current_value_text(hw,fs_skill_name(app->settings.domain_skill[FsDomainHardware]));
    VariableItem* wl = variable_item_list_add(app->settings_list, "Wireless skill", FsSkillCount, fs_settings_wireless_changed, app);
    variable_item_set_current_value_index(wl,(uint8_t)app->settings.domain_skill[FsDomainWireless]); variable_item_set_current_value_text(wl,fs_skill_name(app->settings.domain_skill[FsDomainWireless]));
    VariableItem* cr = variable_item_list_add(app->settings_list, "NFC/RFID skill", FsSkillCount, fs_settings_credential_changed, app);
    variable_item_set_current_value_index(cr,(uint8_t)app->settings.domain_skill[FsDomainCredential]); variable_item_set_current_value_text(cr,fs_skill_name(app->settings.domain_skill[FsDomainCredential]));
    VariableItem* ev = variable_item_list_add(app->settings_list, "Evidence skill", FsSkillCount, fs_settings_evidence_changed, app);
    variable_item_set_current_value_index(ev,(uint8_t)app->settings.domain_skill[FsDomainEvidence]); variable_item_set_current_value_text(ev,fs_skill_name(app->settings.domain_skill[FsDomainEvidence]));
    VariableItem* tm = variable_item_list_add(app->settings_list, "Threat-model skill", FsSkillCount, fs_settings_threat_changed, app);
    variable_item_set_current_value_index(tm,(uint8_t)app->settings.domain_skill[FsDomainThreatModeling]); variable_item_set_current_value_text(tm,fs_skill_name(app->settings.domain_skill[FsDomainThreatModeling]));

    VariableItem* adaptive = variable_item_list_add(app->settings_list, "Adaptive guidance", 2, fs_settings_adaptive_changed, app);
    variable_item_set_current_value_index(adaptive, app->settings.adaptive_guidance ? 1 : 0);
    variable_item_set_current_value_text(adaptive, app->settings.adaptive_guidance ? "On" : "Off");

    VariableItem* mode = variable_item_list_add(app->settings_list, "Operator mode", 2, fs_settings_mode_changed, app);
    variable_item_set_current_value_index(mode, app->settings.mode == FsModeField ? 1 : 0);
    variable_item_set_current_value_text(mode, app->settings.mode == FsModeField ? "Field" : "Learn");

    VariableItem* severity = variable_item_list_add(app->settings_list, "Default severity", FsSeverityCount, fs_settings_severity_changed, app);
    variable_item_set_current_value_index(severity, (uint8_t)app->settings.severity);
    variable_item_set_current_value_text(severity, fs_severity_name(app->settings.severity));

    VariableItem* baud = variable_item_list_add(app->settings_list, "UART baud", 5, fs_settings_baud_changed, app);
    variable_item_set_current_value_index(baud, app->settings.uart_baud_index);
    variable_item_set_current_value_text(baud, fs_uart_baud_label(app->settings.uart_baud_index));

    VariableItem* duration = variable_item_list_add(app->settings_list, "UART window", 3, fs_settings_duration_changed, app);
    variable_item_set_current_value_index(duration, app->settings.uart_duration_index);
    variable_item_set_current_value_text(duration, fs_uart_duration_label(app->settings.uart_duration_index));
    fs_switch_view(app, FsViewSettings);
}

static void fs_text_done(void* context) {
    FieldSecApp* app = context;
    switch(app->input_purpose) {
    case FsInputEngagement:
        snprintf(app->engagement.engagement, sizeof(app->engagement.engagement), "%s", app->input_buffer);
        fs_save_state(app);
        fs_build_menu(app, FsMenuEngagement);
        break;
    case FsInputTarget:
        snprintf(app->engagement.target, sizeof(app->engagement.target), "%s", app->input_buffer);
        fs_save_state(app);
        fs_build_menu(app, FsMenuEngagement);
        break;
    case FsInputScope:
        snprintf(app->engagement.scope, sizeof(app->engagement.scope), "%s", app->input_buffer);
        fs_save_state(app);
        fs_build_menu(app, FsMenuEngagement);
        break;
    case FsInputNewTarget: {
        bool ok = fs_target_add(app->storage, app->input_buffer);
        if(ok) {
            snprintf(app->engagement.target, sizeof(app->engagement.target), "%s", app->input_buffer);
            fs_save_state(app);
            fs_refresh_targets(app);
            fs_target_profile_load(app->storage, app->engagement.target, &app->target_profile);
        }
        fs_notice(app, ok ? "Target saved" : "Save failed", ok ? "Target added to targets.txt and selected as the active target." : "Could not save target. Verify that the microSD card is writable.");
        break;
    }
    case FsInputFindingTitle:
        snprintf(app->finding_title, sizeof(app->finding_title), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputFindingCondition, "Observed condition", "");
        break;
    case FsInputFindingCondition:
        snprintf(app->finding_condition, sizeof(app->finding_condition), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputFindingEvidence, "Evidence reference", "");
        break;
    case FsInputFindingEvidence:
        snprintf(app->finding_evidence, sizeof(app->finding_evidence), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputFindingImpact, "Validated impact", "");
        break;
    case FsInputFindingImpact:
        snprintf(app->finding_impact, sizeof(app->finding_impact), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputFindingRemediation, "Recommended control", "");
        break;
    case FsInputFindingRemediation:
        snprintf(app->finding_remediation, sizeof(app->finding_remediation), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputFindingRetest, "Retest success criteria", "");
        break;
    case FsInputFindingRetest: {
        snprintf(app->finding_retest, sizeof(app->finding_retest), "%s", app->input_buffer);
        char finding_id[FS_ID_MAX] = {0};
        bool ok = fs_finding_append_structured_id(
            app->storage, &app->engagement, app->finding_title, app->settings.severity,
            app->finding_condition, app->finding_evidence, app->finding_impact,
            app->finding_remediation, app->finding_retest, finding_id, sizeof(finding_id));
        app->input_purpose = FsInputNone;
        char body[320];
        snprintf(body, sizeof(body), ok ? "%s saved. Cite this ID during remediation retest. Supporting evidence should reference OBS IDs." : "Could not write the finding. Verify that the microSD card is present and writable.", ok ? finding_id : "Finding");
        fs_notice(app, ok ? "Finding saved" : "Save failed", body);
        break;
    }
    case FsInputRetestFindingId:
        snprintf(app->retest_finding_id, sizeof(app->retest_finding_id), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputRetestResult, "Retest result (Pass/Fail/Partial)", "");
        break;
    case FsInputRetestResult:
        snprintf(app->retest_result, sizeof(app->retest_result), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputRetestNote, "Retest observation", "");
        break;
    case FsInputRetestNote: {
        snprintf(app->retest_note, sizeof(app->retest_note), "%s", app->input_buffer);
        char retest_id[FS_ID_MAX] = {0};
        bool ok = fs_retest_append(app->storage, &app->engagement, app->retest_finding_id, app->retest_result, app->retest_note, retest_id, sizeof(retest_id));
        app->input_purpose = FsInputNone;
        char body[320];
        snprintf(body, sizeof(body), ok ? "%s saved for %s. Retest records preserve the observed result separately from the original finding." : "Could not save retest record.", ok ? retest_id : "Retest", app->retest_finding_id);
        fs_notice(app, ok ? "Retest saved" : "Save failed", body);
        break;
    }
    case FsInputHypStatement:
        snprintf(app->hyp_statement, sizeof(app->hyp_statement), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputHypTestPlan, "Least-invasive test plan", "");
        break;
    case FsInputHypTestPlan:
        snprintf(app->hyp_test_plan, sizeof(app->hyp_test_plan), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputHypExpected, "Expected result before test", "");
        break;
    case FsInputHypExpected:
        snprintf(app->hyp_expected, sizeof(app->hyp_expected), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputHypObserved, "Observed result (or pending)", "");
        break;
    case FsInputHypObserved:
        snprintf(app->hyp_observed, sizeof(app->hyp_observed), "%s", app->input_buffer);
        fs_start_text_input(app, FsInputHypStatus, "Status: UNTESTED/TESTING/SUPPORTED/NOT SUPPORTED/INCONCLUSIVE", app->hyp_observed[0] ? "INCONCLUSIVE" : "UNTESTED");
        break;
    case FsInputHypStatus: {
        snprintf(app->hyp_status, sizeof(app->hyp_status), "%s", app->input_buffer);
        char id[FS_ID_MAX]={0};
        bool ok=fs_hypothesis_append(app->storage,&app->engagement,app->hyp_statement,app->hyp_test_plan,app->hyp_expected,app->hyp_observed,app->hyp_status,id,sizeof(id));
        char body[360]; snprintf(body,sizeof(body), ok ? "%s saved. Keep prediction, observation, and interpretation separate; revise status only as evidence justifies." : "Could not save hypothesis.", ok?id:"Hypothesis");
        fs_notice(app,ok?"Hypothesis saved":"Save failed",body); break;
    }
    case FsInputJournal: {
        char id[FS_ID_MAX]={0}; bool ok=fs_journal_append(app->storage,&app->engagement,app->input_buffer,id,sizeof(id));
        char body[300]; snprintf(body,sizeof(body),ok?"%s saved to engagement journal.":"Could not save journal entry.",ok?id:"Entry"); fs_notice(app,ok?"Journal saved":"Save failed",body); break;
    }
    case FsInputAnnotationObs:
        snprintf(app->annotation_obs,sizeof(app->annotation_obs),"%s",app->input_buffer);
        fs_start_text_input(app,FsInputAnnotationText,"Evidence annotation",""); break;
    case FsInputAnnotationText: {
        char id[FS_ID_MAX]={0}; bool ok=fs_annotation_append(app->storage,&app->engagement,app->annotation_obs,app->input_buffer,id,sizeof(id));
        char body[360]; snprintf(body,sizeof(body),ok?"%s linked to %s. An annotation adds analyst context; it does not modify the underlying observation.":"Could not save annotation.",ok?id:"Annotation",app->annotation_obs); fs_notice(app,ok?"Annotation saved":"Save failed",body); break;
    }
    case FsInputInterfaceMap: {
        bool ok=fs_interface_map_append(app->storage,&app->engagement,app->input_buffer); fs_notice(app,ok?"Interface mapped":"Save failed",ok?"Interface inventory item saved. Inventory alone is not a vulnerability.":"Could not save interface map item."); break;
    }
    case FsInputServiceMap: {
        bool ok=fs_service_map_append(app->storage,&app->engagement,app->input_buffer); fs_notice(app,ok?"Service mapped":"Save failed",ok?"Service inventory item saved. Validate trust boundaries and controls before creating a finding.":"Could not save service map item."); break;
    }
    case FsInputTargetType:
        snprintf(app->target_profile.type, sizeof(app->target_profile.type), "%s", app->input_buffer);
        fs_target_profile_save(app->storage, &app->target_profile); fs_build_menu(app, FsMenuTargetProfile); break;
    case FsInputTargetIp:
        snprintf(app->target_profile.ip, sizeof(app->target_profile.ip), "%s", app->input_buffer);
        fs_target_profile_save(app->storage, &app->target_profile); fs_build_menu(app, FsMenuTargetProfile); break;
    case FsInputTargetMac:
        snprintf(app->target_profile.mac, sizeof(app->target_profile.mac), "%s", app->input_buffer);
        fs_target_profile_save(app->storage, &app->target_profile); fs_build_menu(app, FsMenuTargetProfile); break;
    case FsInputTargetFirmware:
        snprintf(app->target_profile.firmware, sizeof(app->target_profile.firmware), "%s", app->input_buffer);
        fs_target_profile_save(app->storage, &app->target_profile); fs_build_menu(app, FsMenuTargetProfile); break;
    case FsInputTargetInterfaces:
        snprintf(app->target_profile.interfaces, sizeof(app->target_profile.interfaces), "%s", app->input_buffer);
        fs_target_profile_save(app->storage, &app->target_profile); fs_build_menu(app, FsMenuTargetProfile); break;
    case FsInputTargetNotes:
        snprintf(app->target_profile.notes, sizeof(app->target_profile.notes), "%s", app->input_buffer);
        fs_target_profile_save(app->storage, &app->target_profile); fs_build_menu(app, FsMenuTargetProfile); break;
    default:
        fs_build_menu(app, FsMenuRoot);
        break;
    }
}

static void fs_start_text_input(FieldSecApp* app, FsInputPurpose purpose, const char* header, const char* initial) {
    app->input_purpose = purpose;
    text_input_reset(app->text_input);
    snprintf(app->input_buffer, sizeof(app->input_buffer), "%s", initial ? initial : "");
    text_input_set_header_text(app->text_input, header);
    text_input_set_result_callback(app->text_input, fs_text_done, app, app->input_buffer, sizeof(app->input_buffer), false);
    fs_switch_view(app, FsViewTextInput);
}

static void fs_detail_button_cb(GuiButtonType result, InputType type, void* context) {
    if(type != InputTypeShort) return;
    FieldSecApp* app = context;
    if(result == GuiButtonTypeLeft) {
        fs_build_menu(app, (FsMenuId)app->current_menu);
        return;
    }

    if(app->current_menu == FsMenuTraining) {
        size_t count = 0;
        const FsLesson* lessons = fs_lessons_get(&count);
        if(app->selected_lesson >= count) return;
        if(result == GuiButtonTypeCenter) {
            uint8_t* st = &app->lesson_status[app->selected_lesson];
            if(*st == FsLessonNotStarted) {
                *st = FsLessonInProgress;
            } else if(*st == FsLessonInProgress) {
                if(lessons[app->selected_lesson].paired_instrument >= 0 && app->lesson_practice[app->selected_lesson] == 0) {
                    fs_notice(app, "Practice checkpoint", "Run the paired real-world instrument at least once before completing this lesson. The goal is to interpret actual evidence, not only read the concept.");
                    return;
                }
                *st = FsLessonComplete;
            } else {
                *st = FsLessonNotStarted;
                app->lesson_practice[app->selected_lesson] = 0;
            }
            fs_training_save(app->storage, app->lesson_status, app->lesson_practice, count);
            fs_show_lesson(app);
            return;
        }
        if(result == GuiButtonTypeRight && lessons[app->selected_lesson].paired_instrument >= 0) {
            if(app->lesson_practice[app->selected_lesson] < 255) app->lesson_practice[app->selected_lesson]++;
            fs_training_save(app->storage, app->lesson_status, app->lesson_practice, count);
            fs_run_instrument(app, (uint32_t)lessons[app->selected_lesson].paired_instrument);
            return;
        }
    }

    if(result == GuiButtonTypeRight && app->current_menu == FsMenuModules) {
        size_t count = 0;
        const FsModule* catalog = fs_catalog_get(&count);
        if(app->selected >= count) return;
        char obs_id[FS_ID_MAX] = {0};
        bool ok = fs_evidence_append_event_id(app->storage, &app->engagement, catalog[app->selected].category, catalog[app->selected].name, catalog[app->selected].risk, "Module reviewed / observation recorded", obs_id, sizeof(obs_id));
        char msg[220];
        snprintf(msg, sizeof(msg), ok ? "%s appended to evidence.csv. Use this observation ID when a later finding depends on it." : "Could not write evidence. Verify that the microSD card is present and writable.", ok ? obs_id : "Observation");
        fs_notice(app, ok ? "Evidence saved" : "Save failed", msg);
    }
}

static size_t fs_training_completed(FieldSecApp* app, size_t* total) {
    size_t count = 0;
    fs_lessons_get(&count);
    size_t done = 0;
    for(size_t i = 0; i < count && i < FS_MAX_LESSONS; i++) {
        if(app->lesson_status[i] == FsLessonComplete) done++;
    }
    if(total) *total = count;
    return done;
}

static void fs_show_dashboard(FieldSecApp* app) {
    char body[640];
    size_t training_total = 0;
    size_t training_done = fs_training_completed(app, &training_total);
    size_t ev_count = 0, finding_count = 0, retest_count = 0;
    if(app->engagement.target[0]) fs_target_stats(app->storage, app->engagement.target, &ev_count, &finding_count, &retest_count);
    uint8_t capstone = fs_capstone_score(app->storage, &app->engagement, app->lesson_status, training_total, NULL, 0);
    snprintf(body, sizeof(body),
        "Skill: %s%s\nMode: %s\nEngagement: %s\nTarget: %s\nAuthorized: %s\nSaved targets: %u\nTarget profile: %s\nUART: %s / %s\nTraining: %u/%u complete\nRecords: %u OBS / %u FND / %u RT\nCapstone: %u/7\n\nWorkflow:\nScope > Recon > Hypothesis > Validate > Evidence > Finding > Mitigate > Retest",
        fs_skill_name(app->settings.skill_level),
        app->settings.adaptive_guidance ? " / ADAPTIVE" : "",
        app->settings.mode == FsModeField ? "FIELD" : "LEARN",
        app->engagement.engagement[0] ? app->engagement.engagement : "(not set)",
        app->engagement.target[0] ? app->engagement.target : "(not set)",
        app->engagement.authorized ? "YES" : "NO",
        (unsigned)app->target_count,
        app->target_profile.type[0] ? app->target_profile.type : "not classified",
        fs_uart_baud_label(app->settings.uart_baud_index),
        fs_uart_duration_label(app->settings.uart_duration_index),
        (unsigned)training_done,
        (unsigned)training_total,
        (unsigned)ev_count,
        (unsigned)finding_count,
        (unsigned)retest_count,
        (unsigned)capstone);
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 3, AlignCenter, AlignTop, FontPrimary, "FieldSec Dashboard");
    widget_add_text_scroll_element(app->widget, 3, 15, 122, 36, body);
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Back", fs_detail_button_cb, app);
    fs_switch_view(app, FsViewDetail);
}

static void fs_show_engagement(FieldSecApp* app) {
    char body[640];
    snprintf(body, sizeof(body),
        "Engagement: %s\nTarget: %s\nScope: %s\nAuthorization: %s\n\nActive testing must remain inside written scope. FieldSec stores operator-entered metadata only.",
        app->engagement.engagement[0] ? app->engagement.engagement : "(not set)",
        app->engagement.target[0] ? app->engagement.target : "(not set)",
        app->engagement.scope[0] ? app->engagement.scope : "(not set)",
        app->engagement.authorized ? "ACKNOWLEDGED" : "NOT ACKNOWLEDGED");
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 3, AlignCenter, AlignTop, FontPrimary, "Engagement Status");
    widget_add_text_scroll_element(app->widget, 3, 15, 122, 36, body);
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Back", fs_detail_button_cb, app);
    fs_switch_view(app, FsViewDetail);
}

static void fs_show_target_profile(FieldSecApp* app) {
    if(!app->engagement.target[0]) {
        fs_notice(app, "Select target", "Choose or create an active target before editing target intelligence.");
        return;
    }
    fs_target_profile_load(app->storage, app->engagement.target, &app->target_profile);
    char body[1000];
    size_t ev_count = 0, finding_count = 0, retest_count = 0;
    fs_target_stats(app->storage, app->engagement.target, &ev_count, &finding_count, &retest_count);
    snprintf(body, sizeof(body),
        "Name: %s\nType: %s\nIP: %s\nMAC/BSSID: %s\nFirmware: %s\nInterfaces: %s\nNotes: %s\nRecords: %u observations / %u findings / %u retests\n\nInstruction: inventory what is observed. Do not promote an identifier, version, or exposed interface into a vulnerability without validation.",
        app->target_profile.name,
        app->target_profile.type[0] ? app->target_profile.type : "(not set)",
        app->target_profile.ip[0] ? app->target_profile.ip : "(not set)",
        app->target_profile.mac[0] ? app->target_profile.mac : "(not set)",
        app->target_profile.firmware[0] ? app->target_profile.firmware : "(not set)",
        app->target_profile.interfaces[0] ? app->target_profile.interfaces : "(not set)",
        app->target_profile.notes[0] ? app->target_profile.notes : "(not set)",
        (unsigned)ev_count, (unsigned)finding_count, (unsigned)retest_count);
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 2, AlignCenter, AlignTop, FontPrimary, "Target Intelligence");
    widget_add_text_scroll_element(app->widget, 2, 12, 124, 40, body);
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Back", fs_detail_button_cb, app);
    fs_switch_view(app, FsViewDetail);
}

static void fs_show_workspace(FieldSecApp* app) {
    char body[1900]; fs_target_workspace_render(app->storage, app->engagement.target, body, sizeof(body));
    widget_reset(app->widget); widget_add_string_element(app->widget,64,2,AlignCenter,AlignTop,FontPrimary,"Target Workspace");
    widget_add_text_scroll_element(app->widget,2,12,124,40,body); widget_add_button_element(app->widget,GuiButtonTypeLeft,"Back",fs_detail_button_cb,app); fs_switch_view(app,FsViewDetail);
}

static void fs_show_after_action(FieldSecApp* app) {
    char body[1800]; size_t total=0; fs_lessons_get(&total); fs_after_action_render(app->storage,&app->engagement,app->lesson_status,total,body,sizeof(body));
    widget_reset(app->widget); widget_add_string_element(app->widget,64,2,AlignCenter,AlignTop,FontPrimary,"After-Action Review");
    widget_add_text_scroll_element(app->widget,2,12,124,40,body); widget_add_button_element(app->widget,GuiButtonTypeLeft,"Back",fs_detail_button_cb,app); fs_switch_view(app,FsViewDetail);
}

static void fs_show_timeline(FieldSecApp* app) {
    char body[1800];
    fs_timeline_render(app->storage, app->engagement.target, body, sizeof(body));
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 2, AlignCenter, AlignTop, FontPrimary, "Evidence Timeline");
    widget_add_text_scroll_element(app->widget, 2, 12, 124, 40, body);
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Back", fs_detail_button_cb, app);
    fs_switch_view(app, FsViewDetail);
}

static void fs_show_capstone(FieldSecApp* app) {
    char body[1000];
    size_t total = 0;
    fs_lessons_get(&total);
    fs_capstone_score(app->storage, &app->engagement, app->lesson_status, total, body, sizeof(body));
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 2, AlignCenter, AlignTop, FontPrimary, "Capstone Engagement");
    widget_add_text_scroll_element(app->widget, 2, 12, 124, 40, body);
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Back", fs_detail_button_cb, app);
    fs_switch_view(app, FsViewDetail);
}

static void fs_show_hypotheses(FieldSecApp* app) { char body[1900]; fs_hypothesis_render(app->storage,app->engagement.target,body,sizeof(body)); fs_notice(app,"Hypothesis Workspace",body); }
static void fs_show_journal(FieldSecApp* app) { char body[1800]; fs_journal_render(app->storage,app->engagement.target,body,sizeof(body)); fs_notice(app,"Engagement Journal",body); }
static void fs_show_annotations(FieldSecApp* app) { char body[1800]; fs_annotation_render(app->storage,app->engagement.target,body,sizeof(body)); fs_notice(app,"Evidence Annotations",body); }
static void fs_show_target_map(FieldSecApp* app) { char body[1900]; fs_target_map_render(app->storage,app->engagement.target,body,sizeof(body)); fs_notice(app,"Interface / Service Map",body); }
static void fs_show_instructor_review(FieldSecApp* app) { char body[2100]; fs_instructor_review_render(app->storage,&app->engagement,body,sizeof(body)); fs_notice(app,"Instructor Review",body); }
static void fs_show_adaptive_path(FieldSecApp* app) { char body[2100]; size_t count=0; fs_lessons_get(&count); fs_adaptive_render(&app->settings,&app->target_profile,app->lesson_status,app->lesson_practice,count,body,sizeof(body)); fs_notice(app,"Adaptive Learning Path",body); }
static void fs_show_adaptive_curriculum(FieldSecApp* app) { char body[2100]; fs_curriculum_render(&app->settings,&app->target_profile,body,sizeof(body)); fs_notice(app,"Adaptive Curriculum",body); }
static void fs_show_target_graph(FieldSecApp* app) { if(!app->engagement.target[0]) { fs_notice(app,"Select target","Choose an active target first."); return; } fs_target_profile_load(app->storage,app->engagement.target,&app->target_profile); char body[2100]; fs_target_graph_render(app->storage,&app->engagement,&app->target_profile,body,sizeof(body)); fs_notice(app,"Target Relationship Graph",body); }
static void fs_show_competency(FieldSecApp* app) { char body[2100]; size_t count=0; fs_lessons_get(&count); fs_competency_render(app->storage,&app->engagement,&app->settings,app->lesson_status,app->lesson_practice,count,body,sizeof(body)); fs_notice(app,"Competency Engine",body); }
static void fs_show_assessment_plan(FieldSecApp* app) { if(!app->engagement.target[0]) { fs_notice(app,"Select target","Choose an active target before generating an assessment plan."); return; } fs_target_profile_load(app->storage,app->engagement.target,&app->target_profile); char body[2200]; fs_assessment_plan_render(app->storage,&app->engagement,&app->settings,&app->target_profile,body,sizeof(body)); fs_notice(app,"Assessment Planner",body); }

static void fs_show_module(FieldSecApp* app) {
    size_t count = 0;
    const FsModule* catalog = fs_catalog_get(&count);
    if(app->selected >= count) return;
    const FsModule* m = &catalog[app->selected];
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 2, AlignCenter, AlignTop, FontPrimary, m->name);
    char body[800];
    if(app->settings.mode == FsModeLearn) {
        if(app->settings.adaptive_guidance && app->settings.skill_level >= FsSkillAdvanced)
            snprintf(body,sizeof(body),"[%s | %s | %s]\n%s\n\nPRACTICE: %s\n\nANALYZE: %s\n\nNEXT: %s\n\nFull beginner/intermediate guidance remains available by disabling Adaptive Guidance.",m->category,fs_risk_name(m->risk),fs_skill_name(app->settings.skill_level),m->purpose,m->setup,m->analysis,m->next_step);
        else
            snprintf(body, sizeof(body), "[%s | %s | %s]\n\nWHAT/WHY:\n%s\n\nSETUP:\n%s\n\nANALYZE:\n%s\n\nMITIGATE:\n%s\n\nNEXT:\n%s", m->category, fs_risk_name(m->risk), fs_skill_name(app->settings.skill_level), m->purpose, m->setup, m->analysis, m->mitigation, m->next_step);
    } else {
        snprintf(body, sizeof(body), "[%s | %s | %s]\n%s\n\nSETUP: %s\n\nANALYZE: %s\n\nNEXT: %s", m->category, fs_risk_name(m->risk), fs_skill_name(app->settings.skill_level), m->purpose, m->setup, m->analysis, m->next_step);
    }
    widget_add_text_scroll_element(app->widget, 2, 12, 124, 40, body);
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Back", fs_detail_button_cb, app);
    widget_add_button_element(app->widget, GuiButtonTypeRight, "Log", fs_detail_button_cb, app);
    fs_switch_view(app, FsViewDetail);
}

static void fs_show_lesson(FieldSecApp* app) {
    size_t count = 0;
    const FsLesson* lessons = fs_lessons_get(&count);
    if(app->selected_lesson >= count) return;
    const FsLesson* l = &lessons[app->selected_lesson];
    uint8_t status = app->lesson_status[app->selected_lesson];
    char body[1400];
    if(app->settings.adaptive_guidance && app->settings.skill_level == FsSkillExpert)
        snprintf(body,sizeof(body),"[%s | %s | Expert/Field | practice %u]\n\nOBJECTIVE: %s\n\nPRACTICE: %s\n\nINTERPRET: %s\n\nCHECKPOINT: %s\n\nFull instructional content remains available by disabling Adaptive Guidance.",l->domain,fs_lesson_status_name(status),(unsigned)app->lesson_practice[app->selected_lesson],l->objective,l->practice,l->interpret,l->checkpoint);
    else if(app->settings.adaptive_guidance && app->settings.skill_level == FsSkillAdvanced)
        snprintf(body,sizeof(body),"[%s | %s | Advanced | practice %u]\n\nOBJECTIVE:\n%s\n\nSETUP/PRACTICE:\n%s\n%s\n\nINTERPRET:\n%s\n\nDEFEND:\n%s\n\nCHECKPOINT:\n%s",l->domain,fs_lesson_status_name(status),(unsigned)app->lesson_practice[app->selected_lesson],l->objective,l->setup,l->practice,l->interpret,l->defend,l->checkpoint);
    else
        snprintf(body, sizeof(body),
            "[%s | %s | %s | practice %u]\n\nOBJECTIVE:\n%s\n\nCONCEPT:\n%s\n\nSETUP:\n%s\n\nPRACTICE:\n%s\n\nINTERPRET:\n%s\n\nDEFEND:\n%s\n\nCHECKPOINT:\n%s",
            l->domain, fs_lesson_status_name(status), fs_skill_name(app->settings.skill_level), (unsigned)app->lesson_practice[app->selected_lesson], l->objective, l->concept, l->setup, l->practice, l->interpret, l->defend, l->checkpoint);
    widget_reset(app->widget);
    widget_add_string_element(app->widget, 64, 2, AlignCenter, AlignTop, FontPrimary, l->title);
    widget_add_text_scroll_element(app->widget, 2, 12, 124, 39, body);
    widget_add_button_element(app->widget, GuiButtonTypeLeft, "Back", fs_detail_button_cb, app);
    widget_add_button_element(app->widget, GuiButtonTypeCenter, status == FsLessonNotStarted ? "Start" : (status == FsLessonInProgress ? "Done" : "Reset"), fs_detail_button_cb, app);
    if(l->paired_instrument >= 0) widget_add_button_element(app->widget, GuiButtonTypeRight, "Practice", fs_detail_button_cb, app);
    fs_switch_view(app, FsViewDetail);
}

static void fs_run_uart(FieldSecApp* app) {
    if(!app->engagement.authorized) {
        fs_notice(app, "Scope first", "Acknowledge engagement authorization before using live instruments.");
        return;
    }
    char preview[FS_UART_CAPTURE_MAX], hex_preview[FS_UART_HEX_MAX];
    size_t captured = 0;
    uint8_t printable = 0;
    bool saved = fs_uart_capture(app->storage, fs_uart_baud_from_index(app->settings.uart_baud_index),
        fs_uart_duration_ms_from_index(app->settings.uart_duration_index), preview, sizeof(preview),
        hex_preview, sizeof(hex_preview), &captured, &printable);
    char body[2100];
    snprintf(body, sizeof(body),
        "UART Workbench | receive-only\nBaud: %s\nWindow: %s\nBytes: %u\nPrintable ratio: %u%%\nRaw: uart_capture.bin\nHistory: uart_history.csv\n\nTEXT VIEW\n%s\n\nHEX VIEW\n%s\n\nGUIDED ANALYSIS\n1. Is framing plausible at this baud?\n2. Which strings are direct evidence?\n3. Does any output expose identity/configuration?\n4. What additional test would validate impact?",
        fs_uart_baud_label(app->settings.uart_baud_index), fs_uart_duration_label(app->settings.uart_duration_index),
        (unsigned)captured, (unsigned)printable, preview, hex_preview);
    if(saved) {
        char note[220];
        char obs_id[FS_ID_MAX] = {0};
        snprintf(note, sizeof(note), "UART workbench captured %u byte(s) at %s baud over %s; printable ratio %u%%. Raw evidence retained.",
            (unsigned)captured, fs_uart_baud_label(app->settings.uart_baud_index), fs_uart_duration_label(app->settings.uart_duration_index), (unsigned)printable);
        if(fs_evidence_append_event_id(app->storage, &app->engagement, "Hardware/IoT", "UART Workbench", FsRiskLocal, note, obs_id, sizeof(obs_id))) {
            size_t used = strlen(body);
            snprintf(body + used, sizeof(body) - used, "\n\nObservation ID: %s", obs_id);
        }
    }
    fs_notice(app, "UART Workbench", body);
}

static void fs_run_i2c(FieldSecApp* app) {
    if(!app->engagement.authorized) {
        fs_notice(app, "Scope first", "Acknowledge engagement authorization before using live instruments.");
        return;
    }
    char scan[720];
    size_t added = 0, removed = 0;
    size_t found = fs_i2c_scan_compare(app->storage, scan, sizeof(scan), &added, &removed);
    char body[1100];
    snprintf(body, sizeof(body),
        "I2C Workbench\nResponding addresses: %u\nNew since prior scan: %u\nMissing since prior scan: %u\n\n%s\n\nGUIDED ANALYSIS\nAn address is bus-presence evidence, not device identity. A change may reflect power state, wiring, mux state, firmware behavior, or a real topology change. Validate before assigning security meaning.",
        (unsigned)found, (unsigned)added, (unsigned)removed, scan);
    char note[220];
    char obs_id[FS_ID_MAX] = {0};
    snprintf(note, sizeof(note), "I2C workbench observed %u address(es); %u new and %u missing relative to prior scan.", (unsigned)found, (unsigned)added, (unsigned)removed);
    if(fs_evidence_append_event_id(app->storage, &app->engagement, "Hardware/IoT", "I2C Workbench", FsRiskLocal, note, obs_id, sizeof(obs_id))) {
        size_t used = strlen(body);
        snprintf(body + used, sizeof(body) - used, "\n\nObservation ID: %s", obs_id);
    }
    fs_notice(app, "I2C Workbench", body);
}

static void fs_show_wifi_board(FieldSecApp* app) {
    if(!app->engagement.authorized) {
        fs_notice(app, "Scope first", "Acknowledge engagement authorization before collecting live Wi-Fi telemetry.");
        return;
    }
    char scan[1000];
    size_t aps = 0, opens = 0, new_aps = 0, disappeared = 0;
    bool saved = fs_wifi_board_scan_compare(app->storage, scan, sizeof(scan), &aps, &opens, &new_aps, &disappeared);
    char body[1500];
    snprintf(body, sizeof(body),
        "Wi-Fi Workbench | passive survey\nAP records: %u\nAdvertised OPEN: %u\nNew BSSIDs: %u\nDisappeared BSSIDs: %u\nRaw: wifi_scan.txt\n\n%s\n\nGUIDED ANALYSIS\nA new or disappearing BSSID is a change observation, not automatically an intrusion. Consider mobility, power cycles, channel conditions, randomized/virtual interfaces and ownership before forming a finding.",
        (unsigned)aps, (unsigned)opens, (unsigned)new_aps, (unsigned)disappeared, scan);
    if(saved) {
        char note[240];
        char obs_id[FS_ID_MAX] = {0};
        snprintf(note, sizeof(note), "Passive Wi-Fi workbench observed %u AP record(s), %u OPEN auth, %u new BSSID(s), and %u disappeared BSSID(s) relative to prior survey.",
            (unsigned)aps, (unsigned)opens, (unsigned)new_aps, (unsigned)disappeared);
        if(fs_evidence_append_event_id(app->storage, &app->engagement, "Wireless", "Wi-Fi Workbench", FsRiskPassive, note, obs_id, sizeof(obs_id))) {
            size_t used = strlen(body);
            snprintf(body + used, sizeof(body) - used, "\n\nObservation ID: %s", obs_id);
        }
    }
    fs_notice(app, "Wi-Fi Workbench", body);
}

static void fs_show_uart_review(FieldSecApp* app) { char body[1900]; fs_uart_history_render(app->storage,body,sizeof(body)); fs_notice(app,"UART Session Review",body); }
static void fs_show_uart_strings(FieldSecApp* app) { char body[1800]; fs_uart_extract_strings(app->storage,body,sizeof(body)); fs_notice(app,"UART Extracted Strings",body); }
static void fs_show_i2c_history(FieldSecApp* app) { char body[1900]; fs_i2c_history_render(app->storage,body,sizeof(body)); fs_notice(app,"I2C Topology History",body); }
static void fs_show_wifi_history(FieldSecApp* app) { char body[1900]; fs_wifi_history_render(app->storage,body,sizeof(body)); fs_notice(app,"Wi-Fi Survey History",body); }

static void fs_show_gpio_guide(FieldSecApp* app) {
    if(!app->engagement.authorized) { fs_notice(app,"Scope first","Acknowledge engagement authorization before collecting live GPIO observations."); return; }
    char body[1800];
    if(fs_gpio_snapshot(app->storage,body,sizeof(body))) {
        char obs_id[FS_ID_MAX]={0};
        if(fs_evidence_append_event_id(app->storage,&app->engagement,"Hardware/IoT","GPIO Monitor",FsRiskPassive,"Passive one-second digital observation of external connector pins with sampled edge counts; no pin configuration was changed.",obs_id,sizeof(obs_id))) {
            size_t used=strlen(body); snprintf(body+used,sizeof(body)-used,"\n\nObservation ID: %s",obs_id);
        }
        fs_notice(app,"GPIO Monitor",body);
    } else fs_notice(app,"GPIO Monitor","GPIO observation failed before changing any GPIO configuration.");
}

static void fs_run_instrument(FieldSecApp* app, uint32_t instrument) {
    switch(instrument) {
    case FsInstrumentUart: fs_run_uart(app); break;
    case FsInstrumentUartReview: fs_show_uart_review(app); break;
    case FsInstrumentUartStrings: fs_show_uart_strings(app); break;
    case FsInstrumentI2c: fs_run_i2c(app); break;
    case FsInstrumentI2cHistory: fs_show_i2c_history(app); break;
    case FsInstrumentWifiBoard: fs_show_wifi_board(app); break;
    case FsInstrumentWifiHistory: fs_show_wifi_history(app); break;
    case FsInstrumentGpioGuide: fs_show_gpio_guide(app); break;
    default: break;
    }
}

static void fs_menu_cb(void* context, uint32_t index) {
    FieldSecApp* app = context;
    if(app->current_menu == FsMenuRoot) {
        switch(index) {
        case FsRootDashboard: fs_show_dashboard(app); break;
        case FsRootEngagement: fs_build_menu(app, FsMenuEngagement); break;
        case FsRootTargets: fs_build_menu(app, FsMenuTargets); break;
        case FsRootTargetIntel: fs_build_menu(app, FsMenuTargetProfile); break;
        case FsRootWorkspace: fs_show_workspace(app); break;
        case FsRootHypotheses:
            if(!app->engagement.target[0]) fs_notice(app,"Select target","Choose an active target before creating hypotheses.");
            else { fs_show_hypotheses(app); }
            break;
        case FsRootJournal: fs_show_journal(app); break;
        case FsRootAnnotations: fs_show_annotations(app); break;
        case FsRootTargetMap: fs_show_target_map(app); break;
        case FsRootInstructorReview: fs_show_instructor_review(app); break;
        case FsRootAdaptivePath: fs_show_adaptive_path(app); break;
        case FsRootAdaptiveCurriculum: fs_show_adaptive_curriculum(app); break;
        case FsRootTargetGraph: fs_show_target_graph(app); break;
        case FsRootCompetency: fs_show_competency(app); break;
        case FsRootAssessmentPlan: fs_show_assessment_plan(app); break;
        case FsRootNewHypothesis:
            if(!app->engagement.authorized || !app->engagement.target[0]) fs_notice(app,"Scope first","Select a target and acknowledge authorization before planning target-specific validation.");
            else fs_start_text_input(app,FsInputHypStatement,"Hypothesis statement","");
            break;
        case FsRootNewJournal: fs_start_text_input(app,FsInputJournal,"Journal observation / decision",""); break;
        case FsRootNewAnnotation: fs_start_text_input(app,FsInputAnnotationObs,"Observation ID (OBS-######)","OBS-"); break;
        case FsRootMapInterface: fs_start_text_input(app,FsInputInterfaceMap,"Observed interface / port",""); break;
        case FsRootMapService: fs_start_text_input(app,FsInputServiceMap,"Observed service / endpoint",""); break;
        case FsRootInstruments: fs_build_menu(app, FsMenuInstruments); break;
        case FsRootModules: fs_build_menu(app, FsMenuModules); break;
        case FsRootTraining: fs_build_menu(app, FsMenuTraining); break;
        case FsRootFinding:
            if(!app->engagement.authorized) fs_notice(app, "Scope first", "Acknowledge engagement authorization before creating assessment findings.");
            else fs_start_text_input(app, FsInputFindingTitle, "Finding title", "");
            break;
        case FsRootRetest:
            if(!app->engagement.authorized) fs_notice(app, "Scope first", "Acknowledge engagement authorization before recording retest results.");
            else fs_start_text_input(app, FsInputRetestFindingId, "Finding ID (FND-######)", "FND-");
            break;
        case FsRootTimeline: fs_show_timeline(app); break;
        case FsRootCapstone: fs_show_capstone(app); break;
        case FsRootAfterAction: fs_show_after_action(app); break;
        case FsRootExport: {
            bool ok = fs_export_summary(app->storage, &app->engagement, &app->settings);
            fs_notice(app, ok ? "Summary exported" : "Export failed", ok ? "summary.txt written to the FieldSec app-data directory." : "Verify that the microSD card is writable.");
            break;
        }
        case FsRootSettings: fs_show_settings(app); break;
        case FsRootAbout: fs_notice(app, "FieldSec " FS_VERSION, "First-release handheld cybersecurity learning and authorized field-assessment environment.\n\nSkill-aware instruction: overall + per-domain Beginner > Intermediate > Advanced > Expert/Field.\n\nOfficial Flipper firmware target.\nOfficial Wi-Fi Developer Board target.\n\nObserve > Predict > Test > Compare > Validate > Trace > Defend > Retest"); break;
        }
    } else if(app->current_menu == FsMenuEngagement) {
        switch(index) {
        case 0: fs_show_engagement(app); break;
        case 1: fs_start_text_input(app, FsInputEngagement, "Engagement / lab name", app->engagement.engagement); break;
        case 2: fs_start_text_input(app, FsInputTarget, "Primary target", app->engagement.target); break;
        case 3: fs_start_text_input(app, FsInputScope, "Scope note", app->engagement.scope); break;
        case 4: app->engagement.authorized = !app->engagement.authorized; fs_save_state(app); fs_build_menu(app, FsMenuEngagement); break;
        default: break;
        }
    } else if(app->current_menu == FsMenuTargets) {
        if(index == 0) fs_start_text_input(app, FsInputNewTarget, "New target name", "");
        else if(index - 1 < app->target_count) {
            snprintf(app->engagement.target, sizeof(app->engagement.target), "%s", app->targets[index - 1]);
            fs_save_state(app);
            fs_target_profile_load(app->storage, app->engagement.target, &app->target_profile);
            fs_notice(app, "Active target", app->engagement.target);
        }
    } else if(app->current_menu == FsMenuTargetProfile) {
        if(!app->engagement.target[0]) { fs_notice(app, "Select target", "Choose or create an active target first."); return; }
        fs_target_profile_load(app->storage, app->engagement.target, &app->target_profile);
        switch(index) {
        case 0: fs_show_target_profile(app); break;
        case 1: fs_start_text_input(app, FsInputTargetType, "Device / target type", app->target_profile.type); break;
        case 2: fs_start_text_input(app, FsInputTargetIp, "IP / network identity", app->target_profile.ip); break;
        case 3: fs_start_text_input(app, FsInputTargetMac, "MAC / BSSID", app->target_profile.mac); break;
        case 4: fs_start_text_input(app, FsInputTargetFirmware, "Firmware / version", app->target_profile.firmware); break;
        case 5: fs_start_text_input(app, FsInputTargetInterfaces, "Interfaces", app->target_profile.interfaces); break;
        case 6: fs_start_text_input(app, FsInputTargetNotes, "Target notes", app->target_profile.notes); break;
        default: break;
        }
    } else if(app->current_menu == FsMenuInstruments) {
        fs_run_instrument(app, index);
    } else if(app->current_menu == FsMenuTraining) {
        size_t count = 0;
        fs_lessons_get(&count);
        if(index < count) { app->selected_lesson = index; fs_show_lesson(app); }
    } else if(app->current_menu == FsMenuModules) {
        app->selected = index;
        fs_show_module(app);
    }
}

static void fs_build_menu(FieldSecApp* app, FsMenuId menu) {
    app->current_menu = menu;
    submenu_reset(app->submenu);
    if(menu == FsMenuRoot) {
        submenu_set_header(app->submenu, "FieldSec " FS_VERSION);
        submenu_add_item(app->submenu, "Dashboard", FsRootDashboard, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Engagement", FsRootEngagement, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Target Database", FsRootTargets, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Target Intelligence", FsRootTargetIntel, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Target Workspace", FsRootWorkspace, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Hypotheses: View", FsRootHypotheses, fs_menu_cb, app);
        submenu_add_item(app->submenu, "+ New Hypothesis", FsRootNewHypothesis, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Engagement Journal", FsRootJournal, fs_menu_cb, app);
        submenu_add_item(app->submenu, "+ Journal Entry", FsRootNewJournal, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Evidence Annotations", FsRootAnnotations, fs_menu_cb, app);
        submenu_add_item(app->submenu, "+ Annotate OBS", FsRootNewAnnotation, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Interface/Service Map", FsRootTargetMap, fs_menu_cb, app);
        submenu_add_item(app->submenu, "+ Map Interface", FsRootMapInterface, fs_menu_cb, app);
        submenu_add_item(app->submenu, "+ Map Service", FsRootMapService, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Instructor Review", FsRootInstructorReview, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Adaptive Learning Path", FsRootAdaptivePath, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Adaptive Curriculum", FsRootAdaptiveCurriculum, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Target Relationship Graph", FsRootTargetGraph, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Live Instruments", FsRootInstruments, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Assessment Modules", FsRootModules, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Training Academy", FsRootTraining, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Finding Builder", FsRootFinding, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Retest Logger", FsRootRetest, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Evidence Timeline", FsRootTimeline, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Capstone Status", FsRootCapstone, fs_menu_cb, app);
        submenu_add_item(app->submenu, "After-Action Review", FsRootAfterAction, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Export Summary", FsRootExport, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Settings", FsRootSettings, fs_menu_cb, app);
        submenu_add_item(app->submenu, "About", FsRootAbout, fs_menu_cb, app);
    } else if(menu == FsMenuEngagement) {
        submenu_set_header(app->submenu, "Engagement Setup");
        submenu_add_item(app->submenu, "View status", 0, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Set engagement", 1, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Set target", 2, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Set scope", 3, fs_menu_cb, app);
        submenu_add_item(app->submenu, app->engagement.authorized ? "Authorization: YES" : "Authorization: NO", 4, fs_menu_cb, app);
    } else if(menu == FsMenuTargets) {
        fs_refresh_targets(app);
        submenu_set_header(app->submenu, "Targets");
        submenu_add_item(app->submenu, "+ Add target", 0, fs_menu_cb, app);
        for(size_t i = 0; i < app->target_count; i++) submenu_add_item(app->submenu, app->targets[i], i + 1, fs_menu_cb, app);
    } else if(menu == FsMenuTargetProfile) {
        submenu_set_header(app->submenu, "Target Intelligence");
        submenu_add_item(app->submenu, "View profile", 0, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Set type", 1, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Set IP/network ID", 2, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Set MAC/BSSID", 3, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Set firmware", 4, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Set interfaces", 5, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Set notes", 6, fs_menu_cb, app);
    } else if(menu == FsMenuInstruments) {
        submenu_set_header(app->submenu, "Live Workbenches");
        submenu_add_item(app->submenu, "UART Workbench", FsInstrumentUart, fs_menu_cb, app);
        submenu_add_item(app->submenu, "UART Session Review", FsInstrumentUartReview, fs_menu_cb, app);
        submenu_add_item(app->submenu, "UART Extract Strings", FsInstrumentUartStrings, fs_menu_cb, app);
        submenu_add_item(app->submenu, "I2C Workbench", FsInstrumentI2c, fs_menu_cb, app);
        submenu_add_item(app->submenu, "I2C Topology History", FsInstrumentI2cHistory, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Wi-Fi Workbench", FsInstrumentWifiBoard, fs_menu_cb, app);
        submenu_add_item(app->submenu, "Wi-Fi Survey History", FsInstrumentWifiHistory, fs_menu_cb, app);
        submenu_add_item(app->submenu, "GPIO Monitor", FsInstrumentGpioGuide, fs_menu_cb, app);
    } else if(menu == FsMenuTraining) {
        submenu_set_header(app->submenu, "Training Academy");
        size_t count = 0;
        const FsLesson* lessons = fs_lessons_get(&count);
        for(size_t i = 0; i < count; i++) {
            char label[64];
            const char* mark = app->lesson_status[i] == FsLessonComplete ? "[x]" : (app->lesson_status[i] == FsLessonInProgress ? "[>]" : "[ ]");
            snprintf(label, sizeof(label), "%s %s", mark, lessons[i].title);
            submenu_add_item(app->submenu, label, i, fs_menu_cb, app);
        }
    } else {
        submenu_set_header(app->submenu, app->settings.mode == FsModeField ? "Modules | FIELD" : "Modules | LEARN");
        size_t count = 0;
        const FsModule* catalog = fs_catalog_get(&count);
        for(size_t i = 0; i < count; i++) submenu_add_item(app->submenu, catalog[i].name, i, fs_menu_cb, app);
    }
    fs_switch_view(app, FsViewMenu);
}

static bool fs_nav_cb(void* context) {
    FieldSecApp* app = context;
    if(app->active_view == FsViewTextInput) {
        app->input_purpose = FsInputNone;
        fs_build_menu(app, (FsMenuId)app->current_menu);
        return true;
    }
    if(app->active_view == FsViewSettings || app->active_view == FsViewDetail) {
        fs_build_menu(app, (FsMenuId)app->current_menu);
        return true;
    }
    if(app->current_menu != FsMenuRoot) {
        fs_build_menu(app, FsMenuRoot);
        return true;
    }
    view_dispatcher_stop(app->dispatcher);
    return true;
}

int32_t fieldsec_app(void* p) {
    UNUSED(p);
    FieldSecApp* app = malloc(sizeof(FieldSecApp));
    memset(app, 0, sizeof(FieldSecApp));
    app->selected = UINT32_MAX;
    app->settings.mode = FsModeLearn;
    app->settings.skill_level = FsSkillBeginner;
    app->settings.adaptive_guidance = true;
    app->settings.setup_complete = false;
    app->settings.severity = FsSeverityInfo;
    app->settings.uart_baud_index = 4;
    app->settings.uart_duration_index = 1;

    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    bool had_state = fs_state_load(app->storage, &app->engagement, &app->settings);
    fs_refresh_targets(app);
    if(app->engagement.target[0]) fs_target_profile_load(app->storage, app->engagement.target, &app->target_profile);
    fs_training_load(app->storage, app->lesson_status, app->lesson_practice, FS_MAX_LESSONS);

    app->dispatcher = view_dispatcher_alloc();
    app->submenu = submenu_alloc();
    app->widget = widget_alloc();
    app->text_input = text_input_alloc();
    app->settings_list = variable_item_list_alloc();

    view_dispatcher_set_event_callback_context(app->dispatcher, app);
    view_dispatcher_set_navigation_event_callback(app->dispatcher, fs_nav_cb);
    view_dispatcher_add_view(app->dispatcher, FsViewMenu, submenu_get_view(app->submenu));
    view_dispatcher_add_view(app->dispatcher, FsViewDetail, widget_get_view(app->widget));
    view_dispatcher_add_view(app->dispatcher, FsViewTextInput, text_input_get_view(app->text_input));
    view_dispatcher_add_view(app->dispatcher, FsViewSettings, variable_item_list_get_view(app->settings_list));

    view_dispatcher_attach_to_gui(app->dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    fs_build_menu(app, FsMenuRoot);
    if(!had_state || !app->settings.setup_complete) {
        fs_show_settings(app); /* First-run setup begins with Skill level + Adaptive guidance. */
    }
    view_dispatcher_run(app->dispatcher);

    fs_save_state(app);
    view_dispatcher_remove_view(app->dispatcher, FsViewSettings);
    view_dispatcher_remove_view(app->dispatcher, FsViewTextInput);
    view_dispatcher_remove_view(app->dispatcher, FsViewDetail);
    view_dispatcher_remove_view(app->dispatcher, FsViewMenu);
    variable_item_list_free(app->settings_list);
    text_input_free(app->text_input);
    widget_free(app->widget);
    submenu_free(app->submenu);
    view_dispatcher_free(app->dispatcher);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    free(app);
    return 0;
}
