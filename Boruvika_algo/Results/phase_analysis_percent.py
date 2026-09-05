import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

# =====================================================================
# CONFIGURATION
# =====================================================================
COMPILED_DIR = Path('./compiled_results')
VIS_DIR = COMPILED_DIR / 'phase_analysis'
TARGET_ALGOS = ['omp_half', 'omp_intermediate']
PHASES = ['phase0_s', 'phase1_s', 'phase2_s', 'phase3_s']
PHASE_LABELS = ['Phase 0', 'Phase 1', 'Phase 2', 'Phase 3']

# Colors for the 4 phases
PHASE_COLORS = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']

def generate_phase_visualizations():
    VIS_DIR.mkdir(parents=True, exist_ok=True)
    
    csv_files = list(COMPILED_DIR.glob('*_compiled.csv'))
    if not csv_files:
        print(f"No compiled CSVs found in {COMPILED_DIR}")
        return
        
    df_list = [pd.read_csv(f) for f in csv_files]
    df = pd.concat(df_list, ignore_index=True)
    df = df[df['algorithm'].isin(TARGET_ALGOS)]
    
    configs = df[['threads', 'chunk_size_p0']].drop_duplicates()
    
    for _, row in configs.iterrows():
        t = int(row['threads'])
        c = int(row['chunk_size_p0'])
        
        config_df = df[(df['threads'] == t) & (df['chunk_size_p0'] == c)]
        if config_df.empty:
            continue
            
        graphs = sorted(config_df['graph'].unique())
        x = np.arange(len(graphs))
        width = 0.35
        
        fig, ax = plt.subplots(figsize=(14, 7))
        
        added_labels = set()
        
        for i, algo in enumerate(TARGET_ALGOS):
            algo_df = config_df[config_df['algorithm'] == algo].set_index('graph')
            algo_df = algo_df.reindex(graphs).fillna(0)
            
            # --- Convert raw time to percentages ---
            total_time = algo_df[PHASES].sum(axis=1)
            total_time = total_time.replace(0, 1) 
            algo_df[PHASES] = algo_df[PHASES].div(total_time, axis=0) * 100
            
            offset = -width/2 if i == 0 else width/2
            bottom = np.zeros(len(graphs))
            hatch_pattern = '' if i == 0 else '......'
            
            for phase_idx, phase in enumerate(PHASES):
                label_name = PHASE_LABELS[phase_idx]
                
                if label_name not in added_labels:
                    label = label_name
                    added_labels.add(label_name)
                else:
                    label = ""
                
                # ax.bar returns a list of rectangles (the actual bars drawn)
                bars = ax.bar(
                    x + offset, 
                    algo_df[phase], 
                    width, 
                    bottom=bottom, 
                    label=label, 
                    color=PHASE_COLORS[phase_idx], 
                    edgecolor='white',
                    hatch=hatch_pattern
                )
                
                # --- NEW: Add the text labels inside the bars ---
                for bar in bars:
                    height = bar.get_height()
                    # Only print the text if the bar is tall enough (e.g., > 4%) to avoid clutter
                    if height > 4.0:
                        ax.text(
                            bar.get_x() + bar.get_width() / 2,  # X center of the bar
                            bar.get_y() + height / 2,           # Y center of the bar segment
                            f'{height:.1f}%',                   # The text (formatted to 1 decimal)
                            ha='center', 
                            va='center',
                            color='black',                      # Can change to 'white' if your phase colors are very dark
                            fontsize=5,
                            fontweight='bold',
                            # Add a subtle white outline to make text readable over hatches/colors
                            bbox=dict(facecolor='white', alpha=0.6, edgecolor='none', pad=1)
                        )

                bottom += algo_df[phase].values
                
        ax.set_title(f'Phase Execution Time Breakdown (%) (Threads: {t}, Chunk Size: {c})', fontsize=14, pad=15)
        ax.set_ylabel('Percentage of Execution Time (%)', fontsize=12)
        ax.set_xlabel('Graph Dataset', fontsize=12)
        
        ax.set_xticks(x)
        ax.set_xticklabels(graphs, rotation=45, ha='right', fontsize=10)
        ax.set_ylim(0, 100)
        
        from matplotlib.patches import Patch
        handles, labels = ax.get_legend_handles_labels()
        handles.append(Patch(facecolor='white', edgecolor='black', label=TARGET_ALGOS[0]))
        handles.append(Patch(facecolor='white', edgecolor='black', hatch='//', label=TARGET_ALGOS[1]))
        labels.extend([TARGET_ALGOS[0], TARGET_ALGOS[1]])
        
        ax.legend(handles, labels, loc='center left', bbox_to_anchor=(1, 0.5), fontsize=10)
        ax.grid(axis='y', linestyle='--', alpha=0.7, zorder=0)
        
        plt.tight_layout()
        
        out_filename = VIS_DIR / f'phase_analysis_pct_{t}_{c}.png'
        plt.savefig(out_filename, dpi=300, bbox_inches='tight')
        plt.close()
        
        print(f"Generated percentage phase plot with labels: {out_filename.name}")

if __name__ == '__main__':
    generate_phase_visualizations()