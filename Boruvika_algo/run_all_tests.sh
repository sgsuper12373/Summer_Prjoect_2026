#!/bin/bash
set -euo pipefail

# Runs the Boruvka benchmark on every .egr file in the tests/ folder.
# Each run writes its timings to Results/<testfile>_result.csv.

BIN=./a.out
SRC=ECL_boruvkas.cpp
TESTS_DIR=./tests
RESULTS_DIR=Results

# Compile the benchmark (OpenMP + C++17 for std::filesystem).
echo "Compiling ${SRC}..."
g++ -O3 -fopenmp -std=c++17 "${SRC}" -o "${BIN}"

mkdir -p "${RESULTS_DIR}"

file="${TESTS_DIR}/internet.egr"

if [[ ! -f "$file" ]]; then
    echo "File $file not found!"
    exit 1
fi

echo "Running tests on ${file}"
echo "=================================================="

threads=(1 2 3 4 8 16)
chunks=(12 16 20)

for t in "${threads[@]}"; do
    for c in "${chunks[@]}"; do
        echo ""
        echo ">>> Running on: ${file} (threads=${t}, chunk_size=${c})"
        "${BIN}" "${file}" -n ${t} -p0 ${c}
    done
done

echo ""
echo "=================================================="
echo "All tests done. CSV results are stored in the default results directory."
