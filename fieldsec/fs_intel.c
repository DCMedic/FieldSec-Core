#include "fieldsec.h"
#include <storage/storage.h>
#include <stdio.h>
#include <string.h>

#define FS_DIR APP_DATA_PATH("fieldsec")
#define FS_HYPOTHESES FS_DIR "/hypotheses.csv"
#define FS_JOURNAL FS_DIR "/journal.csv"
#define FS_ANNOTATIONS FS_DIR "/annotations.csv"
#define FS_INTERFACES FS_DIR "/interfaces.csv"
#define FS_SERVICES FS_DIR "/services.csv"

static bool intel_mkdir(Storage* storage) {
    storage_common_mkdir(storage, FS_DIR);
    return true;
}

static void csv_clean(char* out, size_t n, const char* in) {
    if(!out || !n) return;
    size_t j=0;
    for(size_t i=0; in && in[i] && j+1<n; i++) {
        char c=in[i];
        if(c=='"') c='\'';
        if(c=='\r' || c=='\n') c=' ';
        out[j++]=c;
    }
    out[j]='\0';
}

static bool append_line(Storage* storage, const char* path, const char* line) {
    File* f=storage_file_alloc(storage);
    bool ok=storage_file_open(f,path,FSAM_WRITE,FSOM_OPEN_APPEND);
    if(ok) ok=storage_file_write(f,line,strlen(line))==strlen(line);
    storage_file_close(f); storage_file_free(f); return ok;
}

static bool read_text_local(Storage* storage, const char* path, char* out, size_t n) {
    if(!out || n<2) return false; out[0]='\0';
    File* f=storage_file_alloc(storage);
    if(!storage_file_open(f,path,FSAM_READ,FSOM_OPEN_EXISTING)){storage_file_free(f);return false;}
    size_t got=storage_file_read(f,out,n-1); out[got]='\0';
    storage_file_close(f); storage_file_free(f); return true;
}

static uint32_t next_id(Storage* storage,const char* path) {
    char buf[8192]; if(!read_text_local(storage,path,buf,sizeof(buf))) return 1;
    uint32_t n=0; for(char* p=buf; *p; p++) if(*p=='\n') n++; return n+1;
}

static bool target_line(const char* line,const char* target) {
    if(!target||!target[0]) return true; char needle[FS_TARGET_MAX+4];
    snprintf(needle,sizeof(needle),"\"%s\"",target); return strstr(line,needle)!=NULL;
}

static bool render_recent(Storage* storage,const char* path,const char* target,char* output,size_t output_size,const char* empty) {
    output[0]='\0'; char buf[8192];
    if(!read_text_local(storage,path,buf,sizeof(buf))){snprintf(output,output_size,"%s",empty);return true;}
    char keep[8][240]; size_t kept=0; char* save=NULL; char* line=strtok_r(buf,"\n",&save);
    while(line){
        if(target_line(line,target)){
            size_t slot=kept<8?kept++:7;
            if(kept>=8 && slot==7) for(size_t i=1;i<8;i++) snprintf(keep[i-1],sizeof(keep[i-1]),"%s",keep[i]);
            snprintf(keep[slot],sizeof(keep[slot]),"%.235s",line);
        }
        line=strtok_r(NULL,"\n",&save);
    }
    if(!kept){snprintf(output,output_size,"%s",empty);return true;}
    size_t used=0; for(size_t i=0;i<kept;i++){int w=snprintf(output+used,output_size-used,"%s%s",i?"\n\n":"",keep[i]); if(w<=0||(size_t)w>=output_size-used) break; used+=(size_t)w;}
    return true;
}

bool fs_hypothesis_append(Storage* storage,const FsEngagement* e,const char* statement,const char* test_plan,const char* expected,const char* observed,const char* status,char* out_id,size_t out_id_size){
    if(!storage||!e||!statement||!statement[0]) return false; intel_mkdir(storage);
    uint32_t n=next_id(storage,FS_HYPOTHESES); char id[FS_ID_MAX]; snprintf(id,sizeof(id),"HYP-%06lu",(unsigned long)n);
    char en[80],t[80],s[220],tp[220],ex[220],ob[220]; csv_clean(en,sizeof(en),e->engagement);csv_clean(t,sizeof(t),e->target);csv_clean(s,sizeof(s),statement);csv_clean(tp,sizeof(tp),test_plan);csv_clean(ex,sizeof(ex),expected);csv_clean(ob,sizeof(ob),observed);
    char st[64]; csv_clean(st,sizeof(st),(status&&status[0])?status:((observed&&observed[0])?"INCONCLUSIVE":"UNTESTED")); char line[1200];
    snprintf(line,sizeof(line),"\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",id,en,t,s,tp,ex,ob,st);
    bool ok=append_line(storage,FS_HYPOTHESES,line); if(ok&&out_id&&out_id_size) snprintf(out_id,out_id_size,"%s",id); return ok;
}

