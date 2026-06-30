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

shopt -s nullglob
files=("${TESTS_DIR}"/*.egr)
shopt -u nullglob

if [[ ${#files[@]} -eq 0 ]]; then
    echo "No .egr files found in ${TESTS_DIR}"
    exit 1
fi

echo "Found ${#files[@]} test file(s) in ${TESTS_DIR}"
echo "=================================================="

for file in "${files[@]}"; do
    echo ""
    echo ">>> Running on: ${file}"
    "${BIN}" "${file}" "${RESULTS_DIR}"
done

echo ""
echo "=================================================="
echo "All tests done. CSV results are in ${RESULTS_DIR}/"
