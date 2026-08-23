#!/bin/bash
set -euo pipefail

# Runs the Boruvka benchmark on every .egr file in the tests/ folder.
# Each batch writes its timings to Results/<timestamp>/<testfile>_result.csv.

# Change to the root project directory so paths work everywhere
cd "$(dirname "$0")/.."

BIN=./a.out
SRC=src/ECL_boruvkas.cpp
TESTS_DIR=./tests
RESULTS_DIR="Results/$(date +%Y%m%d_%H%M%S)"

# Compile the benchmark (OpenMP + C++17 for std::filesystem).
echo "Compiling ${SRC}..."
g++ -O3 -fopenmp -std=c++17 "${SRC}" -o "${BIN}"

mkdir -p "${RESULTS_DIR}"

# graphs=("internet.egr" "USA-road-d.NY.egr" "2d-2e20.sym.egr" "amazon0601.egr" "as-skitter.egr" "cit-Patents.egr" "citationCiteseer.egr" "coPapersDBLP.egr" "delaunay_n24.egr" )
graphs=( "2d-2e20.sym.egr"  "citationCiteseer.egr"  "delaunay_n24.egr"  "r4-2e23.sym.egr"       "soc-LiveJournal1.egr"
         "amazon0601.egr"   "cit-Patents.egr"       "europe_osm.egr"    "in-2004.egr"           "rmat16.sym.egr"        "USA-road-d.NY.egr"
         "as-skitter.egr"   "coPapersDBLP.egr"      "internet.egr"      "rmat22.sym.egr"        "USA-road-d.USA.egr" )

threads=(1 2 4 8 12 16)
chunks=(12 16 20)
algo=("serial_half" "omp_half" "omp_intermediate")
N_RUNS=9

for g in "${graphs[@]}"; do
    file="${TESTS_DIR}/${g}"
    if [[ ! -f "$file" ]]; then
        echo "File $file not found! Skipping..."
        continue
    fi
    
    echo "Running tests on ${file}"
    echo "=================================================="

    # Run serial algorithm once per graph
    for a in "serial_half"; do
        for run in $(seq 1 $N_RUNS); do
            echo ""
            echo ">>> Running on: ${file} (threads=1, chunk_size=16), algo=${a}, run=${run}/${N_RUNS}"
            "${BIN}" "${file}" -n 1 -p0 16 --results-dir "${RESULTS_DIR}" -algo "${a}"
        done
    done
    
    # Run parallel algorithms for different thread and chunk configurations
    for t in "${threads[@]}"; do
        for c in "${chunks[@]}"; do
            for a in "omp_half" "omp_intermediate"; do 
                for run in $(seq 1 $N_RUNS); do
                    echo ""
                    echo ">>> Running on: ${file} (threads=${t}, chunk_size=${c}), algo=${a}, run=${run}/${N_RUNS}"
                    "${BIN}" "${file}" -n "${t}" -p0 "${c}" --results-dir "${RESULTS_DIR}" -algo "${a}"
                done
            done 
        done
    done
done

echo ""
echo "=================================================="
echo "All tests done. CSV results are stored in ${RESULTS_DIR}."