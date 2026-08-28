import os
import sys
import pandas as pd
from pathlib import Path

def compile_results(input_dir):
    input_path = Path(input_dir)
    output_dir = Path('./compiled_results')
    
    # Create the output directory if it doesn't exist
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Find all CSV files in the input directory
    csv_files = list(input_path.glob('*.csv'))
    
    if not csv_files:
        print(f"No CSV files found in '{input_dir}'.")
        return

    # The columns that define a unique configuration
    grouping_columns = ['algorithm', 'threads', 'chunk_size_p0']

    for file_path in csv_files:
        try:
            df = pd.read_csv(file_path)
            
            # Check if the necessary columns exist in the file
            missing_cols = [col for col in grouping_columns if col not in df.columns]
            if missing_cols:
                print(f"Skipping {file_path.name}: Missing grouping columns {missing_cols}")
                continue
            
            # Group by configuration and calculate the median
            # numeric_only=True ensures we don't try to calculate medians on string columns (like graph names)
            compiled_df = df.groupby(grouping_columns).median(numeric_only=True).reset_index()
            
            # If the 'graph' column was dropped because it's a string, let's add it back 
            # assuming all rows in a single file belong to the same graph
            if 'graph' in df.columns and 'graph' not in compiled_df.columns:
                compiled_df.insert(0, 'graph', df['graph'].iloc[0])

            # Construct the output file name and path
            output_filename = f"{file_path.stem}_compiled.csv"
            output_filepath = output_dir / output_filename
            
            # Write the compiled data to the new CSV
            compiled_df.to_csv(output_filepath, index=False)
            print(f"Successfully processed {file_path.name} -> {output_filename}")
            
        except Exception as e:
            print(f"Error processing {file_path.name}: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python compile_results.py <input_directory_containing_csvs>")
        sys.exit(1)
        
    input_directory = sys.argv[1]
    compile_results(input_directory)