#!/bin/bash
# SPDX-License-Identifier: GPL-2.0-only
# SPDX-FileCopyrightText: 2026 KolibriOS team

# Shrink older builds on the storage server: zip their images and drop their
# unpacked tree, which is rebuildable from the commit. The current build is
# left alone. Distribution kits, checksums and build logs are never touched -
# note that sha256sums.txt keeps naming the bare images, so a compacted build
# has to be unzipped before verifying.
#
# Layout:  <root>/<version>/<lang>/kolibrios-<descr>-<lang>.{img,iso,raw} -> .zip
#          <root>/<version>/<lang>/data/                                  -> removed
#
# Usage: compact-builds.sh <storage-root> <current-version> [max-per-run]

set -euo pipefail

root=${1:?storage root required}
current=${2:?current version required}
# a raw image is 128 MiB apiece, so cap the first pass over the whole backlog
max=${3:-20}
# a build untouched for this long is not being uploaded by a concurrent deploy
quiet_minutes=30

# images and the unpacked tree both sit exactly two levels below <version>
leftovers() {
    find "$1" -mindepth 2 -maxdepth 2 \
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
done < <(find "$root" -mindepth 1 -maxdepth 1 -type d \
              ! -name "$current" -mmin "+$quiet_minutes" -printf '%f\n' | sort)

echo "compacted $compacted of $pending older build(s), $((pending - compacted)) left for the next run"
