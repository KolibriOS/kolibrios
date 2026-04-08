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

db_path = sys.argv[1] if len(sys.argv) > 1 else ".tup/db"
if not os.path.isfile(db_path):
    sys.exit(f"Database not found: {db_path}")

db = sqlite3.connect(db_path)
nodes = {
    node_id: (parent_id, name)
    for node_id, parent_id, name in db.execute(
        "select id, dir, name from node where type in (0, 2)"
    )
}
paths, updated, skipped = {}, 0, 0

for node_id, dir_id, name, old_sec, old_ns in db.execute(
    "select id, dir, name, mtime, mtime_ns from node where type in (0, 2, 4)"
):
    if dir_id not in paths:
        parts, cur = [], dir_id
        while cur in nodes:
            cur, part = nodes[cur]
            parts.append(part)
        paths[dir_id] = "/".join(reversed(parts))
    path = os.path.join(paths[dir_id], name) if paths[dir_id] else name
    try:
        stat = os.stat(path)
    except OSError:
        skipped += 1
        continue
    sec = int(stat.st_ctime)
    ns = stat.st_ctime_ns - sec * 1_000_000_000
    if sec == old_sec and ns == old_ns:
        continue
    db.execute("update node set mtime=?, mtime_ns=? where id=?", (sec, ns, node_id))
    updated += 1

db.commit()
db.close()

print(f"Updated ctime for {updated} nodes, skipped {skipped} missing files")
