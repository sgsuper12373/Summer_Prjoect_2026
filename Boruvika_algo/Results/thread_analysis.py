import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

# =====================================================================
# CONFIGURATION
# =====================================================================
COMPILED_DIR = Path('./compiled_results')
VIS_DIR = COMPILED_DIR / 'thread_analysis' 
TARGET_ALGOS = ['omp_half', 'omp_intermediate']

def generate_thread_visualizations():
    # 1. Setup output directory
    VIS_DIR.mkdir(parents=True, exist_ok=True)
    
    # 2. Read all compiled CSVs
    csv_files = list(COMPILED_DIR.glob('*_compiled.csv'))
    if not csv_files:
        print(f"No compiled CSVs found in {COMPILED_DIR}")
        return
        
    df_list = [pd.read_csv(f) for f in csv_files]
    df = pd.concat(df_list, ignore_index=True)
    
    # Handle column name formatting (handles leading spaces if present)
    time_col = ' total_time_s' if ' total_time_s' in df.columns else 'total_time_s'
    
    # 3. Identify unique graphs and chunk sizes
    graphs = df['graph'].unique()
    parallel_df = df[df['algorithm'].isin(TARGET_ALGOS)]
    chunk_sizes = sorted(parallel_df['chunk_size_p0'].dropna().unique())
    
    # 4. Generate a plot for each graph and chunk size combination
    for g in graphs:
        graph_df = df[df['graph'] == g]
        
        # Extract the serial baseline time for this specific graph (if it exists)
        serial_df = graph_df[graph_df['algorithm'] == 'serial_half']
        serial_time = serial_df[time_col].iloc[0] if not serial_df.empty else None
        
        for c in chunk_sizes:
            c = int(c)
            
            # Filter parallel data for this specific chunk size
            config_df = graph_df[(graph_df['chunk_size_p0'] == c) & (graph_df['algorithm'].isin(TARGET_ALGOS))]
            
            # Skip if there's no parallel data for this configuration
            if config_df.empty:
                continue
            
            fig, ax = plt.subplots(figsize=(10, 6))
            
            # Plot a line for each parallel algorithm
            for algo in TARGET_ALGOS:
                algo_df = config_df[config_df['algorithm'] == algo].sort_values('threads')
                if not algo_df.empty:
                    ax.plot(
                        algo_df['threads'], 
                        algo_df[time_col], 
                        marker='o', 
                        linewidth=2, 
                        markersize=6,
                        label=algo
                    )
            
            # Plot the serial baseline as a horizontal reference line
            if serial_time is not None:
                ax.axhline(
                    y=serial_time, 
                    color='red', 
                    linestyle='--', 
                    linewidth=1.5, 
                    label='serial_half (Baseline)'
                )
            
            # Formatting the chart
            ax.set_title(f'Thread Scaling vs Execution Time: {g}\n(Chunk Size: {c})', fontsize=14, pad=15)
            ax.set_xlabel('Number of Threads', fontsize=12)
            ax.set_ylabel('Execution Time (Seconds)', fontsize=12)
            
            # Ensure the X-axis only ticks at the tested thread counts
            threads_tested = sorted(config_df['threads'].unique())
            ax.set_xticks(threads_tested)
            
            # Legend and Grid
            ax.legend(fontsize=11)
            ax.grid(True, linestyle='--', alpha=0.7)
            
            plt.tight_layout()
            
            # Save figure matching the requested naming convention
            out_filename = VIS_DIR / f'thread_analysis_{g}_{c}.png'
            plt.savefig(out_filename, dpi=300, bbox_inches='tight')
            plt.close()
            
            print(f"Generated plot: {out_filename.name}")

if __name__ == '__main__':
    generate_thread_visualizations()