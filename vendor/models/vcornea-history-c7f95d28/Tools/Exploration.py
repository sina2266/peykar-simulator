
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# Reload the new dataset
growth_data_new = pd.read_csv(r"C:\Users\joelv\OneDrive\Desktop\vCornea_v_PaperHPC_version_2\Simulation\growth_14001.csv")

# Calculate the correlation coefficients for the new dataset
correlation_new_basal_egf = growth_data_new["Basal Average Total"].corr(growth_data_new["Basal Average EGFseen"])
correlation_new_basal_pressure = growth_data_new["Basal Average Total"].corr(growth_data_new["Basal Average Pressure"])
correlation_new_stem_egf = growth_data_new["Stem Average Total"].corr(growth_data_new["Stem Average EGFseen"])
correlation_new_stem_pressure = growth_data_new["Stem Average Total"].corr(growth_data_new["Stem Average Pressure"])

correlation_values_new = {
    "New Dataset": {
        "Basal_EGF": correlation_new_basal_egf,
        "Basal_Pressure": correlation_new_basal_pressure,
        "Stem_EGF": correlation_new_stem_egf,
        "Stem_Pressure": correlation_new_stem_pressure
    }
}

# Visualization of the trends
fig, ax = plt.subplots(2, 2, figsize=(15, 12))

# Basal cells: Average Total vs. EGFseen
ax[0, 0].scatter(growth_data_new["Basal Average EGFseen"], growth_data_new["Basal Average Total"], alpha=0.6, color="blue")
ax[0, 0].set_title("Basal Average Total vs. EGFseen")
ax[0, 0].set_xlabel("Basal Average EGFseen")
ax[0, 0].set_ylabel("Basal Average Total")

# Basal cells: Average Total vs. Pressure
ax[0, 1].scatter(growth_data_new["Basal Average Pressure"], growth_data_new["Basal Average Total"], alpha=0.6, color="blue")
ax[0, 1].set_title("Basal Average Total vs. Pressure")
ax[0, 1].set_xlabel("Basal Average Pressure")
ax[0, 1].set_ylabel("Basal Average Total")

# Stem cells: Average Total vs. EGFseen
ax[1, 0].scatter(growth_data_new["Stem Average EGFseen"], growth_data_new["Stem Average Total"], alpha=0.6, color="green")
ax[1, 0].set_title("Stem Average Total vs. EGFseen")
ax[1, 0].set_xlabel("Stem Average EGFseen")
ax[1, 0].set_ylabel("Stem Average Total")

# Stem cells: Average Total vs. Pressure
ax[1, 1].scatter(growth_data_new["Stem Average Pressure"], growth_data_new["Stem Average Total"], alpha=0.6, color="green")
ax[1, 1].set_title("Stem Average Total vs. Pressure")
ax[1, 1].set_xlabel("Stem Average Pressure")
ax[1, 1].set_ylabel("Stem Average Total")

plt.tight_layout()
plt.show()

# Calculate the gradient for EGFseen vs. Average Total for Basal cells
gradient_basal_egf = np.gradient(growth_data_new["Basal Average Total"], growth_data_new["Basal Average EGFseen"])

# Calculate the gradient for Pressure vs. Average Total for Basal cells
gradient_basal_pressure = np.gradient(growth_data_new["Basal Average Total"], growth_data_new["Basal Average Pressure"])

# Identify potential points of instability based on large gradient values
instability_threshold = np.percentile(np.abs(gradient_basal_egf), 95)  # Using the 95th percentile as a threshold
instability_indices_egf = np.where(np.abs(gradient_basal_egf) > instability_threshold)[0]

instability_threshold = np.percentile(np.abs(gradient_basal_pressure), 95)
instability_indices_pressure = np.where(np.abs(gradient_basal_pressure) > instability_threshold)[0]

# Extract the corresponding values of EGFseen and Pressure at these points
instability_values_egf = growth_data_new.iloc[instability_indices_egf][["Time", "Basal Average EGFseen", "Basal Average Total"]]
instability_values_pressure = growth_data_new.iloc[instability_indices_pressure][["Time", "Basal Average Pressure", "Basal Average Total"]]

# instability_values_egf, instability_values_pressure


# Define the functions based on the provided governing equations
def EGF_Growth(EGF, EGF_BASAL_HalfMaxValue):
    return (EGF**4 / (EGF_BASAL_HalfMaxValue**4 + EGF**4))

def DensityGrowth(Pressure, DensityBASAL_HalfMaxValue):
    return (DensityBASAL_HalfMaxValue**4 / (DensityBASAL_HalfMaxValue**4 + Pressure**4))

def TotalGrowth(EGF, Pressure, EGF_BASAL_HalfMaxValue, DensityBASAL_HalfMaxValue, BASAL_doubling):
    egf_growth = EGF_Growth(EGF, EGF_BASAL_HalfMaxValue)
    density_growth = DensityGrowth(Pressure, DensityBASAL_HalfMaxValue)
    return (BASAL_doubling * (density_growth * egf_growth)**2 / (1**2 + (density_growth * egf_growth)**2))

