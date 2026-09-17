#!/usr/bin/env python3
import csv,sys
from pathlib import Path
root=Path(__file__).resolve().parents[1]
if len(sys.argv)!=2:
    raise SystemExit('Usage: python tools/check_fap_api.py /path/to/api_symbols.csv')
api=Path(sys.argv[1])
required=[x.strip() for x in (root/'compat/required_fap_api_symbols.txt').read_text().splitlines() if x.strip() and not x.startswith('#')]
rows={}
with api.open(newline='',errors='replace') as f:
    for row in csv.DictReader(f):
        name=(row.get('name') or '').strip()
        if name: rows[name]=(row.get('status') or '').strip()
version='unknown'
with api.open(newline='',errors='replace') as f:
    for row in csv.DictReader(f):
        if (row.get('entry') or '').strip()=='Version': version=(row.get('name') or 'unknown').strip(); break
missing=[]; blocked=[]
for name in required:
    if name not in rows: missing.append(name)
    elif rows[name] != '+': blocked.append((name,rows[name]))
print(f'API version: {version}')
print(f'Required symbols checked: {len(required)}')
if missing: print('Missing:', ', '.join(missing))
if blocked: print('Unavailable:', ', '.join(f'{n}({s})' for n,s in blocked))
if missing or blocked: raise SystemExit(2)
print('FieldSec FAP API compatibility check: PASS')
