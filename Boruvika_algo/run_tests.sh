#!/bin/bash
set -euo pipefail

# Compile the Boruvka CPU Naive implementation
g++ -O3 boruvka_cpu_naive.cpp -o boruvka_cpu_naive

PASS=0
FAIL=0

echo "Running Boruvka's MST Algorithm Tests..."

for file in ./tests/test_*.txt; do
    if [[ "$file" == *.golden.txt ]]; then
        continue
    fi

    base=$(basename "$file" .txt)
    golden="./tests/${base}.golden.txt"

    if [[ ! -f "$golden" ]]; then
        echo "[ERROR] missing golden file for $file"
        FAIL=$((FAIL + 1))
        continue
    fi

    output=$(mktemp)
    ./boruvka_cpu_naive -f "$file" > "$output"

    if diff -u "$golden" "$output" > /dev/null; then
        echo "  $base: PASS"
        PASS=$((PASS + 1))
    else
        echo "  $base: FAIL"
        diff -u "$golden" "$output" || true
        FAIL=$((FAIL + 1))
    fi

    rm -f "$output"
done

echo -e "\nRESULT: Passed=${PASS} Failed=${FAIL}"
if [[ $FAIL -gt 0 ]]; then
    exit 1
fi
