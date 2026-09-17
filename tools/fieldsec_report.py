#!/usr/bin/env python3
"""FieldSec desktop companion: correlate observations, findings and retests.

Reporting and education only. This utility does not launch scans, exploit targets,
collect credentials, or automate active techniques.
"""
from __future__ import annotations
import argparse, csv, re
from pathlib import Path
from collections import defaultdict

SKILLS={'0':'Beginner','1':'Intermediate','2':'Advanced','3':'Expert/Field'}

ID_NUM = re.compile(r"-(\d+)$")

def read_csv(path: Path):
    if not path.exists(): return []
    with path.open(newline='', encoding='utf-8', errors='replace') as f:
        return list(csv.reader(f))

def load_profiles(path: Path):
    profiles={}
    if not path.exists(): return profiles
    for line in path.read_text(encoding='utf-8', errors='replace').splitlines():
        fields=line.split('\t')+['']*7
        name,typ,ip,mac,fw,interfaces,notes=fields[:7]
        if name: profiles[name]=dict(type=typ,ip=ip,mac=mac,firmware=fw,interfaces=interfaces,notes=notes)
    return profiles

def normalize_evidence(rows):
    out=[]
    for r in rows:
        if len(r)>=7 and r[0].startswith('OBS-'):
            out.append(dict(id=r[0], engagement=r[1], target=r[2], category=r[3], tool=r[4], risk=r[5], note=r[6]))
        elif len(r)>=6:
            out.append(dict(id='LEGACY-OBS', engagement=r[0], target=r[1], category=r[2], tool=r[3], risk=r[4], note=r[5]))
    return out

def normalize_findings(rows):
    out=[]
    for r in rows:
        if len(r)>=11 and r[0].startswith('FND-'):
            out.append(dict(id=r[0], engagement=r[1], target=r[2], severity=r[3], title=r[4], condition=r[5], evidence=r[6], impact=r[7], remediation=r[8], retest=r[9], status=r[10]))
        elif len(r)>=10:
            out.append(dict(id='LEGACY-FND', engagement=r[0], target=r[1], severity=r[2], title=r[3], condition=r[4], evidence=r[5], impact=r[6], remediation=r[7], retest=r[8], status=r[9]))
        elif len(r)>=5:
            out.append(dict(id='LEGACY-FND', engagement=r[0], target=r[1], severity=r[2], title=r[3], condition='', evidence=r[4], impact='', remediation='', retest='', status='Open'))
    return out

def normalize_retests(rows):
    return [dict(id=r[0], finding_id=r[1], engagement=r[2], target=r[3], result=r[4], note=r[5]) for r in rows if len(r)>=6]

def id_key(value):
    m=ID_NUM.search(value or '')
    return int(m.group(1)) if m else -1

def load_state(path: Path):
    d={}
    if path.exists():
        for line in path.read_text(encoding='utf-8',errors='replace').splitlines():
            if '=' in line:
                k,v=line.split('=',1); d[k]=v
    return d

