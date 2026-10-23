import subprocess

def run_command(command):
    result = subprocess.run(command, shell=True, check=True, capture_output=True, text=True)
    print(result.stdout)
    print(result.stderr)

# Name of the environment
env_name = "vCornea"

# Create a new conda environment with Python 3.10
print(f"Creating environment {env_name}...")
run_command(f"conda create --name {env_name} python=3.10 -y")

# Activate the new environment
print(f"Activating environment {env_name}...")
run_command(f"conda activate {env_name}")

# Ensure mamba is installed
print("Installing mamba...")
run_command("conda install mamba -y")

# Install Compucell3D from the specified channels
print("Installing Compucell3D...")
run_command("mamba install -c conda-forge -c compucell3d compucell3d=4.6.0 -y")

# Install additional libraries from requirements.txt
with open("requirements.txt") as f:
    for requirement in f:        
        run_command(f"conda install --yes {requirement.strip()}")
        

print(f"Environment {env_name} setup complete.")