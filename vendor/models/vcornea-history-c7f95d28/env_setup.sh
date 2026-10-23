#!/bin/bash

# Source the conda.sh file
source C:/Users/joelv/Anaconda3/etc/profile.d/conda.sh

# Name of the environment
ENV_NAME="vCornea"

# Ensure mamba is installed
conda install -c conda-forge mamba -y

# Create a new conda environment with Python 3.10
conda create --name $ENV_NAME python=3.10 -y

# Activate the new environment
conda activate $ENV_NAME

# Install Compucell3D from the specified channels
mamba install -c conda-forge -c compucell3d compucell3d=4.6.0 -y

# Install additional libraries from requirements.txt
while read requirement; do conda install --yes $requirement; done < requirements.txt