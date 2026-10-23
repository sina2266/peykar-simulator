import os
import json
from pathlib import Path
from os.path import join as Join

def extract_parameters(file_path):
    parameters = {}
    with open(file_path, 'r') as file:
        for line in file:
            if '=' in line:
                key, value = line.split('=', 1)
                parameters[key.strip()] = eval(value.strip())
    return parameters

def traverse_directories(root_path):
    comb_data = {}
    for path in Path(root_path).rglob('Parameters.py'):
        parameters = extract_parameters(path)
        comb_name = path.parts[-4]  # Assuming Parameters.py is always 3 levels deep from Comb#
        comb_data[comb_name] = parameters
    return comb_data

def main():
    root_path = '/u/jvanin/vCornea/Processed_Data/Output_Version_6/9000/Filtered'
    data = traverse_directories(root_path)
    with open(Join(root_path,'parameters_data.json'), 'w') as json_file:
        json.dump(data, json_file, indent=4)

if __name__ == "__main__":
    main()