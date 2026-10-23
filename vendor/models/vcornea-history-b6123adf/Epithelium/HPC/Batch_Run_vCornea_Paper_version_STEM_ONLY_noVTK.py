from pathlib import Path
import os
import glob
import shutil
import subprocess
import datetime
import sys
import time
import pandas as pd

MAX_JOBS = 9999
WAIT_TIME = 60  # seconds

base_path = '/u/jvanin/'
sim_version_path = '/u/jvanin/T_vCornea_paper/Epithelium/HPC/Project/paper_version_STEM_ONLY'
output_version_path = '/u/jvanin/T_vCornea_paper/Epithelium/HPC/Simulations_Output'
run_name = 'STEM_ONLY_proper_thickness_6months'

csv_file_param = None #None  #TODO Provide file with LHS if desired or None


#||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
#                                                               |
#---- Time Scales ---
HOURtoMCS   = 10
DAYtoMCS    = 240
WEEKtoMCS   = 1680
MONTHtoMCS  = 7300
YEARtoMCS   = 87600

replicates = 35

var_dict = {
#---STEM---
    'InitSTEM_LambdaSurface'    : [2.0],
    'InitSTEM_TargetSurface'    : [18.0],

    'InitSTEM_LambdaVolume'     : [2.0],
    'InitSTEM_TargetVolume'     : [40.0],

    'DensitySTEM_HalfMaxValue'  : [125],   
    'EGF_STEM_HalfMaxValue'     : [4.5], 
    'STEM_beta_EGF'             : [1],  
    'InitSTEM_LambdaChemo'      : [100.0],
# Growth Scalars 
    'EGF_GrowthScalar_STEM'     : [1.0],
    'DensityGrowthScalar_STEM'  : [1.0],

    'SLS_STEMDiffCoef'          : [5.0],
#---BASAL---
    'InitBASAL_LambdaSurface'   : [2.0],
    'InitBASAL_TargetSurface'   : [20.0],

    'InitBASAL_LambdaVolume'    : [2.0],
    'InitBASAL_TargetVolume'    : [40.0],

    'InitBASAL_LambdaChemo'     : [1000.0],
    'InitBASAL_Division'        : [20000.0],

    'DensityBASAL_HalfMaxValue' : [125], 
    'EGF_BASAL_HalfMaxValue'    : [8.0], 
    'BASAL_beta_EGF'            : [1],      
# Growth Scalars 
    'EGF_GrowthScalar_BASAL'    : [1.0],
    'DensityGrowthScalar_BASAL' : [1.0],

    'SLS_BASALDiffCoef'         : [5.0],
#---WING---
    'InitWING_LambdaSurface'    : [5.0],
    'InitWING_TargetSurface'    : [25],

    'InitWING_LambdaVolume'     : [2.0],
    'InitWING_TargetVolume'     : [25.0],

    'InitWING_EGFLambdaChemo'   : [20.0],
    'SLS_WINGDiffCoef'          : [5.0],
#---SUPERFICIAL---
    'InitSUPER_LambdaSurface'   : [5.0],
    'InitSUPER_TargetSurface'   : [25.0],

    'InitSUPER_LambdaVolume'    : [5.0],
    'InitSUPER_TargetVolume'    : [25.0],
    'EGF_SUPERDiffCoef'         : [10.0], 
    'SLS_SUPERDiffCoef'         : [5.0],

    'SloughProbability'         : [0.0],
# Death Scalars
    'DeathTimeScalar'           : [1],
    'DeathVolumeScalar'         : [1],
    'SloughScalar'              : [1],

    'SLS_MEMBDiffCoef'          : [5.0],
    'SLS_LIMBDiffCoef'          : [5.0],
    'SLS_TEARDiffCoef'          : [5.0],

# FIELDS
    'MovementBiasScreteAmount'  : [1],
    'MovementBiasUptake'        : [1],

    'EGF_ScreteAmount'          : [1],

    'EGF_FieldUptakeBASAL'      : [0.0],
    'EGF_FieldUptakeSTEM'       : [0.0],
    'EGF_FieldUptakeSuper'      : [0.0],
    'EGF_FieldUptakeWing'       : [0.0],
    'EGF_GlobalDecay'           : [0.5],

# WOUND
    'InjuryType'                : [False],
    'IsInjury'                  : [False],
    'InjuryTime'                : [4000000000],
    'SLS_Injury'                : [False],
    'SLS_Threshold_Method'      : [True],    
#---INJURY AREA---
    'InjuryX_Center'            : [150],
    'InjuryY_Center'            : [60],
    'InjuryRadius'              : [25],
#---SLS AREA---
    'SLS_X_Center'              : [100],
    'SLS_Y_Center'              : [75],
    'SLS_Concentration'         : [1700.0],
    'SLS_Gaussian_pulse'        : [True],
#Links
    'LINKWALL_lambda_distance'  : [50],
    'LINKWALL_target_distance'  : [3],
    'LINKSUPER_lambda_distance' : [50],
    'LINKSUPER_target_distance' : [3],
    'LINKWALL_max_distance'     : [1000],
    'LINKSUPER_max_distance'    : [1000],
    'L_max_links_SS'            : [5],
    'F_max_links_SS'            : [15],
    'AutoAdjustLinks'           : [True],
# DEBUGGING
    'GrowthControl'             : [True],
    'MitosisControl'            : [True],
    'DeathControl'              : [True],
    'DifferentiationControl'    : [True],
#   PLOTS
    'CellCount'                 : [True],
    'XHypTracker'               : [False],
    'SloughTracker'             : [False],
    'PressureTracker'           : [True],
    'PressurePlot'              : [False],
    'VolumeTracker'             : [False],
    'EGF_SeenByCell'            : [True],
    'SLS_SeenByCell'            : [False],
    'CenterBiasPlot'            : [False],
    'CenterBias'                : [False],
    'DivisionTracker'           : [True],
    'GrowthPlot'                : [False],
    'ThicknessPlot'             : [True],
    'VolumeSurfaceDetail'       : [False],  
    'MitosisPlot'               : [True],   
    'SingleCellPresEGFPlot'     : [True],
    'MassConservationPlot'      : [True],
    'SurfactantTracking'        : [False],
    'SnapShot'                  : [False],

# TIME OF SIMULATION
    'SimTime'                   : [7200*6],
}


