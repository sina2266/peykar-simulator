import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator
import numpy as np

# # Load the CSV into a DataFrame
# df = pd.read_csv(r"C:\Users\joelv\OneDrive\Desktop\New folder\vCornea_v_PaperHPC_version_3\Simulation\mass_conservation_rep_14601.csv")

# # Convert the DataFrame to Parquet
# df.to_parquet(r"C:\Users\joelv\OneDrive\Desktop\Bigfile.parquet", index=False)




# Load the Parquet file into a DataFrame
df_parquet = pd.read_parquet(r"C:\Users\joelv\OneDrive\Desktop\Bigfile.parquet")

# Display the first few rows of the DataFrame
print(df_parquet.head())

# Simple plot example
# Let's say you want to visualize the volume distribution of different event types over time
event_types = df_parquet["Event Type"].unique()
print(len(event_types))

# for event in event_types:
#     subset = df_parquet[df_parquet["Event Type"] == event]
#     plt.scatter(subset["Time"], subset["Volume"], label=event, s=2, alpha=0.5)

# plt.xlabel("Time")
# plt.ylabel("Volume")
# plt.legend()
# plt.show()

#_______________________________________________________________
# Plotting sigle cell data events over time vs volume

# cell_ids = df_parquet["Cell ID"].unique()
# subset2 = df_parquet[df_parquet["Cell ID"] == 5443]
# for event in subset2["Event Type"].unique():
#     subset3 = subset2[subset2["Event Type"] == event]
#     plt.scatter(subset3["Time"], subset3["Volume"], label=event, s=10, alpha=0.5)

# plt.xlabel("Time")
# plt.ylabel("Volume")
# plt.grid(True, linewidth=0.2)
# plt.legend()
# plt.show()

#_______________________________________________________________
markers = ['o', 's', '^', 'D', 'p', '*', 'v', '<', '>', 'H', '+', 'x', '|', '_']
cell_ids = df_parquet["Cell ID"].unique()
subset2 = df_parquet[df_parquet["Cell ID"] == 5443]
unique_events = subset2["Event Type"].unique()

# Create a dictionary to map each event to a marker
event_to_marker = {event: markers[i % len(markers)] for i, event in enumerate(unique_events)}

for event in unique_events:
    subset3 = subset2[subset2["Event Type"] == event]
    plt.scatter(subset3["Time"], subset3["Volume"], label=event, alpha=0.5, marker=event_to_marker[event])

plt.xlabel("Time")
plt.ylabel("Volume")
plt.grid(True, linewidth=0.2)
plt.legend()
plt.show()