# Generate a grid of EGFseen and Pressure values
EGF_range = np.linspace(growth_data_new["Basal Average EGFseen"].min(), growth_data_new["Basal Average EGFseen"].max(), 20)
Pressure_range = np.linspace(growth_data_new["Basal Average Pressure"].min(), growth_data_new["Basal Average Pressure"].max(), 20)
EGF_grid, Pressure_grid = np.meshgrid(EGF_range, Pressure_range)

# Calculate the total growth rate for each combination of EGFseen and Pressure using the default values
EGF_BASAL_HalfMaxValue_default = 2.0
DensityBASAL_HalfMaxValue_default = 90.0
BASAL_doubling_default = 1  # Assuming a default value, as it wasn't provided
Growth_values = TotalGrowth(EGF_grid, Pressure_grid, EGF_BASAL_HalfMaxValue_default, DensityBASAL_HalfMaxValue_default, BASAL_doubling_default)

# Calculate the gradients (vector field)
dEGF, dPressure = np.gradient(Growth_values)

# Calculate the magnitude of the vectors
vector_magnitude = np.sqrt(dEGF**2 + dPressure**2)

# Define a threshold for vector magnitude below which we consider the region to be potentially stable
stability_threshold = np.percentile(vector_magnitude, 25)  # Using the 25th percentile as a threshold

# Identify regions of stability
stable_regions = np.where(vector_magnitude < stability_threshold)

# Extract EGFseen and Pressure values from these regions
stable_EGF_values = EGF_grid[stable_regions]
stable_Pressure_values = Pressure_grid[stable_regions]

# Determine the range for EGFseen and Pressure based on stable regions
EGF_stable_range = (np.min(stable_EGF_values), np.max(stable_EGF_values))
Pressure_stable_range = (np.min(stable_Pressure_values), np.max(stable_Pressure_values))

# Exclude the trivial solution (near zero values)
non_trivial_indices = np.where((EGF_grid > 0.1) & (Pressure_grid > 0.1))

# Filtered grid and vector components
filtered_EGF_grid = EGF_grid[non_trivial_indices]
filtered_Pressure_grid = Pressure_grid[non_trivial_indices]
filtered_dEGF = dEGF[non_trivial_indices]
filtered_dPressure = dPressure[non_trivial_indices]
# Filtering the stable values to match the non-trivial regions
filtered_stable_indices = np.where((stable_EGF_values > 0.1) & (stable_Pressure_values > 0.1))
filtered_stable_EGF_values = stable_EGF_values[filtered_stable_indices]
filtered_stable_Pressure_values = stable_Pressure_values[filtered_stable_indices]

# Adjusting the quiver parameters to ensure directional arrows are visible
fig, ax = plt.subplots(figsize=(10, 7))
ax.quiver(filtered_EGF_grid, filtered_Pressure_grid, filtered_dEGF, filtered_dPressure, angles='xy', scale_units='xy', scale=5, width=0.008, color='lightgray', label='Vector Field', pivot='mid', headwidth=4, headlength=5)
ax.scatter(filtered_stable_EGF_values, filtered_stable_Pressure_values, color='red', s=50, label='Stable Regions')
ax.set_title("Vector Field for EGFseen vs. Pressure with Stable Regions")
ax.set_xlabel("EGFseen")
ax.set_ylabel("Pressure")
ax.legend()
plt.tight_layout()
plt.show()

# Visualize the points of instability on the phase transition plots

fig, ax = plt.subplots(1, 2, figsize=(15, 6))

# Basal cells: EGFseen vs. Average Total with points of instability
ax[0].plot(growth_data_new["Basal Average EGFseen"], growth_data_new["Basal Average Total"], color="blue", label="Trajectory", alpha=0.5)
ax[0].scatter(instability_values_egf["Basal Average EGFseen"], instability_values_egf["Basal Average Total"], color="red", label="Points of Instability")
ax[0].set_title("Phase Transition: Basal Average Total vs. EGFseen")
ax[0].set_xlabel("Basal Average EGFseen")
ax[0].set_ylabel("Basal Average Total")
ax[0].legend()

# Basal cells: Pressure vs. Average Total with points of instability
ax[1].plot(growth_data_new["Basal Average Pressure"], growth_data_new["Basal Average Total"], color="blue", label="Trajectory", alpha=0.5)
ax[1].scatter(instability_values_pressure["Basal Average Pressure"], instability_values_pressure["Basal Average Total"], color="red", label="Points of Instability")
ax[1].set_title("Phase Transition: Basal Average Total vs. Pressure")
ax[1].set_xlabel("Basal Average Pressure")
ax[1].set_ylabel("Basal Average Total")
ax[1].legend()

plt.tight_layout()
plt.show()