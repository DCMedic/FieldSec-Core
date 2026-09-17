#include "fieldsec.h"
#include <stdio.h>
#include <string.h>

static size_t completed_lessons(const uint8_t status[], size_t n) {
    size_t c=0; for(size_t i=0;i<n;i++) if(status && status[i]==FsLessonComplete) c++; return c;
}
static size_t practice_runs(const uint8_t practice[], size_t n) {
    size_t c=0; for(size_t i=0;i<n;i++) if(practice) c += practice[i]; return c;
}
static const char* demonstrated_label(size_t evidence, size_t findings, size_t retests, size_t lessons, size_t practice) {
    if(retests && findings && evidence>=3 && lessons>=6 && practice>=3) return "Demonstrated";
    if(findings && evidence>=2 && practice) return "Developing";
    if(evidence || practice || lessons) return "Practicing";
    return "Not yet evidenced";
}

bool fs_competency_render(Storage* storage, const FsEngagement* e, const FsSettings* s,
    const uint8_t lesson_status[], const uint8_t lesson_practice[], size_t lesson_count,
    char* output, size_t output_size) {
    if(!storage || !e || !s || !output || output_size<64) return false;
    size_t ev=0, fnd=0, rt=0; if(e->target[0]) fs_target_stats(storage,e->target,&ev,&fnd,&rt);
    size_t done=completed_lessons(lesson_status,lesson_count), runs=practice_runs(lesson_practice,lesson_count);
    const char* demo=demonstrated_label(ev,fnd,rt,done,runs);
    snprintf(output,output_size,
        "COMPETENCY ENGINE\n\nSelected overall: %s\nEvidence-based state: %s\nTarget: %s\n\nPRACTICE EVIDENCE\nLessons complete: %u/%u\nPractice runs: %u\nOBS/FND/RT: %u/%u/%u\n\nDOMAIN STARTING LEVELS\nHardware: %s\nWireless: %s\nNFC/RFID: %s\nEvidence: %s\nThreat modeling: %s\n\nINTERPRETATION\nSelected skill controls instructional depth. Demonstrated competency is inferred only from completed learning/practice and assessment records; it never locks or unlocks tools. Instructor review remains authoritative for formal evaluation.",
        fs_skill_name(s->skill_level),demo,e->target[0]?e->target:"(no active target)",(unsigned)done,(unsigned)lesson_count,(unsigned)runs,(unsigned)ev,(unsigned)fnd,(unsigned)rt,
        fs_skill_name(s->domain_skill[FsDomainHardware]),fs_skill_name(s->domain_skill[FsDomainWireless]),fs_skill_name(s->domain_skill[FsDomainCredential]),fs_skill_name(s->domain_skill[FsDomainEvidence]),fs_skill_name(s->domain_skill[FsDomainThreatModeling]));
    return true;
}

static bool has(const char* s,const char* needle){ return s && needle && strstr(s,needle); }

bool fs_assessment_plan_render(Storage* storage, const FsEngagement* e, const FsSettings* s,
    const FsTargetProfile* p, char* output, size_t output_size) {
    if(!storage || !e || !s || !p || !output || output_size<64) return false;
    size_t ev=0,fnd=0,rt=0; if(e->target[0]) fs_target_stats(storage,e->target,&ev,&fnd,&rt);
    char steps[1500]="";
    strncat(steps,"1. Confirm written scope and target identity.\n",sizeof(steps)-strlen(steps)-1);
    if(has(p->interfaces,"Wi-Fi")||has(p->interfaces,"wifi")||p->mac[0])
        strncat(steps,"2. Establish/repeat passive Wi-Fi baseline; compare BSSID, channel and advertised security.\n",sizeof(steps)-strlen(steps)-1);
    if(has(p->interfaces,"UART")||has(p->interfaces,"uart"))
        strncat(steps,"3. Use receive-only UART observation before considering any interactive serial test.\n",sizeof(steps)-strlen(steps)-1);
    if(has(p->interfaces,"I2C")||has(p->interfaces,"i2c"))
        strncat(steps,"4. Establish I2C address-presence topology; compare only after preserving baseline.\n",sizeof(steps)-strlen(steps)-1);
    if(has(p->interfaces,"NFC")||has(p->interfaces,"RFID")||has(p->interfaces,"nfc")||has(p->interfaces,"rfid"))
        strncat(steps,"5. Inventory credential technology and trust assumptions with the NFC/RFID learning modules.\n",sizeof(steps)-strlen(steps)-1);
    strncat(steps,"6. Create a falsifiable HYP record before escalating validation.\n7. Retain OBS IDs, create FND only for validated impact, then record RT after remediation.\n",sizeof(steps)-strlen(steps)-1);
    snprintf(output,output_size,
        "GUIDED ASSESSMENT PLAN\n\nTarget: %s\nType: %s\nInterfaces: %s\nCurrent OBS/FND/RT: %u/%u/%u\nGuidance profile: %s\n\nLEAST-INVASIVE PLAN\n%s\nPLANNING RULE\nChoose the next test by information value and minimum target-state change. A plan is guidance, not proof; operator scope and target behavior control execution.",
        p->name[0]?p->name:(e->target[0]?e->target:"(none)"),p->type[0]?p->type:"unclassified",p->interfaces[0]?p->interfaces:"not mapped",(unsigned)ev,(unsigned)fnd,(unsigned)rt,fs_skill_name(s->skill_level),steps);
    return true;
}