def build_report(data_dir: Path) -> str:
    ev=normalize_evidence(read_csv(data_dir/'evidence.csv'))
    fi=normalize_findings(read_csv(data_dir/'findings.csv'))
    rt=normalize_retests(read_csv(data_dir/'retests.csv'))
    profiles=load_profiles(data_dir/'target_profiles.tsv')
    state=load_state(data_dir/'state.ini')
    hyp=[dict(id=r[0],engagement=r[1],target=r[2],statement=r[3],test_plan=r[4],expected=r[5],observed=r[6],status=r[7]) for r in read_csv(data_dir/'hypotheses.csv') if len(r)>=8]
    ann=[dict(id=r[0],engagement=r[1],target=r[2],obs_id=r[3],note=r[4]) for r in read_csv(data_dir/'annotations.csv') if len(r)>=5]
    journal=[dict(id=r[0],engagement=r[1],target=r[2],note=r[3]) for r in read_csv(data_dir/'journal.csv') if len(r)>=4]
    interfaces=[dict(id=r[0],engagement=r[1],target=r[2],note=r[3]) for r in read_csv(data_dir/'interfaces.csv') if len(r)>=4]
    services=[dict(id=r[0],engagement=r[1],target=r[2],note=r[3]) for r in read_csv(data_dir/'services.csv') if len(r)>=4]
    out=['# FieldSec v1.0 Engagement Report','']
    if not ev and not fi and not rt and not hyp and not journal:
        return '\n'.join(out+['No FieldSec assessment records were found.',''])

    engagements=defaultdict(lambda: {'evidence':[], 'findings':[], 'retests':[]})
    for x in ev: engagements[x['engagement'] or '(unnamed)']['evidence'].append(x)
    for x in fi: engagements[x['engagement'] or '(unnamed)']['findings'].append(x)
    for x in rt: engagements[x['engagement'] or '(unnamed)']['retests'].append(x)

    out += ['## Executive Summary','']
    if state:
        out += [f"- Learner skill profile: {SKILLS.get(state.get('skill','0'),'Beginner')}", f"- Adaptive guidance: {'On' if state.get('adaptive','1') != '0' else 'Off'}"]
        out += ['', '### Domain Skill Matrix', '']
        for label,key in [('Hardware Security','domain_hw'),('Wireless Security','domain_wireless'),('NFC/RFID','domain_credential'),('Evidence/Reporting','domain_evidence'),('IoT Threat Modeling','domain_threat')]:
            out.append(f"- {label}: {SKILLS.get(state.get(key,state.get('skill','0')),'Beginner')}")
    out += ['', '### Competency Context', '', 'Selected skill is instructional context; demonstrated competency should be supported by lesson/practice history and assessment records, and does not gate tools.', '']
    total_ev=len(ev); total_fi=len(fi); total_rt=len(rt)
    out += [f'- Retained observations: {total_ev}', f'- Structured findings: {total_fi}', f'- Retest records: {total_rt}', f'- Hypotheses: {len(hyp)}', f'- Evidence annotations: {len(ann)}', f'- Journal entries: {len(journal)}', '', 'FieldSec treats inventory, observation, hypothesis, finding, and retest as separate record classes. Counts alone are not a quality score; traceability and defensible reasoning matter more.', '']

    for eng,bucket in engagements.items():
        out += [f'## Engagement: {eng}','']
        targets=sorted({x['target'] or '(no target)' for kind in bucket.values() for x in kind})
        for target in targets:
            tev=[x for x in bucket['evidence'] if (x['target'] or '(no target)')==target]
            tfi=[x for x in bucket['findings'] if (x['target'] or '(no target)')==target]
            trt=[x for x in bucket['retests'] if (x['target'] or '(no target)')==target]
            out += [f'### Target: {target}','']
            p=profiles.get(target)
            if p:
                for label,key in [('Type','type'),('IP / network ID','ip'),('MAC / BSSID','mac'),('Firmware','firmware'),('Interfaces','interfaces'),('Notes','notes')]:
                    if p[key]: out.append(f'- {label}: {p[key]}')
                out.append('')
            out += [f'- Observations: {len(tev)}', f'- Findings: {len(tfi)}', f'- Retests: {len(trt)}','']
            if tev:
                out += ['#### Observation Timeline','']
                for x in sorted(tev,key=lambda q:id_key(q['id'])):
                    out.append(f"- **{x['id']}** — **{x['tool']}** ({x['category']}, {x['risk']}): {x['note']}")
                out += ['','**Teaching checkpoint:** Separate direct observation, change from baseline, and inference. Only the first two are automatically supported by the record.','']

    out += ['## Cross-Domain Target Relationship Graph','']
    graph_targets=sorted(set(profiles) | {x['target'] for x in ev+fi+rt if x.get('target')})
    for target in graph_targets:
        p=profiles.get(target,{})
        out += [f'### {target}','']
        if p.get('interfaces'): out.append(f"- Interfaces -> {p['interfaces']}")
        svc=[x for x in services if x['target']==target]
        if svc: out.append('- Services -> '+', '.join(x['id'] for x in svc))
        obs=[x for x in ev if x['target']==target]
        hy=[x for x in hyp if x['target']==target]
        fnd=[x for x in fi if x['target']==target]
        ret=[x for x in rt if x['target']==target]
        out.append(f"- Records -> {len(obs)} OBS / {len(hy)} HYP / {len(fnd)} FND / {len(ret)} RT")
        out += ['', 'Relationship edges provide context only; they do not establish causality or security impact.', '']

    out += ['## Hypothesis Register','']
    if hyp:
        for h in sorted(hyp,key=lambda q:id_key(q['id'])):
            out += [f"### {h['id']} — {h['status']}",'',f"- Engagement: {h['engagement'] or '(unnamed)'}",f"- Target: {h['target'] or '(no target)'}",'', '**Statement**', h['statement'] or '(not recorded)','', '**Least-invasive test plan**', h['test_plan'] or '(not recorded)','', '**Expected result (pre-test)**', h['expected'] or '(not recorded)','', '**Observed result**', h['observed'] or '(pending)','', '**Reasoning checkpoint:** A supported hypothesis is not automatically a finding. Validate the security consequence and retain the OBS records that prove it.','']
    else:
        out += ['No hypotheses recorded.','']

    out += ['## Target Inventory Maps','']
    all_targets=sorted({x['target'] for x in interfaces+services if x['target']})
    if all_targets:
        for target in all_targets:
            out += [f'### {target}','']
            for x in interfaces:
                if x['target']==target: out.append(f"- Interface **{x['id']}**: {x['note']}")
            for x in services:
                if x['target']==target: out.append(f"- Service **{x['id']}**: {x['note']}")
            out += ['', 'Inventory establishes attack surface context; it does not establish impact.', '']
    else:
        out += ['No interface/service inventory records.','']

    out += ['## Evidence Annotations','']
    if ann:
        known_obs={x['id'] for x in ev}
        for a in sorted(ann,key=lambda q:id_key(q['id'])):
            suffix='' if a['obs_id'] in known_obs else ' **[missing OBS reference]**'
            out.append(f"- **{a['id']}** → **{a['obs_id']}**: {a['note']}{suffix}")
        out.append('')
    else:
        out += ['No evidence annotations.','']

    out += ['## Engagement Journal','']
    if journal:
        for j in sorted(journal,key=lambda q:id_key(q['id'])): out.append(f"- **{j['id']}** [{j['target'] or 'no target'}]: {j['note']}")
        out.append('')
    else:
        out += ['No journal entries.','']

    out += ['## Findings and Traceability','']
    if not fi: out += ['No findings recorded.','']
    retests_by_finding=defaultdict(list)
    for x in rt: retests_by_finding[x['finding_id']].append(x)
    known_obs={x['id'] for x in ev}
    for f in fi:
        refs=sorted(set(re.findall(r'OBS-\d{6}', f['evidence'] or '')))
        missing=[x for x in refs if x not in known_obs]
        out += [f"### {f['id']}: {f['title']}",'',f"- Engagement: {f['engagement'] or '(unnamed)'}",f"- Target: {f['target'] or '(no target)'}",f"- Severity: {f['severity']}",f"- Status: {f['status']}",'', '**Condition**', f['condition'] or '(not recorded)','', '**Evidence references**', f['evidence'] or '(not recorded)','']
        if refs: out.append(f"Traceable observation IDs: {', '.join(refs)}")
        if missing: out.append(f"**Traceability warning:** referenced observations not present in this export: {', '.join(missing)}")
        out += ['','**Validated Impact**',f['impact'] or '(not recorded)','', '**Remediation**',f['remediation'] or '(not recorded)','', '**Retest Criteria**',f['retest'] or '(not recorded)','']
        matches=retests_by_finding.get(f['id'],[])
        if matches:
            out += ['**Retest Records**','']
            for x in sorted(matches,key=lambda q:id_key(q['id'])):
                out.append(f"- **{x['id']}** — {x['result']}: {x['note']}")
            out.append('')
        else:
            out += ['**Retest Records**','No linked retest record.','']
        out += ['**Quality checkpoint:** Does the stated impact follow from cited observations, and does the retest directly test the remediation success criterion?','']

    out += ['## Traceability Matrix','']
    if fi:
        for f in fi:
            refs=sorted(set(re.findall(r'OBS-\d{6}', f['evidence'] or '')))
            linked_rt=[x['id'] for x in rt if x['finding_id']==f['id']]
            out.append(f"- **{f['id']}** → OBS: {', '.join(refs) if refs else '(none cited)'} → RT: {', '.join(linked_rt) if linked_rt else '(none)'}")
        out.append('')
    else:
        out += ['No findings available for traceability matrix.','']

    out += ['## Adaptive Instruction Review','', f"Current profile: {SKILLS.get(state.get('skill','0'),'Beginner')}" if state else 'Current profile: not exported', 'Skill level changes guidance depth and recommended sequencing; it does not remove capabilities or invalidate lower-level lessons.', '', '## Instructor / Capstone Review','',
            '1. Was scope and authorization established before live work?',
            '2. Are at least three observations retained with unique OBS IDs?',
            '3. Does each finding distinguish condition from validated impact?',
            '4. Do findings cite the OBS IDs that actually support them?',
            '5. Is there at least one remediation retest linked by FND ID?',
            '6. Can another analyst reproduce the reasoning from evidence to finding to retest?',
            '7. Which conclusion remains an inference and therefore requires another test?',
            '8. Were hypotheses written before testing with an expected result that could be falsified?',
            '9. Were inventory observations kept separate from validated security findings?','']
    return '\n'.join(out)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('data_dir',type=Path)
    ap.add_argument('-o','--output',type=Path,default=Path('fieldsec_report.md'))
    args=ap.parse_args()
    args.output.write_text(build_report(args.data_dir),encoding='utf-8')
    print(args.output)

if __name__=='__main__': main()
