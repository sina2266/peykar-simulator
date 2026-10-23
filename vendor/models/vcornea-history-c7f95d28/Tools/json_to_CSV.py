import json
import csv

# Load the JSON data
json_file_path = '/u/jvanin/vCornea/Processed_Data/Output_Version_6/9000/Filtered/parameters_data.json'  # Update this path as needed
with open(json_file_path, 'r') as json_file:
    data = json.load(json_file)

# Identify parameters that change across Combs
all_parameters = set()
varying_parameters = set()

# Initialize a dictionary to hold the first seen value of each parameter for comparison
first_seen_values = {}

for comb_data in data.values():
    for param, value in comb_data.items():
        if param == 'rep_folder':
            continue  # Skip 'rep_folder'
        if param not in all_parameters:
            all_parameters.add(param)
            first_seen_values[param] = value
        elif first_seen_values[param] != value:
            varying_parameters.add(param)

# Sort varying parameters for consistent column ordering
sorted_varying_parameters = sorted(varying_parameters)

# Write the CSV file
csv_file_path = '/u/jvanin/vCornea/Processed_Data/Output_Version_6/9000/Filtered/varying_parameters_data.csv'  # Update this path as needed
with open(csv_file_path, 'w', newline='') as csv_file:
    writer = csv.writer(csv_file)
    # Write the header row for varying parameters
    writer.writerow(sorted_varying_parameters)
    # Write each row for the Combs, including only varying parameters
    for parameters in data.values():
        row = [parameters.get(param, 'NA') for param in sorted_varying_parameters]
        writer.writerow(row)

print(f"CSV file '{csv_file_path}' has been created with only varying parameters.")
