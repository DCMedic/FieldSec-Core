#include "fieldsec.h"
#include <stdio.h>
#include <string.h>

const char* fs_domain_name(FsSkillDomain domain) {
    switch(domain) {
    case FsDomainHardware: return "Hardware Security";
    case FsDomainWireless: return "Wireless Security";
    case FsDomainCredential: return "NFC/RFID";
    case FsDomainEvidence: return "Evidence/Reporting";
    case FsDomainThreatModeling: return "IoT Threat Modeling";
    default: return "Unknown";
    }
}

static const char* next_for(FsSkillLevel level) {
    switch(level) {
    case FsSkillBeginner: return "fundamentals + guided practice";
    case FsSkillIntermediate: return "baseline comparison + hypothesis building";
    case FsSkillAdvanced: return "cross-session correlation + contradictory evidence";
    default: return "concise field workflow + peer/instructor review";
    }
}

bool fs_curriculum_render(const FsSettings* settings, const FsTargetProfile* profile, char* output, size_t output_size) {
    if(!settings || !output || output_size < 2) return false;
    char rows[1200]="";
    for(size_t i=0;i<FsDomainCount;i++) {
        char line[220];
        snprintf(line,sizeof(line),"%s: %s\n  Next: %s\n",fs_domain_name((FsSkillDomain)i),fs_skill_name(settings->domain_skill[i]),next_for(settings->domain_skill[i]));
        strncat(rows,line,sizeof(rows)-strlen(rows)-1);
    }
    snprintf(output,output_size,
        "ADAPTIVE CURRICULUM\n\nOverall profile: %s\nTarget: %s\n\nDOMAIN MATRIX\n%s\nCAPABILITY RULE\nDomain proficiency changes explanation depth and recommended sequencing only. No tool, lesson, record type, or lower-level workflow is removed.",
        fs_skill_name(settings->skill_level), (profile && profile->name[0])?profile->name:"(none selected)", rows);
    return true;
}
