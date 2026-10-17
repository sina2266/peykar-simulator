# V-Cornea: A Computational Model of Corneal Epithelium Homeostasis, Injury, and Recovery

This repository contains the source code and simulation files for **V-Cornea**, an agent-based computational model of the corneal epithelium, as described in the manuscript:

> Vanin J, Getz M, Mahony C, Knudsen TB, Glazier JA (2025) V-Cornea: A computational model of corneal epithelium homeostasis, injury, and recovery. PLoS Comput Biol 21(12): e1013410. [https://doi.org/10.1371/journal.pcbi.1013410](https://doi.org/10.1371/journal.pcbi.1013410)

The model is implemented in CompuCell3D and simulates the dynamics of corneal epithelial tissue, including homeostasis, response to injury, and subsequent recovery processes.

---

## Model Description

V-Cornea is a 2D agent-based model that simulates a radial sagittal section of the corneal limbus and peripheral cornea. It captures the complex, emergent behaviors of the corneal epithelium by defining biologically-inspired rules for individual cells governing proliferation, differentiation, migration, and death.

The model explicitly represents key cellular and structural components:
* **Cell Types**: Limbal Epithelial Stem Cells (LESCs), Basal Cells, Wing Cells, and Superficial Cells.
* **Microenvironment**: The tear film (as a source of Epidermal Growth Factor - EGF), a simplified epithelial basement membrane (EpBM), and the stroma.
* **Signaling**: A reaction-diffusion system for EGF, a critical regulator of cell proliferation and wound healing.

By integrating these components, V-Cornea bridges the gap between cellular-level behaviors and tissue-level outcomes, providing a platform for in vitro to in vivo extrapolation (IVIVE).

---

## Features

* **Tissue Homeostasis**: Simulates the self-organization and long-term maintenance of a stable, stratified corneal epithelium, reproducing physiologically accurate cell turnover rates (7-14 days).
* **Injury and Recovery**: Models tissue response to different severities of injury (slight, mild, and moderate) based on the depth of cellular damage.
* **Predictive Healing**: Accurately predicts healing timeframes of 3-5 days for injuries confined to the epithelium.
* **Pathology Simulation**: Demonstrates emergent, dynamic structural disorganization that mimics recurrent corneal erosions (RCE) following moderate injuries that disrupt the basement membrane.
* **Extensible Framework**: Built with CompuCell3D, the model is modular and can be extended to include more complex biological mechanisms, such as immune response or basement membrane regeneration.

---

## Data and Code Availability

All model code, simulation files, analysis scripts, and data are openly available.

* **GitHub Repository (Development Version)**: The code in this repository is the most up-to-date version.
* **Zenodo Archive (Permanent Version)**: [https://doi.org/10.5281/zenodo.16764319](https://doi.org/10.5281/zenodo.16764319)
    * **Data.zip** (~6.8 GB): Contains all raw simulation outputs needed to reproduce the figures in the manuscript.
    * **vCornea.zip** (~229 MB): A snapshot of the full simulation code from this repository.

---

## Setup and Installation

### 1. Environment Setup with Conda

The most reliable way to set up the required environment is by using Conda. If you don't have it, please install [Anaconda](https://www.anaconda.com/products/distribution) or [Miniconda](https://docs.conda.io/en/latest/miniconda.html).

Once Conda is installed, open the Anaconda Prompt (on Windows) or your terminal (on macOS/Linux) and run the following commands step-by-step:

```bash
# 1. Create a new conda environment named 'v_cornea' with Python 3.12
conda create -n v_cornea python=3.12

# 2. Activate the new environment
conda activate v_cornea

# 3. Install Mamba for faster package installation (recommended)
conda install -c conda-forge mamba

# 4. Install CompuCell3D using the specified channels
mamba install -c main -c conda-forge -c compucell3d compucell3d=4.7.0
```

### 2. Install Additional Libraries

After installing CompuCell3D, you'll need a few more libraries for data analysis and plotting. While still in your `v_cornea` environment, run this command:

```bash
# Install libraries for plotting and handling Parquet files
mamba install -c conda-forge pandas fastparquet matplotlib seaborn scipy
```

### 3. Clone the Repository

Finally, clone this repository to your local machine:

```bash
git clone [https://github.com/VaninJoel/vCornea.git](https://github.com/VaninJoel/vCornea.git)
cd vCornea
```
You are now ready to run the simulations and generate the plots.

---

## Usage on a Local Machine

This project includes two primary simulation configurations located in `Local/Project/` that correspond to the experiments in the manuscript.

### Replicating the Manuscript Results

#### Scenario 1: Tissue Self-Organization (from Stem Cells)

This simulation demonstrates the self-organization of the stratified corneal tissue starting from only limbal epithelial stem cells (LESCs).

1.  Launch the CompuCell3D Player from your terminal (with the `v_cornea` environment active):
    ```bash
    python -m cc3d.player5
    ```
2.  In the Player, go to `File` > `Open` and navigate to the `Local/Project/paper_version_STEM_ONLY/` directory.
3.  Select the `vCornea_v2.cc3d` project file to load the simulation.
4.  Run the simulation to observe the tissue developing to a stable, homeostatic state.

#### Scenario 2: Injury Experiments

This simulation starts with a pre-organized, stable tissue, ready for injury experiments. You can replicate the different injury scenarios from the paper by modifying one parameter.

1.  Launch the CompuCell3D Player:
    ```bash
    python -m cc3d.player5
    ```
2.  Go to `File` > `Open` and navigate to the `Local/Project/paper_version/` directory.
3.  Select the `vCornea_v2.cc3d` project file.
4.  Before running, open the `Local/Project/paper_version/Simulation/Parameters.py` file in a text editor.
5.  Locate the `SLS_Concentration` variable and set its value according to the desired injury level:
    * **Slight Injury**: `SLS_Concentration = 750.0`
    * **Mild Injury**: `SLS_Concentration = 1500.0`
    * **Moderate Injury**: `SLS_Concentration = 2500.0`
6.  Save the `Parameters.py` file and run the simulation in the CompuCell3D Player.

### Exploring New Scenarios

You are encouraged to design your own *in silico* experiments. The `Parameters.py` file is the central control panel for the simulation. You can adjust variables such as `InjuryTime`, `InjuryType` (`True` for chemical, `False` for ablation), `SLS_Gaussian_pulse` (`True` for a localized droplet, `False` for a uniform coat), and various cell properties to explore different biological questions.

---

## Running on a High-Performance Computing (HPC) Cluster

For large-scale parameter sweeps or running many replicates, it is recommended to use the provided scripts for an HPC environment.

### Environment Setup

The Conda environment setup for HPC is the same as the local installation. Ensure you have the `v_cornea` environment available on your cluster.

### Parameter Sweeps

The script `HPC/Batch_Run_vCornea_Paper_version_STEM_ONLY_noVTK.py` is designed to automate running multiple simulations with different parameters.

1.  **Configure the Sweep**: Open the script and modify the `var_dict` dictionary. Each key corresponds to a parameter in the `Parameters.py` file, and the list of values will be iterated through. You can also set the number of `replicates` for each parameter combination.

    **Warning**: The total number of simulations is the product of the number of values for each parameter multiplied by the number of replicates. This can result in a very large number of jobs, so be sure to check the script's estimate before running.

2.  **Run the Script**: Execute the script from the terminal:
    ```bash
    python HPC/Batch_Run_vCornea_Paper_version_STEM_ONLY_noVTK.py
    ```
    The script will generate a directory for each simulation run, copy the necessary files, and create a `BatchCall.sh` script for job submission.

### Job Submission

The batch run script is configured to submit jobs to a **SLURM** workload manager. It will automatically create and submit a job for each simulation.

If your HPC cluster uses a different job scheduler (e.g., PBS, SGE), you will need to modify the `generate_sbatch_string` function in the batch run script to match your system's submission syntax. Please review this function and update the SLURM calls as necessary for your system.

HPC simulations run "headless" (without a graphical interface), but all output data is saved, ready for analysis with the `plot_manager.py` script.

---

## Reproducing Manuscript Figures

### Using Published Data (Recommended)

This is the simplest way to generate the figures exactly as they appear in the manuscript.

1.  **Download and Extract Data**: Get the data from the Zenodo archive: [https://doi.org/10.5281/zenodo.16764319](https://doi.org/10.5281/zenodo.16764319). Unzip the `Data.zip` file.
2.  **Navigate to Scripts**: Open a terminal and navigate to the plotting scripts directory:
    ```bash
    cd Data/Plot\ Scripts/
    ```
3.  **Run the Plot Manager**:
    ```bash
    python plot_manager.py
    ```
    This will generate all manuscript figures using the published data.

### Using Your Own Simulation Data

If you run new simulations and want to generate plots:

1.  **Run Simulations**: Follow the steps in the "Usage" section to generate your own output data.
2.  **Modify the Plot Manager**: Open the `Data/Plot Scripts/plot_manager.py` file.
3.  **Update Paths and Parameters**: Scroll to the bottom of the script (`if __name__ == '__main__':`). You will need to change the path variables (e.g., `slight_path`, `mild_path`) to point to your new output directories. You may also need to adjust the `PlotManager` constructor arguments:
    * `Data_DIR`: The path to your simulation output folder.
    * `Stable_time`: The MCS at which your simulation reached homeostasis (default is `500` in the local version).
    * `Max_time`: The maximum MCS you want to include in the plots.
    * `SimTime_mcs`: The total duration of your simulation run in MCS (default is `7700` in the local version).

### Visuals and Customization

* **Generating Images/Videos**: To generate images and videos from your local runs that match the manuscript, please follow the tutorial here: [https://www.youtube.com/watch?v=0ABZP6Vey1I](CompuCell3D: How to Take Screenshots Automatically).
* **Cell Colors**: To match the color scheme used in the manuscript, use the following color codes in the CompuCell3D Player lower left side panel:

| Type Name  | Color      | Hex Code |
|------------|------------|----------|
| STEM       | Rose       | #ff007f  |
| LIMB       | Pink       | #ffa7f6  |
| BASAL      | Peach      | #ffbe99  |
| WING       | Blue       | #0055ff  |
| SUPER      | Cyan       | #55ffff  |
| MEMB       | Magenta    | #ff00ff  |
| STROMA     | Heliotrope | #c681ff  |
| WALL       | White      | #ffffff  |
| TEAR       | Bright Green| #55ff00  |

---

## Citation

If you use this model or the associated code in your research, please cite both the manuscript and the Zenodo software archive.

### Manuscript

```bibtex
@article{Vanin2025VCornea,
  author = {Vanin, Joel and Getz, Michael and Mahony, Catherine and Knudsen, Thomas B. and Glazier, James A.},
  title = {{V-Cornea: A computational model of corneal epithelium homeostasis, injury, and recovery}},
  journal = {PLOS Computational Biology},
  year = {2025},
  volume = {21},
  number = {12},
  pages = {e1013410},
  doi = {10.1371/journal.pcbi.1013410},
  url = {[https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1013410](https://journals.plos.org/ploscompbiol/article?id=10.1371/journal.pcbi.1013410)}
}
```

### Software

```bibtex
@software{Vanin2025VCorneaCode,
  author = {Vanin, Joel},
  title = {{vCornea: A computational model of corneal epithelium homeostasis, injury, and recovery}},
  month = jun,
  year = 2025,
  publisher = {Zenodo},
  version = {v1.0.0},
  doi = {10.5281/zenodo.16764319},
  url = {[https://doi.org/10.5281/zenodo.16764319](https://doi.org/10.5281/zenodo.16764319)}
}
