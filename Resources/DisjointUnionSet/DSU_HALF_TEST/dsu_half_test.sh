#!/bin/bash
set -euo pipefail

# Compile the current DSU implementation
g++ ../DSU.cpp -o ../DSU

PASS=0
FAIL=0

for file in ./test_*.txt; do
    if [[ "$file" == *.golden.txt ]]; then
        continue
    fi

    base=$(basename "$file" .txt)
    golden="./${base}.golden.txt"

    if [[ ! -f "$golden" ]]; then
        echo "[ERROR] missing golden file for $file"
        FAIL=$((FAIL + 1))
        continue
    fi

    header=$(head -n1 "$file" || true)
    N=$(awk '{print $2}' <<< "$header")

    if [[ -z "$N" ]]; then
        echo "[ERROR] invalid header in $file"
        FAIL=$((FAIL + 1))
        continue
    fi

    output=$(mktemp)
    tail -n +2 "$file" | ../DSU 2 "$N" > "$output"

    if diff -u "$golden" "$output" > /dev/null; then
        echo "$base: PASS"
        PASS=$((PASS + 1))
    else
        echo "$base: FAIL"
        diff -u "$golden" "$output" || true
        FAIL=$((FAIL + 1))
    fi

    rm -f "$output"
done

echo "\nRESULT: Passed=$PASS Failed=$FAIL"
if [[ $FAIL -gt 0 ]]; then
    exit 1
fi
