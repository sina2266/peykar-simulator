#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=8    
#SBATCH -J "py_API"
#SBATCH -o 'simulation_out'_%j.txt
#SBATCH -e 'err_file'_%j.err
#SBATCH --mail-user=jvanin@iu.edu
#SBATCH --time=24:00:00  # Set a max walltime to 24 hours
# Change to the directory where your script is located

cd /u/jvanin/T_vCornea_paper/Epithelium/Local/
# Run your Python script
python RunScript.py

SIM_PID=$!

# Wait for the simulation to complete by checking for a specific log message
while kill -0 $SIM_PID 2>/dev/null; do
    if grep -q "Simulation finished successfully" 'simulation_out'_${{SLURM_JOBID}}.txt; then
        echo "Simulation finished"
        kill $SIM_PID
        break
    fi
    sleep 60  # Check every 60 seconds
done

OUTPUT_FILE="'simulation_out'_${{SLURM_JOBID}}.txt"
ERROR_FILE="'err_file'_${{SLURM_JOBID}}.err"
mail -s "SLURM Job ${{SLURM_JOBID}} Results" jvanin@iu.edu < "$OUTPUT_FILE"
mail -s "SLURM Job ${{SLURM_JOBID}} Errors" jvanin@iu.edu < "$ERROR_FILE"  
    """