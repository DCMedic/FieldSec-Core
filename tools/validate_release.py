#!/usr/bin/env python3
from pathlib import Path
import re, py_compile, tempfile, csv, importlib.util, subprocess, sys
root=Path(__file__).resolve().parents[1]
required=[
 root/'README.md',root/'CHANGELOG.md',root/'fieldsec'/'fieldsec.c',root/'fieldsec'/'fieldsec.h',root/'fieldsec'/'fs_storage.c',root/'fieldsec'/'fs_catalog.c',root/'fieldsec'/'fs_targets.c',root/'fieldsec'/'fs_hardware.c',root/'fieldsec'/'fs_training.c',root/'fieldsec'/'fs_intel.c',root/'fieldsec'/'fs_adaptive.c',root/'fieldsec'/'fs_curriculum.c',root/'fieldsec'/'fs_graph.c',root/'fieldsec'/'fs_release.c',root/'fieldsec'/'application.fam',
 root/'devboard'/'main'/'main.c',root/'tools'/'fieldsec_report.py',root/'tools'/'parse_devboard.py',root/'tools'/'check_fap_api.py',root/'tools'/'preflight.py',
 root/'docs'/'V1_ENGINEERING_NOTES.md',root/'docs'/'V1_TEST_PLAN.md',root/'docs'/'RELEASE_RUNBOOK.md',root/'docs'/'RELEASE_NOTES_1.0.md',root/'docs'/'MIGRATION_TO_1.0.md',root/'docs'/'CAPABILITY_PRESERVATION.md',root/'compat'/'required_fap_api_symbols.txt']
missing=[str(x.relative_to(root)) for x in required if not x.exists()]
assert not missing, missing
h=(root/'fieldsec'/'fieldsec.h').read_text(); c=(root/'fieldsec'/'fieldsec.c').read_text(); st=(root/'fieldsec'/'fs_storage.c').read_text(); hw=(root/'fieldsec'/'fs_hardware.c').read_text(); db=(root/'devboard'/'main'/'main.c').read_text(); fam=(root/'fieldsec'/'application.fam').read_text(); cat=(root/'fieldsec'/'fs_catalog.c').read_text(); tr=(root/'fieldsec'/'fs_training.c').read_text(); report=(root/'tools'/'fieldsec_report.py').read_text(); cur=(root/'fieldsec'/'fs_curriculum.c').read_text(); graph=(root/'fieldsec'/'fs_graph.c').read_text(); rel=(root/'fieldsec'/'fs_release.c').read_text()
assert '#define FS_VERSION "1.0.0"' in h
assert 'fap_version="1.0.0"' in fam
assert 'FIELDSEC_DEVBOARD_VERSION "1.0.0"' in db
assert 'WIFI_SCAN_TYPE_PASSIVE' in db
assert 'furi_hal_serial_disable_direction(serial, FuriHalSerialDirectionTx)' in hw
assert 'fs_gpio_snapshot' in hw and 'furi_hal_resources_get_ext_pin_number' in hw and 'furi_hal_gpio_read' in hw
# GPIO v1.0 implementation must remain non-mutating in its function body.
gpio_body=hw.split('bool fs_gpio_snapshot',1)[1]
for forbidden in ['furi_hal_gpio_init(', 'furi_hal_gpio_init_simple(', 'furi_hal_gpio_init_ex(', 'furi_hal_gpio_write(']:
    assert forbidden not in gpio_body, f'GPIO snapshot became mutating: {forbidden}'
assert 'fs_competency_render' in rel and 'fs_assessment_plan_render' in rel
assert 'Competency Engine' in c and 'Assessment Planner' in c
assert 'FsDomainHardware' in h and 'FsDomainThreatModeling' in h and 'FsDomainCount' in h
for k in ['domain_hw','domain_wireless','domain_credential','domain_evidence','domain_threat']: assert k in st
assert 'v0.9 migration' in st
assert 'Adaptive Curriculum' in c and 'Target Relationship Graph' in c
assert 'CAPABILITY RULE' in cur and 'CORRELATION RULE' in graph
assert 'Domain Skill Matrix' in report and 'Cross-Domain Target Relationship Graph' in report
# non-regression: v1.0 adds capabilities without removing prior major workflows.
for needle in ['UART Workbench','UART Session Review','UART Extract Strings','I2C Workbench','I2C Topology History','Wi-Fi Workbench','Wi-Fi Survey History','GPIO Monitor','Training Academy','Hypotheses: View','Evidence Annotations','Engagement Journal','Finding Builder','Retest Logger','Capstone Status','After-Action Review','Adaptive Learning Path','Adaptive Curriculum','Target Relationship Graph']:
    assert needle in c, needle