#                                                               |
#||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

def estimate_average_time(sim_time, num_combinations, num_rep, mcs=2010, seconds=900):
    # 14600 MCS took 2736 seconds.
    # Estimate average time for one combination
    avg_time_per_combination = (sim_time / mcs) * seconds
    # Calculate total time for all combinations and reps
    total_time = avg_time_per_combination * num_combinations * num_rep
    return total_time

def estimate_time_at_capacity(total_time, capacity_percentage, jobs_at_full_capacity=100):
    adjusted_jobs = (capacity_percentage / 100) * jobs_at_full_capacity
    return total_time / adjusted_jobs if adjusted_jobs != 0 else float('inf')

def calculate_total_combinations(dict):
    total_combinations = 1
    for key, values in dict.items():
        total_combinations *= len(values)
    return total_combinations


def generate_dicts_from_csv(csv_file_path):
    """
    Generate a list of dictionaries with parameter-value mappings from a CSV file.
    
    Args:
    - csv_file_path (str): Path to the CSV file containing the parameter combinations.
    
    Returns:
    - List of dictionaries where each dictionary represents a unique combination of parameter-value pairs.
    """
    # Read the CSV file
    df = pd.read_csv(csv_file_path)
    
    # Convert the DataFrame to a list of dictionaries
    combinations_dicts = df.to_dict(orient='records')
    
    return combinations_dicts

def _original_generate_lists(data, keys=None, values=None):
    """Original recursive logic to generate combinations from the provided dictionary."""
    if keys is None:
        keys = list(data.keys())
    if values is None:
        values = []

    if keys:
        current_key = keys.pop(0)
        current_values = data[current_key]
        new_values = []
        if values:
            for value in values:
                for current_value in current_values:
                    if isinstance(value, list):
                        new_values.append(value + [current_value])
                    else:
                        new_values.append([value, current_value])
        else:
            new_values = [[value] for value in current_values]
        return _original_generate_lists(data, keys, new_values)
    else:
        return values
    
