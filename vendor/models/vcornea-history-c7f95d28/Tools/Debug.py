import os
import glob

def check_files(directory):
    mismatched_combinations = {}
    
    for root, dirs, files in os.walk(directory):
        for dir_name in dirs:
            if "Comb" in dir_name:
                for pattern_name in ['Creation_of_mass_*.csv', 'Destruction_of_mass_*.csv', 'cell_count_*.csv', 'growth_*.csv']:
                    pattern_path = os.path.join(root, dir_name, "0", "Simulation", pattern_name)
                    matched_files = glob.glob(pattern_path)
                    
                    # If not exactly one file is matched, flag it
                    if len(matched_files) != 1:
                        mismatched_combinations.setdefault(dir_name, []).append(pattern_name)

    return mismatched_combinations

mismatched = check_files('/u/jvanin/vCornea/Processed_Data/Output_Version_3/Output_v3_expl2')  # Replace with your directory path
print("Combinations with mismatched files:")
for comb, patterns in mismatched.items():
    print(f"{comb} has mismatches in: {', '.join(patterns)}")