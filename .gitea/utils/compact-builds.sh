#!/bin/bash
# SPDX-License-Identifier: GPL-2.0-only
# SPDX-FileCopyrightText: 2026 KolibriOS team

# Shrink older builds on the storage server: compress their images and drop
# their unpacked tree, which is rebuildable from the commit. sha256sums.txt
# keeps naming the bare images, so a compacted build has to be decompressed
# before it can be verified.
#
# Layout:  <root>/<version>/<lang>/kolibrios-<descr>-<lang>.{img,iso,raw} -> .zst
#          <root>/<version>/<lang>/data/                                  -> removed
#
# Usage: compact-builds.sh <storage-root> [keep] [max-per-run]
#        DRY_RUN=1 walks the same selection and changes nothing

set -euo pipefail

root=${1:?storage root required}
# the build just published is always among these, so an upload in flight is safe
keep=${2:-5}
# a build is hundreds of MiB to re-compress: cap the first pass over the backlog
max=${3:-10}
dry=${DRY_RUN:-0}
verb=compacted
[ "$dry" = 1 ] && verb="would compact"

# --rm drops the source only once the archive is written; the box serves the
# site over NFS, so compaction yields to it
compress_image() {
    nice -n 10 ionice -c3 zstd -12 -T1 -q --rm "$1"
}
export -f compress_image

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

    if [ "$dry" = 1 ]; then
        compacted=$((compacted + 1))
        echo "$verb $version"
        continue
    fi

    # a build holds nine images: one per core, each zstd kept to one thread so
    # the two levels of parallelism do not fight. Archive before dropping the
    # tree, so a failure here leaves the build merely uncompacted and the next
    # run picks it up again
    find "$dir" -mindepth 2 -maxdepth 2 -type f \
         \( -name '*.img' -o -name '*.iso' -o -name '*.raw' \) -print0 \
        | xargs -0 -r -P "$(nproc)" -I{} bash -c 'compress_image "$@"' _ {}

    find "$dir" -mindepth 2 -maxdepth 2 -type d -name data -exec rm -rf {} +

    compacted=$((compacted + 1))
    echo "$verb $version"
# <tag>-<commit count>-g<sha>: order by the count, never by mtime
done < <(find "$root" -mindepth 1 -maxdepth 1 -type d -name '*-*-g*' -printf '%f\n' \
             | sort -t- -k2,2n | head -n "-$keep")

echo "$verb $compacted of $pending older build(s), $((pending - compacted)) left for the next run"
