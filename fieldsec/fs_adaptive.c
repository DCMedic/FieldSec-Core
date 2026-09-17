#include "fieldsec.h"
#include <stdio.h>
#include <string.h>

const char* fs_skill_name(FsSkillLevel level) {
    switch(level) {
    case FsSkillBeginner: return "Beginner";
    case FsSkillIntermediate: return "Intermediate";
    case FsSkillAdvanced: return "Advanced";
    case FsSkillExpert: return "Expert/Field";
    default: return "Beginner";
    }
}

static bool has_word(const char* a, const char* b) {
    if(!a || !b) return false;
    return strstr(a,b) != NULL;
}

bool fs_adaptive_render(const FsSettings* settings, const FsTargetProfile* profile, const uint8_t lesson_status[], const uint8_t lesson_practice[], size_t lesson_count, char* output, size_t output_size) {
    if(!settings || !output || output_size < 2) return false;
    size_t complete=0, practice=0;
    for(size_t i=0;i<lesson_count;i++){ if(lesson_status && lesson_status[i]==FsLessonComplete) complete++; if(lesson_practice) practice += lesson_practice[i]; }
    const char* base;
    switch(settings->skill_level) {
    case FsSkillBeginner:
        base="Start with Engagement Fundamentals, then Wi-Fi Recon, UART, I2C, and Evidence-to-Finding. FieldSec will emphasize definitions, wiring/setup, safe observation, and why an observation is not automatically a vulnerability."; break;
    case FsSkillIntermediate:
        base="Prioritize baseline comparison, hypothesis formation, structured evidence, and retesting. Use the same live tools, but focus on explaining why each next test is necessary and sufficient."; break;
    case FsSkillAdvanced:
        base="Prioritize session intelligence, contradictory evidence, target correlation, interface/service mapping, and traceability from OBS to FND to RT. Beginner lessons remain available for review."; break;
    default:
        base="Use concise field guidance by default. Prioritize hypothesis quality, efficient least-invasive validation, evidence correlation, retest discipline, and reporting. All instructional layers remain available on demand."; break;
    }
    char target[700]="No target-specific recommendation yet. Classify the target and observed interfaces to improve adaptation.";
    if(profile && profile->name[0]) {
        char rec[500]="";
        if(has_word(profile->interfaces,"UART") || has_word(profile->interfaces,"uart")) strncat(rec," UART: run receive-only capture and compare sessions.",sizeof(rec)-strlen(rec)-1);
        if(has_word(profile->interfaces,"I2C") || has_word(profile->interfaces,"i2c")) strncat(rec," I2C: establish a passive topology baseline and compare changes.",sizeof(rec)-strlen(rec)-1);
        if(has_word(profile->interfaces,"Wi-Fi") || has_word(profile->interfaces,"wifi") || profile->mac[0]) strncat(rec," Wi-Fi: use passive BSSID/channel/auth history before forming a security conclusion.",sizeof(rec)-strlen(rec)-1);
        if(has_word(profile->interfaces,"NFC") || has_word(profile->interfaces,"RFID")) strncat(rec," Credential interface: inventory technology and trust assumptions before testing controls.",sizeof(rec)-strlen(rec)-1);
        if(!rec[0]) snprintf(rec,sizeof(rec)," Use Target Intelligence to map observed physical, radio, and network interfaces before selecting the next module.");
        snprintf(target,sizeof(target),"Target: %s (%s).%s",profile->name,profile->type[0]?profile->type:"unclassified",rec);
    }
    snprintf(output,output_size,
        "ADAPTIVE LEARNING PATH\n\nSkill: %s\nAdaptive guidance: %s\nAcademy: %u/%u complete\nPractice runs: %u\n\nLEVEL GUIDANCE\n%s\n\nTARGET CORRELATION\n%s\n\nCAPABILITY RULE\nSkill level changes explanation depth and recommended order; it never removes tools, lessons, evidence formats, or lower-level workflows.",
        fs_skill_name(settings->skill_level),settings->adaptive_guidance?"ON":"OFF",(unsigned)complete,(unsigned)lesson_count,(unsigned)practice,base,target);
    return true;
}
