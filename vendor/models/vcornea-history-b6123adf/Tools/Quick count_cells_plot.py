import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd

# Load the data from the CSV file
data_path = r"C:\Users\joelv\Documents\GitHub\vCornea\Epithelium\Local\Project\paper_version_STEM_ONLY\Simulation\cell_count_36002.csv"
data_5m = pd.read_csv(data_path)

color_codes = ['#55ffff','#0055ff', '#ffbe99', '#ff007f',]

# Plot longitudinal data and histograms in a 2x2 grid
fig = plt.figure(figsize=(14, 10))

# Plot time series data in the first row, spanning both columns
ax_longitudinal = plt.subplot2grid((3, 2), (0, 0), colspan=2)
for i, column in enumerate(data_5m.columns[0:4]):
    ax_longitudinal.plot(data_5m['Time'], data_5m[column], label=column, color=color_codes[i])
ax_longitudinal.set_title('Longitudinal Data')
ax_longitudinal.set_xlabel('Time (Hours)')
ax_longitudinal.set_ylabel('Number of Cells')
ax_longitudinal.legend()

# Plot histograms for the distributions in a 2x2 grid
for i, column in enumerate(data_5m.columns[0:4]):
    row = (i // 2) + 1
    col = i % 2
    ax_hist = plt.subplot2grid((3, 2), (row, col))
    sns.histplot(data_5m[column], kde=True, ax=ax_hist, label=column, element='step', color=color_codes[i])
    ax_hist.set_title(f'{column} Distribution')
    ax_hist.set_xlabel('Number of Cells')
    ax_hist.set_ylabel('Frequency')
    ax_hist.legend()

plt.tight_layout()
plt.show()