def generate_lists(data, csv_file_path=None):
    """
    Generate a list of unique combinations from a dictionary or CSV file.
    
    Args:
    - data (dict): Dictionary containing parameter-value pairs.
    - csv_file_path (str, optional): Path to the CSV file containing parameter combinations.
    
    Returns:
    - List of lists where each inner list represents a unique combination of parameter values.
    """
    
    if csv_file_path:
        # Get the combinations as dictionaries
        combinations_dicts = generate_dicts_from_csv(csv_file_path)
        
        # Convert dictionaries to lists while preserving all keys from the original var_dict
        combinations = []
        for comb_dict in combinations_dicts:
            comb = []
            for key in data.keys():
                if key in comb_dict:
                    comb.append(comb_dict[key])
                else:
                    comb.append(data[key][0])
            combinations.append(comb)
    else:
        combinations = _original_generate_lists(data)
    
    return combinations

def generate_sbatch_string(cc3d_file, node=8, job_name='vCornea', out_file='simulation_out',
                            err_file='err_file', VTK_frames=10):
    sbatch_string = f"""#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node={node}    
#SBATCH -J {job_name}
#SBATCH -o {out_file}_%j.txt
#SBATCH -e {err_file}_%j.err
#SBATCH --mail-user=jvanin@iu.edu
#SBATCH --time=24:00:00  # Set a max walltime to 24 hours

# Function to clean up background processes
cleanup() {{
    echo "Cleaning up..."
    kill $XVFB_PID
}}
trap cleanup EXIT

# Xvfb :99 -screen 0 1024x768x16 &
# XVFB_PID=$!
# export DISPLAY=:99
# echo $DISPLAY

# Run the simulation in the background
# xvfb-run -a 
srun {base_path}.conda/envs/vCornea/bin/cc3d_runScript.sh -i {cc3d_file}

SIM_PID=$!

# Wait for the simulation to complete by checking for a specific log message
while kill -0 $SIM_PID 2>/dev/null; do
    if grep -q "Simulation finished successfully" {out_file}_${{SLURM_JOBID}}.txt; then
        echo "Simulation finished"
        kill $SIM_PID
        break
    fi
    sleep 60  # Check every 60 seconds
done

# Wait for background processes
wait $XVFB_PID

OUTPUT_FILE="{out_file}_${{SLURM_JOBID}}.txt"
ERROR_FILE="{err_file}_${{SLURM_JOBID}}.err"
mail -s "SLURM Job ${{SLURM_JOBID}} Results" jvanin@iu.edu < "$OUTPUT_FILE"
mail -s "SLURM Job ${{SLURM_JOBID}} Errors" jvanin@iu.edu < "$ERROR_FILE"  
    """
    return sbatch_string