bool fs_hypothesis_render(Storage* storage,const char* target,char* output,size_t output_size){
    return render_recent(storage,FS_HYPOTHESES,target,output,output_size,"No hypotheses for the active target. Create a testable statement, predicted result, and least-invasive test plan before escalating activity.");
}

bool fs_journal_append(Storage* storage,const FsEngagement* e,const char* note,char* out_id,size_t out_id_size){
    if(!storage||!e||!note||!note[0])return false; intel_mkdir(storage); uint32_t n=next_id(storage,FS_JOURNAL);char id[FS_ID_MAX];snprintf(id,sizeof(id),"JRN-%06lu",(unsigned long)n);
    char en[80],t[80],no[256];csv_clean(en,sizeof(en),e->engagement);csv_clean(t,sizeof(t),e->target);csv_clean(no,sizeof(no),note);char line[600];snprintf(line,sizeof(line),"\"%s\",\"%s\",\"%s\",\"%s\"\n",id,en,t,no);bool ok=append_line(storage,FS_JOURNAL,line);if(ok&&out_id&&out_id_size)snprintf(out_id,out_id_size,"%s",id);return ok;
}
bool fs_journal_render(Storage* storage,const char* target,char* output,size_t output_size){return render_recent(storage,FS_JOURNAL,target,output,output_size,"No engagement journal entries for the active target.");}

bool fs_annotation_append(Storage* storage,const FsEngagement* e,const char* obs_id,const char* note,char* out_id,size_t out_id_size){
    if(!storage||!e||!obs_id||!obs_id[0]||!note||!note[0])return false;intel_mkdir(storage);uint32_t n=next_id(storage,FS_ANNOTATIONS);char id[FS_ID_MAX];snprintf(id,sizeof(id),"ANN-%06lu",(unsigned long)n);
    char en[80],t[80],o[40],no[256];csv_clean(en,sizeof(en),e->engagement);csv_clean(t,sizeof(t),e->target);csv_clean(o,sizeof(o),obs_id);csv_clean(no,sizeof(no),note);char line[700];snprintf(line,sizeof(line),"\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",id,en,t,o,no);bool ok=append_line(storage,FS_ANNOTATIONS,line);if(ok&&out_id&&out_id_size)snprintf(out_id,out_id_size,"%s",id);return ok;
}
bool fs_annotation_render(Storage* storage,const char* target,char* output,size_t output_size){return render_recent(storage,FS_ANNOTATIONS,target,output,output_size,"No evidence annotations for the active target.");}

static bool map_append(Storage* storage,const char* path,const char* prefix,const FsEngagement* e,const char* note){
    if(!storage||!e||!note||!note[0])return false;intel_mkdir(storage);uint32_t n=next_id(storage,path);char id[FS_ID_MAX];snprintf(id,sizeof(id),"%s-%06lu",prefix,(unsigned long)n);char en[80],t[80],no[256];csv_clean(en,sizeof(en),e->engagement);csv_clean(t,sizeof(t),e->target);csv_clean(no,sizeof(no),note);char line[650];snprintf(line,sizeof(line),"\"%s\",\"%s\",\"%s\",\"%s\"\n",id,en,t,no);return append_line(storage,path,line);
}
bool fs_interface_map_append(Storage* s,const FsEngagement* e,const char* n){return map_append(s,FS_INTERFACES,"IFC",e,n);}
bool fs_service_map_append(Storage* s,const FsEngagement* e,const char* n){return map_append(s,FS_SERVICES,"SVC",e,n);}

bool fs_target_map_render(Storage* storage,const char* target,char* output,size_t output_size){
    char a[1000],b[1000];render_recent(storage,FS_INTERFACES,target,a,sizeof(a),"No mapped interfaces.");render_recent(storage,FS_SERVICES,target,b,sizeof(b),"No mapped services.");snprintf(output,output_size,"INTERFACES\n%s\n\nSERVICES\n%s\n\nReasoning rule: inventory is not impact. A listening service, exposed bus, or physical port becomes a security finding only after the relevant trust or control failure is validated.",a,b);return true;
}

bool fs_instructor_review_render(Storage* storage,const FsEngagement* e,char* output,size_t output_size){
    char hyp[1200],ann[800];fs_hypothesis_render(storage,e?e->target:NULL,hyp,sizeof(hyp));fs_annotation_render(storage,e?e->target:NULL,ann,sizeof(ann));
    snprintf(output,output_size,"INSTRUCTOR REVIEW\n\nHYPOTHESES\n%s\n\nANNOTATIONS\n%s\n\nREASONING CHECKPOINTS\n1. Is each hypothesis falsifiable?\n2. Was an expected result stated before testing?\n3. Is the observed result separated from interpretation?\n4. Is contradictory evidence retained?\n5. Does each finding cite supporting OBS IDs?\n6. Was the least-invasive adequate test selected?\n7. Does the retest directly verify the recommended control?\n\nReview reasoning quality, not attack success.",hyp,ann);return true;
}
