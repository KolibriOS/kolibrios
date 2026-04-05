#!/usr/bin/env python3
# Sync tup's mtime/mtime_ns in .tup/db with the actual filesystem ctime.
#
# On Linux, tup uses st_ctim (ctime) for change detection, not st_mtim.
# After restoring build cache with cp -a, files get new ctimes while the
# database still has old values. This script updates the database to match.
#
# Usage: python3 sync-tup-ctime.py [path/to/.tup/db]

import os
import sqlite3
import sys

dbpath = sys.argv[1] if len(sys.argv) > 1 else '.tup/db'
if not os.path.isfile(dbpath):
    print(f'Database not found: {dbpath}')
    sys.exit(1)

db = sqlite3.connect(dbpath)

# Build dir_id -> (parent_id, name) mapping
# type=0: TUP_NODE_DIR, type=2: TUP_NODE_FILE
dirs = {}
for row in db.execute('SELECT id, dir, name FROM node WHERE type IN (0, 2)'):
    dirs[row[0]] = (row[1], row[2])

def resolve_path(dir_id):
    parts = []
    while dir_id in dirs:
        parent, name = dirs[dir_id]
        parts.append(name)
        dir_id = parent
    return '/'.join(reversed(parts))

updated = 0
skipped = 0

# Update all tracked node types: dirs (0), files (2), generated (4)
for node_type in (0, 2, 4):
    for node_id, dir_id, name, db_sec, db_ns in db.execute(
        'SELECT id, dir, name, mtime, mtime_ns FROM node WHERE type=?',
        (node_type,)
    ):
        dirpath = resolve_path(dir_id)
        filepath = os.path.join(dirpath, name) if dirpath else name
        try:
            st = os.stat(filepath)
        except OSError:
            skipped += 1
            continue

        fs_sec = int(st.st_ctime)
        fs_ns = st.st_ctime_ns - fs_sec * 1_000_000_000
        if db_sec != fs_sec or db_ns != fs_ns:
            db.execute(
                'UPDATE node SET mtime=?, mtime_ns=? WHERE id=?',
                (fs_sec, fs_ns, node_id),
            )
            updated += 1

db.commit()
db.close()
print(f'Updated ctime for {updated} nodes, skipped {skipped} missing files')
