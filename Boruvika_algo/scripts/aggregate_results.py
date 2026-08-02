import os
import glob
import pandas as pd
import sys

def aggregate_csvs(results_dir):
    # Find all result and phase CSVs
    result_files = glob.glob(os.path.join(results_dir, "*_result.csv"))
    phase_files = glob.glob(os.path.join(results_dir, "*_phases.csv"))

    if not result_files and not phase_files:
        print(f"No CSV files found in {results_dir}")
        return

    # Process result CSVs
    if result_files:
        print(f"Aggregating {len(result_files)} result files...")
        combined_results = []
        for file in result_files:
            df = pd.read_csv(file)
            combined_results.append(df)
        
        results_df = pd.concat(combined_results, ignore_index=True)
        results_out = os.path.join(results_dir, "combined_results.csv")
        results_df.to_csv(results_out, index=False)
        print(f"Saved combined results to {results_out}")

    # Process phase CSVs
    if phase_files:
        print(f"Aggregating {len(phase_files)} phase files...")
        combined_phases = []
        for file in phase_files:
            df = pd.read_csv(file)
            combined_phases.append(df)
        
        phases_df = pd.concat(combined_phases, ignore_index=True)
        phases_out = os.path.join(results_dir, "combined_phases.csv")
        phases_df.to_csv(phases_out, index=False)
        print(f"Saved combined phases to {phases_out}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python aggregate_results.py <results_dir>")
        sys.exit(1)
    
    results_dir = sys.argv[1]
    aggregate_csvs(results_dir)
