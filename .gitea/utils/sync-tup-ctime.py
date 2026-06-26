#!/usr/bin/env python3
# Sync tup's mtime/mtime_ns in .tup/db with the actual filesystem ctime.
#
# On Linux, tup uses st_ctim (ctime) for change detection, not st_mtim.
# After restoring a build cache with cp -a, files get new ctimes while the
# database still has old values. This script updates the database to match.
#
# Files in changed-files-list (changed since the cache's commit) are forced
# dirty instead, so tup still rebuilds them and their dependents.
#
# Usage: python3 sync-tup-ctime.py [path/to/.tup/db] [changed-files-list]

import os
import sqlite3
import sys

db_path = sys.argv[1] if len(sys.argv) > 1 else ".tup/db"
if not os.path.isfile(db_path):
    sys.exit(f"Database not found: {db_path}")

# Paths changed since the cache's commit: keep dirty so tup rebuilds them.
dirty = set()
if len(sys.argv) > 2 and sys.argv[2]:
    if os.path.isfile(sys.argv[2]):
        with open(sys.argv[2], encoding="utf-8") as f:
            dirty = {line.strip() for line in f if line.strip()}
    else:
        print(f"Warning: changed-files list not found: {sys.argv[2]}")

db = sqlite3.connect(db_path)
# Map every node id -> (parent dir id, name). Any node type can be a parent
# directory in the chain (generated/variant dirs are not type 0/2), so the map
# must cover all of them or path resolution yields a bare filename and stat fails.
nodes = {
    node_id: (parent_id, name)
    for node_id, parent_id, name in db.execute(
        "select id, dir, name from node"
    )
}
paths, updated, forced, skipped = {}, 0, 0, []
matched = set()  # dirty paths that actually resolved to a tup node

for node_id, dir_id, name, ntype, old_sec, old_ns in db.execute(
    "select id, dir, name, type, mtime, mtime_ns from node where type in (0, 2, 4)"
):
    if dir_id not in paths:
        parts, cur, seen = [], dir_id, set()
        while cur in nodes and cur not in seen:
            seen.add(cur)
            cur, part = nodes[cur]
            parts.append(part)
        paths[dir_id] = "/".join(reversed(parts))
    # Build the path with forward slashes (tup stores them that way and git
    # emits them that way), independent of the host os.sep. tup's root dir
    # node is named ".", so paths come out as "./foo/bar"; strip that prefix
    # so they match git's plain "foo/bar" entries in the dirty set.
    path = paths[dir_id] + "/" + name if paths[dir_id] else name
    if path.startswith("./"):
        path = path[2:]

    # Changed since the cache base: force a rebuild instead of aligning.
    if path in dirty:
        matched.add(path)
        if old_sec != 0 or old_ns != 0:
            db.execute(
                "update node set mtime=0, mtime_ns=0 where id=?", (node_id,))
            forced += 1
        continue

    try:
        stat = os.stat(path)
    except OSError:
        skipped.append((path, ntype))
        continue
    sec = int(stat.st_ctime)
    ns = stat.st_ctime_ns - sec * 1_000_000_000
    if sec == old_sec and ns == old_ns:
        continue
    db.execute("update node set mtime=?, mtime_ns=? where id=?", (sec, ns, node_id))
    updated += 1

db.commit()
db.close()

print(f"Aligned {updated} nodes, forced {forced} changed nodes dirty "
      f"({len(matched)}/{len(dirty)} changed paths matched a tup node), "
      f"skipped {len(skipped)} missing files")

# A changed path that matches no tup node is normal (docs, CI files, etc.),
# but if NONE of them match while the list is non-empty, the path format from
# git almost certainly disagrees with tup's node paths -- which means changed
# files would silently NOT be rebuilt. Make that loud instead of silent.
if dirty and not matched:
    print("WARNING: no changed path matched a tup node -- path format "
          "mismatch likely; changed files may NOT be rebuilt.")

if skipped:
    from collections import Counter
    by_type = Counter(ntype for _, ntype in skipped)
    print("Skipped by node type: "
          + ", ".join(f"type {t}: {c}" for t, c in sorted(by_type.items())))
