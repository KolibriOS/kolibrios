#!/bin/bash
# SPDX-License-Identifier: GPL-2.0-only
# SPDX-FileCopyrightText: 2026 KolibriOS team

# Shrink older builds on the storage server: zip their images and drop their
# unpacked tree, which is rebuildable from the commit. sha256sums.txt keeps
# naming the bare images, so a compacted build has to be unzipped to verify.
#
# Layout:  <root>/<version>/<lang>/kolibrios-<descr>-<lang>.{img,iso,raw} -> .zip
#          <root>/<version>/<lang>/data/                                  -> removed
#
# Usage: compact-builds.sh <storage-root> [keep] [max-per-run]

set -euo pipefail

root=${1:?storage root required}
# the build just published is always among these, so an upload in flight is safe
keep=${2:-5}
# a build is hundreds of MiB to re-compress: cap the first pass over the backlog
max=${3:-10}

leftovers() {
    local build=$1
    find "$build" -mindepth 2 -maxdepth 2 \
        \( -type d -name data \
           -o -type f \( -name '*.img' -o -name '*.iso' -o -name '*.raw' \) \) \
        -print -quit
}

compacted=0
pending=0

while IFS= read -r version; do
    dir="$root/$version"
    [ -n "$(leftovers "$dir")" ] || continue

    pending=$((pending + 1))
    [ "$compacted" -lt "$max" ] || continue

    find "$dir" -mindepth 2 -maxdepth 2 -type d -name data -exec rm -rf {} +

    # zip -m drops the source only once the archive is written
    while IFS= read -r -d '' image; do
        ( cd "$(dirname "$image")" \
          && zip -9 -q -m "$(basename "$image").zip" "$(basename "$image")" )
    done < <(find "$dir" -mindepth 2 -maxdepth 2 -type f \
                  \( -name '*.img' -o -name '*.iso' -o -name '*.raw' \) -print0)

    compacted=$((compacted + 1))
    echo "compacted $version"
# <tag>-<commit count>-g<sha>: order by the count, never by mtime
done < <(find "$root" -mindepth 1 -maxdepth 1 -type d -name '*-*-g*' -printf '%f\n' \
             | sort -t- -k2,2n | head -n "-$keep")

echo "compacted $compacted of $pending older build(s), $((pending - compacted)) left for the next run"
