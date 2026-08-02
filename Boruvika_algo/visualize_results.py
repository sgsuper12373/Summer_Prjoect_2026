import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import os

def generate_plots(results_dir):
    combined_results_path = os.path.join(results_dir, "combined_results.csv")
    combined_phases_path = os.path.join(results_dir, "combined_phases.csv")
    
    if not os.path.exists(combined_results_path):
        print(f"Error: {combined_results_path} not found.")
        return
        
    df_results = pd.read_csv(combined_results_path)
    
    # 1. Scaling Analysis (Execution Time vs Threads)
    # We fix chunk size to 16 for thread scaling analysis
    df_scaling = df_results[df_results['chunk_size_p0'] == 16]
    
    graphs = df_results['graph'].unique()
    
    for graph in graphs:
        plt.figure(figsize=(10, 6))
        df_graph = df_scaling[df_scaling['graph'] == graph]
        
        # Plot each algorithm
        sns.lineplot(data=df_graph, x='threads', y='serial_half_s', marker='o', label='serial_half')
        sns.lineplot(data=df_graph, x='threads', y='omp_half_s', marker='s', label='omp_half')
        sns.lineplot(data=df_graph, x='threads', y='omp_intermediate_s', marker='^', label='omp_intermediate')
        
        plt.title(f"Scaling Analysis for {graph} (chunk_size=16)")
        plt.xlabel("Number of Threads")
        plt.ylabel("Execution Time (seconds)")
        plt.grid(True)
        plt.legend()
        plt.savefig(os.path.join(results_dir, f"{graph}_scaling.png"))
        plt.close()
        
    # 2. Chunk Size Impact
    # We fix threads to maximum (16) for chunk size analysis
    df_chunk = df_results[df_results['threads'] == 16]
    
    for graph in graphs:
        plt.figure(figsize=(10, 6))
        df_graph = df_chunk[df_chunk['graph'] == graph]
        
        sns.lineplot(data=df_graph, x='chunk_size_p0', y='omp_half_s', marker='s', label='omp_half')
        sns.lineplot(data=df_graph, x='chunk_size_p0', y='omp_intermediate_s', marker='^', label='omp_intermediate')
        
        plt.title(f"Chunk Size Impact for {graph} (threads=16)")
        plt.xlabel("Chunk Size")
        plt.ylabel("Execution Time (seconds)")
        plt.grid(True)
        plt.xticks([12, 16, 20])
        plt.legend()
        plt.savefig(os.path.join(results_dir, f"{graph}_chunk_size.png"))
        plt.close()

    # 3. Phase Breakdown
    if os.path.exists(combined_phases_path):
        df_phases = pd.read_csv(combined_phases_path)
        
        # Filter to a representative configuration: threads=16, chunk_size=16
        df_p_rep = df_phases[(df_phases['threads'] == 16) & (df_phases['chunk_size_p0'] == 16)]
        
        for graph in graphs:
            df_g_phases = df_p_rep[df_p_rep['graph'] == graph]
            if df_g_phases.empty:
                continue
                
            df_g_phases.set_index('algorithm')[['phase0_s', 'phase1_s', 'phase2_s', 'phase3_s']].plot(
                kind='bar', stacked=True, figsize=(10, 6), colormap='viridis'
            )
            
            plt.title(f"Phase Breakdown for {graph} (threads=16, chunk=16)")
            plt.xlabel("Algorithm")
            plt.ylabel("Time (seconds)")
            plt.xticks(rotation=0)
            plt.legend(title="Phases", loc="upper left")
            plt.tight_layout()
            plt.savefig(os.path.join(results_dir, f"{graph}_phases.png"))
            plt.close()
            
    print("Visualizations generated successfully!")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python visualize_results.py <results_dir>")
        sys.exit(1)
        
    results_dir = sys.argv[1]
    generate_plots(results_dir)
