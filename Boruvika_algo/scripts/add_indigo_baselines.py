import os
import sys
import pandas as pd
import subprocess
import re

def add_indigo_baseline(results_dir):
    combined_csv = os.path.join(results_dir, "combined_results.csv")
    if not os.path.exists(combined_csv):
        print(f"Error: {combined_csv} not found.")
        return

    df = pd.read_csv(combined_csv)
    
    # We will add a new column 'indigo_omp_s' if it doesn't exist
    if 'indigo_omp_s' not in df.columns:
        df['indigo_omp_s'] = -1.0

    indigo_bin = "baselines/Indigo3Suite/executables/OMP/MST-OMP/MST_OMP_V_Data_IntType_Atomic_NoBoundsBug_NoFieldBug_Default_NoNbrBoundsBug"
    
    if not os.path.exists(indigo_bin):
        print(f"Error: Indigo binary not found at {indigo_bin}")
        print("Please ensure it was compiled first.")
        return

    # Get unique combinations of graphs and threads
    configs = df[['graph', 'threads']].drop_duplicates()
    
    for _, row in configs.iterrows():
        graph_name = row['graph']
        threads = row['threads']
        
        graph_path = f"tests/{graph_name}.egr"
        
        print(f"Running Indigo3 baseline on {graph_name} with {threads} threads...")
        
        # Run the command and capture output
        cmd = [indigo_bin, graph_path, str(threads)]
        try:
            # Note: Indigo3 runs the algorithm multiple times natively and prints the runtime
            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
            output = result.stdout
            
            # Parse the runtime
            match = re.search(r"runtime:\s+([0-9\.]+)s", output)
            if match:
                runtime_s = float(match.group(1))
                print(f"  -> Extracted runtime: {runtime_s}s")
                
                # Update the DataFrame for all rows matching this graph/thread config
                mask = (df['graph'] == graph_name) & (df['threads'] == threads)
                df.loc[mask, 'indigo_omp_s'] = runtime_s
            else:
                print(f"  -> Warning: Could not parse runtime from output.")
                print(f"     Output: {output}")
        except subprocess.CalledProcessError as e:
            print(f"  -> Error running command for {graph_name} (threads={threads})")
            print(e.stderr)
            
    # Save back to CSV
    df.to_csv(combined_csv, index=False)
    print(f"Updated {combined_csv} with Indigo3 baselines.")

if __name__ == "__main__":
    # Ensure script is run from project root or paths are relative to root
    # Switch to project root just in case
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(os.path.join(script_dir, ".."))
    
    if len(sys.argv) < 2:
        print("Usage: python add_indigo_baselines.py <results_dir>")
        sys.exit(1)
        
    results_dir = sys.argv[1]
    add_indigo_baseline(results_dir)
