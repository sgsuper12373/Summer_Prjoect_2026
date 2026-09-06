# Reads the per-graph, median-compiled benchmark CSVs in compiled_results/ and
# converts total execution time into throughput (million edges per second).
# It generates two chart families:
#   1. graph_visualize/: algorithm-comparison charts for every
#      (thread-count, chunk-size) configuration.
#   2. threads_througput_graph_analysis/: one chart per algorithm and chunk
#      size, with graph datasets on the x-axis and thread counts as grouped
#      bars.
#   3. chunk_througput_analysis/: one chart per algorithm and selected thread
#      count, with graph datasets on the x-axis and chunk sizes as grouped
#      bars.
# Run this script from the Results/ directory: python3 visulize.py

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
    'as-skitter' : 22190596 ,
    'in-2004' : 27182946, 
    'r4-2e23.sym' : 67108846, 
    'soc-LiveJournal1' : 85702474, 
    'coPapersDBLP' : 30491458, 
    'europe_osm' : 108109320, 
    'delaunay_n24' : 100663202, 
    'amazon0601': 4886816, 
    'USA-road-d.NY': 730100, 
    'rmat22.sym': 65660814, 
    'internet': 387240,
    'rmat16.sym': 967866,
    'USA-road-d.USA': 57708624,

    
}

TARGET_ALGOS = ['serial_half', 'omp_half', 'omp_intermediate']
COMPILED_DIR = Path('./compiled_results')
VIS_DIR = COMPILED_DIR / 'graph_visualize'
THREADS_VIS_DIR = COMPILED_DIR / 'threads_througput_graph_analysis'
CHUNK_VIS_DIR = COMPILED_DIR / 'chunk_througput_analysis'
NUM_THREADS = [1, 2, 4, 8, 12, 16]

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


def generate_thread_throughput_visualizations():
    """Create grouped throughput charts for every algorithm and chunk size."""
    THREADS_VIS_DIR.mkdir(parents=True, exist_ok=True)

    csv_files = sorted(COMPILED_DIR.glob('*_compiled.csv'))
    if not csv_files:
        print(f"No compiled CSVs found in {COMPILED_DIR}")
        return

    # The compiled CSVs already contain the median of the benchmark runs, so
    # each row is used directly without another timing aggregation.
    df = pd.concat((pd.read_csv(file) for file in csv_files), ignore_index=True)
    time_col = ' total_time_s' if ' total_time_s' in df.columns else 'total_time_s'
    df['edges'] = df['graph'].map(GRAPH_EDGES)

    missing_graphs = sorted(df.loc[df['edges'].isna(), 'graph'].unique())
    if missing_graphs:
        raise ValueError(
            'Missing edge counts for graph(s): ' + ', '.join(missing_graphs)
        )

    df = df[df['algorithm'].isin(TARGET_ALGOS)].copy()
    df['throughput_Meps'] = df['edges'] / df[time_col] / 1_000_000

    chunk_sizes = sorted(df['chunk_size_p0'].unique())
    for algorithm in TARGET_ALGOS:
        algorithm_df = df[df['algorithm'] == algorithm]
        for chunk_size in chunk_sizes:
            # serial_half does not use chunk size and is benchmarked only at
            # chunk size 16. Reuse its same median baseline in each chunk-size
            # chart so every algorithm has a comparable set of three charts.
            chart_df = algorithm_df if algorithm == 'serial_half' else algorithm_df[
                algorithm_df['chunk_size_p0'] == chunk_size
            ]
            if chart_df.empty:
                print(f'No data for {algorithm} at chunk size {chunk_size}; skipping.')
                continue

            # Rows are graph datasets; grouped bars within a dataset are thread
            # counts. pivot() also catches accidental duplicate median rows.
            pivot_df = chart_df.pivot(
                index='graph', columns='threads', values='throughput_Meps'
            ).sort_index()
            pivot_df = pivot_df.reindex(sorted(pivot_df.columns), axis=1)

            ax = pivot_df.plot(kind='bar', figsize=(16, 7), width=0.8, zorder=3)
            chunk_note = ' (not used by serial)' if algorithm == 'serial_half' else ''
            ax.set_title(
                f'{algorithm} Throughput by Thread Count '
                f'(Chunk Size: {chunk_size}{chunk_note})',
                fontsize=14,
                pad=15,
            )
            ax.set_xlabel('Graph Dataset', fontsize=12)
            ax.set_ylabel('Throughput (Mega-edges/s)', fontsize=12)
            plt.setp(ax.get_xticklabels(), rotation=45, ha='right', fontsize=10)
            ax.legend(title='Threads', fontsize=10, title_fontsize=11)
            ax.grid(axis='y', linestyle='--', alpha=0.7, zorder=0)
            plt.tight_layout()

            out_file = THREADS_VIS_DIR / (
                f'{algorithm}_CH_{chunk_size}_thread_throughput.png'
            )
            plt.savefig(out_file, dpi=300)
            plt.close()
            print(f'Generated plot: {out_file}')


