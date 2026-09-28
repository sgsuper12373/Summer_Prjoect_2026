#!/usr/bin/env bash
# Converts every .egr graph in tests/ to BoruvkaUMinho's .edges format,
# writing output into tests_edges/ (created if missing).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"

CONVERTER="$ROOT_DIR/src/egr2edges"
INPUT_DIR="$ROOT_DIR/tests"
OUTPUT_DIR="$ROOT_DIR/tests_edges"

if [[ ! -x "$CONVERTER" ]]; then
    echo "ERROR: converter not found or not executable at $CONVERTER" >&2
    echo "Build it first: g++ -O2 -o src/egr2edges src/egr2edges.cpp" >&2
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

shopt -s nullglob
egr_files=("$INPUT_DIR"/*.egr)
shopt -u nullglob

if [[ ${#egr_files[@]} -eq 0 ]]; then
    echo "ERROR: no .egr files found in $INPUT_DIR" >&2
    exit 1
fi

echo "Found ${#egr_files[@]} .egr files. Converting..."
echo

# Same default as run_all_tests.sh, so both scripts generate identical
# weights for unweighted graphs unless WEIGHT_SEED is overridden by the caller.
WEIGHT_SEED="${WEIGHT_SEED:-20260907}"
fail_count=0
for egr in "${egr_files[@]}"; do
    name="$(basename "$egr" .egr)"
    out="$OUTPUT_DIR/${name}.edges"

    # Only pass the seed when needed; egr2edges rejects a seed arg it doesn't
    # need? -> no, it's fine either way since weighted graphs ignore it, but
    # we pass it unconditionally for simplicity since it's a no-op when the
    # graph already has real weights.
    if "$CONVERTER" "$egr" "$out" "$WEIGHT_SEED"; then
        :  # converter already prints its own success/note line
    else
        echo "FAILED: $egr" >&2
        fail_count=$((fail_count + 1))
    fi
done

echo
if [[ $fail_count -eq 0 ]]; then
    echo "All ${#egr_files[@]} graphs converted successfully into $OUTPUT_DIR (weight seed: ${WEIGHT_SEED})"
else
    echo "$fail_count of ${#egr_files[@]} conversions FAILED — check output above" >&2
    exit 1
fi