module_count=len(re.findall(r'\{"[^\n]+FsRisk(?:Passive|Local|ActiveSafe)\}',cat))
lesson_count=len(re.findall(r'^\s*\{\n\s*"',tr,flags=re.M))
assert module_count>=15 and lesson_count==8
for path in [root/'fieldsec'/'fieldsec.c',root/'fieldsec'/'fs_hardware.c',root/'fieldsec'/'fs_targets.c',root/'fieldsec'/'fs_storage.c',root/'fieldsec'/'fs_intel.c',root/'fieldsec'/'fs_curriculum.c',root/'fieldsec'/'fs_graph.c',root/'fieldsec'/'fs_release.c']:
    text=path.read_text(); assert text.count('{')==text.count('}'), f'Brace mismatch {path.name}'
# No stale prerelease version literals in executable source/tool paths.
for path in list((root/'fieldsec').glob('*'))+list((root/'devboard').rglob('*.c'))+[x for x in (root/'tools').glob('*.py') if x.name!='validate_release.py']:
    if path.is_file() and path.suffix in {'.c','.h','.py','.fam'}:
        text=path.read_text(errors='replace'); assert '0.10.0' not in text, f'stale 0.10.0 in {path}'
for tool in ['fieldsec_report.py','parse_devboard.py','check_fap_api.py','preflight.py']:
    py_compile.compile(str(root/'tools'/tool),doraise=True)
spec=importlib.util.spec_from_file_location('r',root/'tools'/'fieldsec_report.py'); mod=importlib.util.module_from_spec(spec); spec.loader.exec_module(mod)
with tempfile.TemporaryDirectory() as td:
    d=Path(td)
    (d/'state.ini').write_text('skill=2\ndomain_hw=2\ndomain_wireless=1\ndomain_credential=0\ndomain_evidence=2\ndomain_threat=1\nadaptive=1\n')
    with (d/'evidence.csv').open('w',newline='') as f: csv.writer(f).writerow(['OBS-000001','Lab','Camera','Hardware/IoT','UART Workbench','LOCAL','Readable boot banner'])
    with (d/'hypotheses.csv').open('w',newline='') as f: csv.writer(f).writerow(['HYP-000001','Lab','Camera','UART may expose diagnostics','RX-only capture','Readable strings','OBS-000001','SUPPORTED'])
    with (d/'interfaces.csv').open('w',newline='') as f: csv.writer(f).writerow(['IFC-000001','Lab','Camera','3.3V UART'])
    with (d/'services.csv').open('w',newline='') as f: csv.writer(f).writerow(['SVC-000001','Lab','Camera','HTTPS'])
    with (d/'findings.csv').open('w',newline='') as f: csv.writer(f).writerow(['FND-000001','Lab','Camera','Medium','Debug output','UART readable','OBS-000001','Disclosure','Disable debug','No readable output','Open'])
    with (d/'retests.csv').open('w',newline='') as f: csv.writer(f).writerow(['RT-000001','FND-000001','Lab','Camera','Pass','No output'])
    text=mod.build_report(d)
    for n in ['Domain Skill Matrix','Hardware Security: Advanced','Wireless Security: Intermediate','NFC/RFID: Beginner','Cross-Domain Target Relationship Graph','Camera','1 OBS / 1 HYP / 1 FND / 1 RT','Traceability Matrix']:
        assert n in text,n
# API checker logic on a synthetic compatible table and one intentionally bad table.
req=[x.strip() for x in (root/'compat'/'required_fap_api_symbols.txt').read_text().splitlines() if x.strip() and not x.startswith('#')]
with tempfile.TemporaryDirectory() as td:
    good=Path(td)/'api.csv'
    with good.open('w',newline='') as f:
        w=csv.writer(f); w.writerow(['entry','status','name','type','params']); w.writerow(['Version','+','88.2','',''])
        for name in req: w.writerow(['Function' if not name.startswith('gpio_') and name!='furi_hal_i2c_handle_external' else 'Variable','+',name,'',''])
    subprocess.run([sys.executable,str(root/'tools'/'check_fap_api.py'),str(good)],check=True,stdout=subprocess.DEVNULL)
print(f'FieldSec v1.0 release validation OK: {module_count} assessment modules, {lesson_count} academy lessons, GPIO snapshot + competency/planner + release closure')
