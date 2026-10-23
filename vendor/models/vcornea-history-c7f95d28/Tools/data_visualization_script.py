
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm

# Load the data
file_path = r"C:\Users\joelv\Documents\GitHub\vCornea\Processed Data\Output_Version_1\Output_version_1_run3_checking_lambdaVolume_STEM_09292023_0956\Partial\Comb0\0\single_cell_pres_EGF_rep_14601.csv"
data = pd.read_csv(file_path)

# Split data into BASAL and STEM cells
basal_cell_data = data[data["CellType"] == "BASAL"]
stem_cell_data = data[data["CellType"] == "STEM"]

# Histograms showcasing the distributions of absolute pressure and EGF values for both BASAL and STEM cells
fig, axes = plt.subplots(2, 2, figsize=(20, 12))
axes[0, 0].hist(basal_cell_data['pressure'].abs(), bins=300, color='blue', alpha=0.7)
axes[0, 0].set_title('Distribution of abs(Pressure) for BASAL Cells')
axes[0, 0].set_xlabel('abs(Pressure)')
axes[0, 0].set_ylabel('Frequency')
axes[0, 1].hist(basal_cell_data['EGF'], bins=300, color='red', alpha=0.7)
axes[0, 1].set_title('Distribution of EGF for BASAL Cells')
axes[0, 1].set_xlabel('EGF')
axes[0, 1].set_ylabel('Frequency')
axes[1, 0].hist(stem_cell_data['pressure'].abs(), bins=300, color='blue', alpha=0.7)
axes[1, 0].set_title('Distribution of abs(Pressure) for STEM Cells')
axes[1, 0].set_xlabel('abs(Pressure)')
axes[1, 0].set_ylabel('Frequency')
axes[1, 1].hist(stem_cell_data['EGF'], bins=300, color='red', alpha=0.7)
axes[1, 1].set_title('Distribution of EGF for STEM Cells')
axes[1, 1].set_xlabel('EGF')
axes[1, 1].set_ylabel('Frequency')
plt.tight_layout()
plt.show()

# Hexbin plots for the entire dataset
fig, axes = plt.subplots(2, 1, figsize=(20, 16))
hb1 = axes[0].hexbin(basal_cell_data['pressure'].abs(), basal_cell_data['EGF'], gridsize=300, cmap='viridis', extent=(0, max(basal_cell_data['pressure'].abs()), 0, max(stem_cell_data['EGF'])))
axes[0].set_title("BASAL Cells: abs(Pressure) vs EGF 14600 mcs")
axes[0].set_xlabel("abs(Pressure)")
axes[0].set_ylabel("EGF")
cb1 = fig.colorbar(hb1, ax=axes[0])
cb1.set_label('Count of Hits')
hb2 = axes[1].hexbin(stem_cell_data['pressure'].abs(), stem_cell_data['EGF'], gridsize=300, cmap='viridis', extent=(0, max(stem_cell_data['pressure'].abs()), 0, max(stem_cell_data['EGF'])))
axes[1].set_title("STEM Cells: abs(Pressure) vs EGF 14600 mcs")
axes[1].set_xlabel("abs(Pressure)")
axes[1].set_ylabel("EGF")
cb2 = fig.colorbar(hb2, ax=axes[1])
cb2.set_label('Count of Hits')
plt.tight_layout()
plt.show()

