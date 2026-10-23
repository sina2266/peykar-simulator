import os

def extract_parameters(content):
    """Extract parameters and their values from the file content."""
    parameters = {}
    lines = content.strip().split("\n")
    for line in lines:
        parts = line.split('=')
        if len(parts) == 2:
            param_name = parts[0].strip()
            param_value = parts[1].strip()
            try:
                param_value = eval(param_value)
            except:
                pass
            parameters[param_name] = param_value
    return parameters

def compare_multiple_files(file_paths):
    all_parameters = {}
    for file_path in file_paths:
        with open(file_path, 'r') as f:
            content = f.read()
        params = extract_parameters(content)
        label = params['rep_folder'].split('/')[-2]
        for param, value in params.items():
            if param not in all_parameters:
                all_parameters[param] = {}
            all_parameters[param][label] = value
    return all_parameters

def collect_parameter_files(directory):
    """Collect paths to Parameters.py files from Comb# directories."""
    parameter_files = []
    for root, dirs, files in os.walk(directory):
        for dir_name in dirs:
            if "Comb" in dir_name:
                param_file_path = os.path.join(root, dir_name, "0", "Simulation", "Parameters.py")
                if os.path.exists(param_file_path):
                    parameter_files.append(param_file_path)
    return parameter_files

if __name__ == "__main__":
    # Path to the directory with Comb# folders
    directory_path = r"C:\Users\joelv\OneDrive\Desktop\Success_9_24_2023"
    
    # Collecting all Parameters.py file paths
    file_paths = collect_parameter_files(directory_path)
    
    all_param_values = compare_multiple_files(file_paths)
    for param, values in all_param_values.items():
        print(f"Parameter: EGF_STEM_HalfMaxValue")
        for label, value in values.items():
            print(f"  {label}: {value}")
        print("="*50)