def generate_chunk_throughput_visualizations():
    """Create grouped chunk-size throughput charts for selected thread counts."""
    CHUNK_VIS_DIR.mkdir(parents=True, exist_ok=True)

    csv_files = sorted(COMPILED_DIR.glob('*_compiled.csv'))
    if not csv_files:
        print(f"No compiled CSVs found in {COMPILED_DIR}")
        return

    df = pd.concat((pd.read_csv(file) for file in csv_files), ignore_index=True)
    time_col = ' total_time_s' if ' total_time_s' in df.columns else 'total_time_s'
    df['edges'] = df['graph'].map(GRAPH_EDGES)

    missing_graphs = sorted(df.loc[df['edges'].isna(), 'graph'].unique())
    if missing_graphs:
        raise ValueError(
            'Missing edge counts for graph(s): ' + ', '.join(missing_graphs)
        )

    df = df[df['algorithm'].isin(TARGET_ALGOS)].copy()
    df['throughput_Meps'] = df['edges'] / df[time_col] / 1_000_000
    chunk_sizes = sorted(df['chunk_size_p0'].unique())

    for algorithm in TARGET_ALGOS:
        algorithm_df = df[df['algorithm'] == algorithm]
        for thread_count in NUM_THREADS:
            # serial_half runs on one thread and does not use chunk size. Its
            # median baseline is intentionally repeated for each requested
            # thread-count and chunk-size chart so every algorithm has the
            # same chart and bar set.
            if algorithm == 'serial_half':
                chart_df = pd.concat(
                    [algorithm_df.assign(chunk_size_p0=chunk_size)
                     for chunk_size in chunk_sizes],
                    ignore_index=True,
                )
            else:
                chart_df = algorithm_df[algorithm_df['threads'] == thread_count]
            if chart_df.empty:
                print(f'No data for {algorithm} at {thread_count} threads; skipping.')
                continue

            # Rows are graph datasets; grouped bars within a dataset are chunk
            # sizes. pivot() also catches accidental duplicate median rows.
            pivot_df = chart_df.pivot(
                index='graph', columns='chunk_size_p0', values='throughput_Meps'
            ).sort_index()
            pivot_df = pivot_df.reindex(sorted(pivot_df.columns), axis=1)

            ax = pivot_df.plot(kind='bar', figsize=(16, 7), width=0.8, zorder=3)
            thread_note = ' (not used by serial)' if algorithm == 'serial_half' else ''
            ax.set_title(
                f'{algorithm} Throughput by Chunk Size '
                f'(Threads: {thread_count}{thread_note})',
                fontsize=14,
                pad=15,
            )
            ax.set_xlabel('Graph Dataset', fontsize=12)
            ax.set_ylabel('Throughput (Mega-edges/s)', fontsize=12)
            plt.setp(ax.get_xticklabels(), rotation=45, ha='right', fontsize=10)
            ax.legend(title='Chunk Size', fontsize=10, title_fontsize=11)
            ax.grid(axis='y', linestyle='--', alpha=0.7, zorder=0)
            plt.tight_layout()

            out_file = CHUNK_VIS_DIR / (
                f'{algorithm}_t{thread_count}_chunk_throughput.png'
            )
            plt.savefig(out_file, dpi=300)
            plt.close()
            print(f'Generated plot: {out_file}')

if __name__ == '__main__':
    generate_visualizations()
    generate_thread_throughput_visualizations()
    generate_chunk_throughput_visualizations()