def get_total_slurm_job_count():
    result = subprocess.run('squeue | wc -l', shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output = result.stdout.decode().strip()
    try:
        return int(output) - 1  # Subtract one for the header line
    except ValueError:
        print("Error parsing job count: ", output)
        return 0
# Model inputs to be changed must be in a dictionary format with the same 
# name as the created BatchTest.py file

mult_dict = var_dict

# number of replicates
num_rep = replicates

# Model output frequency
model_out_freq = 1

# Output frequency of simulation data per simulation replica
out_freq = 120 # VTK frequence in MCS

# Define base output directory
base_output_folder = output_version_path 

# Generate a timestamp
timestamp = datetime.datetime.now().strftime("%m%d%Y_%H%M")

# Append timestamp to base directory
sweep_output_folder = os.path.join(base_output_folder, run_name + f'_{timestamp}') 

simulation_folder = Path(sim_version_path) 

files_toCopy = glob.glob(os.path.join(simulation_folder, "*.cc3d"))
files_toCopy.extend(glob.glob(os.path.join(simulation_folder, "*.piff")))
files_toCopy.extend(glob.glob(os.path.join(simulation_folder,'Simulation', "*.py")))
files_toCopy.extend(glob.glob(os.path.join(simulation_folder,'Simulation', "*.xml")))
files_toCopy.extend(glob.glob(os.path.join(simulation_folder,'screenshot_data', "*.json")))

output = generate_lists(var_dict, csv_file_path=csv_file_param) 

# Calculate the total combinations
num_combinations = len(output)

total_time_seconds = estimate_average_time(var_dict['SimTime'][0], num_combinations, replicates)

days, remainder = divmod(total_time_seconds, 86400)  # 86400 seconds in a day
hours, remainder = divmod(remainder, 3600)
minutes, seconds = divmod(remainder, 60)
print('')
print(f"Total combinations possible: {num_combinations}")
print(f"Estimated time for completion if run sequentially: {days} days, {hours} hours, {minutes} minutes, and {seconds} seconds.")

capacities = [100, 75, 50, 25]
for cap in capacities:
    estimated_time = estimate_time_at_capacity(total_time_seconds, cap)
    e_days, e_remainder = divmod(estimated_time, 86400)
    e_hours, e_remainder = divmod(e_remainder, 3600)
    e_minutes, e_seconds = divmod(e_remainder, 60)
    print(f"At {cap}% capacity (approx. {cap/100*103:.0f} jobs in parallel), estimated time: {e_days} days, {e_hours} hours, {e_minutes} minutes, and {e_seconds:.2f} seconds.")

# Ask for user confirmation
confirmation = input("Do you want to proceed? (yes/no): ")

if confirmation.lower() != "yes":
    sys.exit("Operation aborted by user.")

# Ensure directory exists
if not os.path.isdir(sweep_output_folder):
    Path(sweep_output_folder).mkdir(parents=True, exist_ok=True)

# Create files and parameters 
for idx, comb in enumerate(output):        

        for ii in range(num_rep):

            rep_folder = os.path.join(sweep_output_folder, f'Comb{idx}',str(ii))
            Path(rep_folder).mkdir(parents=True, exist_ok=True)

            rep_folder_simulation = os.path.join(rep_folder, "Simulation")
            Path(rep_folder_simulation).mkdir(parents=True, exist_ok=True)
            

            for file in files_toCopy:

                if "Simulation" in file:
                    shutil.copy2(file,rep_folder_simulation)

                elif "screenshot_data" in file:
                    rep_folder_screenshot_data = os.path.join(rep_folder, "screenshot_data")
                    Path(rep_folder_screenshot_data).mkdir(parents=True, exist_ok=True)

                    shutil.copy2(file, rep_folder_screenshot_data)

                else:
                    shutil.copy2(file, rep_folder)

            cc3d_file = glob.glob(os.path.join(rep_folder, "*.cc3d"))[0]

            with open(os.path.join(rep_folder, "BatchCall.sh"),"w") as f:

                f.write(generate_sbatch_string(cc3d_file, node=1,
                                                job_name='simulation',
                                                  out_file='simulation_out',
                                                    err_file='err_file'))

            with open(os.path.join(rep_folder_simulation, "Parameters.py"),"w") as f:
                f.write(f'rep_folder = "{rep_folder}"\n')
                for val_idx, key in enumerate(var_dict.keys()):
                   f.write(f'{key}={comb[val_idx]}\n')

#  Call batch	
files_toCall = glob.glob(os.path.join(sweep_output_folder,"**", "BatchCall.sh"),recursive=True)
print(files_toCall)
processes = []

for file in files_toCall:
    while get_total_slurm_job_count() >= MAX_JOBS:
        print(f"Waiting for available slots in queue... Current job count: {get_total_slurm_job_count()}, Max: {MAX_JOBS}")
        time.sleep(WAIT_TIME)
    
    os.chmod(file, 0o777)
    directory = os.path.dirname(file)
    p = subprocess.Popen(['sbatch', file], cwd=directory)
    processes.append(p)
    
