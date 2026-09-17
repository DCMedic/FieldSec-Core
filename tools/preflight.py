#!/usr/bin/env python3
import shutil,subprocess,sys
from pathlib import Path
root=Path(__file__).resolve().parents[1]
print('FieldSec 1.0 release preflight')
print('Python:',sys.version.split()[0])
for cmd in ['ufbt','idf.py']:
    path=shutil.which(cmd)
    print(f'{cmd}:', path or 'NOT FOUND')
print('Source validator:', 'present' if (root/'tools/validate_release.py').exists() else 'MISSING')
print('Release runbook:', 'present' if (root/'docs/RELEASE_RUNBOOK.md').exists() else 'MISSING')
print('Note: missing ufbt/idf.py prevents final device builds but does not invalidate source-only checks.')
