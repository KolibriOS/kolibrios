#!/usr/bin/env python3
# Restore file modification times from git history.
# Usage: git log --pretty=format:%at --name-only --diff-filter=ACMR | python3 restore-mtime.py

import os
import sys

seen = set()
mtime = None

for line in sys.stdin:
    line = line.rstrip('\n')
    if line.isdigit():
        mtime = int(line)
    elif line and mtime is not None and line not in seen:
        seen.add(line)
        if os.path.exists(line):
            os.utime(line, (mtime, mtime))
