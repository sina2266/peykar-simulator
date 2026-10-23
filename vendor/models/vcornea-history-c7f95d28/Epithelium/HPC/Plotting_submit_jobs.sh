#!/bin/bash

#(CHANGE THIS TO YOUR PYTHON SCRIPT)
base_folder="/u/jvanin/vCornea/Processed_Data/Output_Version_6/Long_year"
num_jobs=100  # adjust as necessary Tatooine has 103 cores

# Find all Comb# folders
folders=( $(ls $base_folder | grep 'Comb') )
echo "Identified folders: ${folders[@]}"
# Count the number of folders and calculate the number of folders each job should process
total_folders=${#folders[@]}
folders_per_job=$(( (total_folders + num_jobs - 1) / num_jobs ))  # ceiling division
echo "Folders per job: $folders_per_job"
echo "Total folders: $total_folders"
# Submit jobs
for ((i=0; i<$num_jobs; i++)); do
    start_idx=$(( i * folders_per_job ))
    end_idx=$(( start_idx + folders_per_job - 1 ))

    # Extract the specific folders this job should process
    job_folders="${folders[@]:$start_idx:$folders_per_job}"
    echo "Job folders for job $i: $job_folders"
    sbatch --job-name=process_comb_data_$i <<EOL
#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH -J PlottingHPC
#SBATCH --mail-user=jvanin@iu.edu
#SBATCH --output=comb_data_out_$i.txt
#SBATCH --error=comb_data_err_$i.txt

OUTPUT_FILE="comb_data_out_${SLURM_JOBID}.txt"
ERROR_FILE="comb_data_err_${SLURM_JOBID}.err"

# Run the Python script (CHANGE THIS TO YOUR PYTHON SCRIPT)
python Plotting_Classifying_HPC_Parallel_Paper_version.py $job_folders

# After Python script runs, send email
TEMP_FILE=\$(mktemp)

# Append output and errors to the temporary file
echo "----- OUTPUT -----" >> \$TEMP_FILE
echo "Processing folders: $job_folders" >> \$TEMP_FILE
cat "\$OUTPUT_FILE" >> \$TEMP_FILE
echo -e "\n\n----- ERRORS -----" >> \$TEMP_FILE
cat "\$ERROR_FILE" >> \$TEMP_FILE

# Send the merged contents in one email
mail -s "SLURM Job \${SLURM_JOBID} Results and Errors" jvanin@iu.edu < "\$TEMP_FILE"

# Remove the temporary file
rm -f \$TEMP_FILE
EOL

done