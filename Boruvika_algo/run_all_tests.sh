#!/bin/bash
set -euo pipefail

# Runs the Boruvka benchmark on every .egr file in the tests/ folder.
# Each batch writes its timings to Results/<timestamp>/<testfile>_result.csv.

BIN=./a.out
SRC=ECL_boruvkas.cpp
TESTS_DIR=./tests
RESULTS_DIR="Results/$(date +%Y%m%d_%H%M%S)"

# Compile the benchmark (OpenMP + C++17 for std::filesystem).
echo "Compiling ${SRC}..."
g++ -O3 -fopenmp -std=c++17 "${SRC}" -o "${BIN}"

mkdir -p "${RESULTS_DIR}"

graphs=("internet.egr" "USA-road-d.NY.egr")
threads=(1 2 4 8 12 16)
chunks=(12 16 20)

for g in "${graphs[@]}"; do
    file="${TESTS_DIR}/${g}"
    if [[ ! -f "$file" ]]; then
        echo "File $file not found! Skipping..."
        continue
    fi
    
    echo "Running tests on ${file}"
    echo "=================================================="
    
    for t in "${threads[@]}"; do
        for c in "${chunks[@]}"; do
            echo ""
            echo ">>> Running on: ${file} (threads=${t}, chunk_size=${c})"
            "${BIN}" "${file}" -n "${t}" -p0 "${c}" --results-dir "${RESULTS_DIR}"
        done
    done
done

echo ""
echo "=================================================="
echo "All tests done. CSV results are stored in ${RESULTS_DIR}."
