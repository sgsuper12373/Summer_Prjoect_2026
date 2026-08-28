import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

# =====================================================================
# CONFIGURATION
# =====================================================================
# You MUST fill in the actual number of edges for each graph here 
# to accurately calculate the edges/second throughput.
GRAPH_EDGES = {
    '2d-2e20.sym' : 4190208, 
    'cit-Patents' : 33037894, 
    'citationCiteseer' : 2313294, 
    's-skitter' : 22190596 , 
    'in-2004' : 27182946, 
    'r4-2e23.sym' : 67108846, 
    'soc-LiveJournal1' : 85702474, 
    'coPapersDBLP' : 30491458, 
    'europe_osm' : 108109320, 
    'delaunay_n24' : 100663202, 
    'amazon0601': 4886816, 
    'USA-road-d.NY': 730100, 
    'rmat22.sym': 65660814, 
    'internet': 387240

    
}

TARGET_ALGOS = ['serial_half', 'omp_half', 'omp_intermediate']
COMPILED_DIR = Path('./compiled_results')
VIS_DIR = COMPILED_DIR / 'graph_visualize'

def generate_visualizations():
    # 1. Setup output directory
    VIS_DIR.mkdir(parents=True, exist_ok=True)
    
    # 2. Read all compiled CSVs into a single DataFrame
    csv_files = list(COMPILED_DIR.glob('*_compiled.csv'))
    if not csv_files:
        print(f"No compiled CSVs found in {COMPILED_DIR}")
        return
        
    df_list = [pd.read_csv(f) for f in csv_files]
    df = pd.concat(df_list, ignore_index=True)
    
    # 3. Handle column name formatting (handles leading spaces if present)
    time_col = ' total_time_s' if ' total_time_s' in df.columns else 'total_time_s'
    
    # 4. Map edges and calculate throughput
    df['edges'] = df['graph'].map(GRAPH_EDGES)
    
    missing_graphs = df[df['edges'].isna()]['graph'].unique()
    if len(missing_graphs) > 0:
        print(f"WARNING: Missing edge counts for graphs: {missing_graphs}")
        print("Throughput will be NaN for these. Please update GRAPH_EDGES in the script.")
    
    # Calculate Throughput (Mega-edges per second)
    df['throughput_Meps'] = (df['edges'] / df[time_col]) / 1_000_000
    
    # 5. Filter for the requested algorithms
    df = df[df['algorithm'].isin(TARGET_ALGOS)]
    
    # 6. Separate Serial and Parallel data
    # Serial data acts as a universal baseline for all plots
    serial_df = df[df['algorithm'] == 'serial_half'].copy()
    parallel_df = df[df['algorithm'] != 'serial_half'].copy()
    
    # Get all unique (threads, chunk_size) configurations from parallel runs
    configs = parallel_df[['threads', 'chunk_size_p0']].drop_duplicates()
    
    # 7. Generate a plot for each configuration
    for _, row in configs.iterrows():
        t = int(row['threads'])
        c = int(row['chunk_size_p0'])
        
        # Get parallel data for this specific configuration
        config_par_df = parallel_df[(parallel_df['threads'] == t) & (parallel_df['chunk_size_p0'] == c)]
        
        # Combine parallel data with the universal serial baseline
        config_df = pd.concat([config_par_df, serial_df], ignore_index=True)
        
        # Pivot the dataframe so graphs are rows (X-axis) and algorithms are columns (Bars)
        pivot_df = config_df.pivot_table(
            index='graph', 
            columns='algorithm', 
            values='throughput_Meps',
            aggfunc='mean'
        )
        
        # Reorder columns to ensure consistent order in the bar chart
        cols = [alg for alg in TARGET_ALGOS if alg in pivot_df.columns]
        pivot_df = pivot_df[cols]
        
        # 8. Plotting
        # Pandas plot wrapper automatically creates grouped bar charts
        ax = pivot_df.plot(kind='bar', figsize=(14, 6), width=0.75, zorder=3)
        
        plt.title(f'Algorithm Throughput Comparison (Threads: {t}, Chunk Size: {c})', fontsize=14, pad=15)
        plt.xlabel('Graph Dataset', fontsize=12)
        plt.ylabel('Mega-edges/s', fontsize=12)
        
        # Formatting X-axis labels
        plt.xticks(rotation=45, ha='right', fontsize=10)
        
        # Legend and Grid
        plt.legend(title='Algorithm', fontsize=10, title_fontsize=11)
        plt.grid(axis='y', linestyle='--', alpha=0.7, zorder=0)
        plt.tight_layout()
        
        # 9. Save figure
        out_filename = VIS_DIR / f'combined_results_{t}_{c}.png'
        plt.savefig(out_filename, dpi=300)
        plt.close()
        
        print(f"Generated plot: {out_filename.name}")

if __name__ == '__main__':
    generate_visualizations()