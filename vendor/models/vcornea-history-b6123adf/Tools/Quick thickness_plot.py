import matplotlib.pyplot as plt
import pandas as pd

# Load the data from the CSV file
file_path = r"C:\Users\joelv\Documents\GitHub\vCornea\Epithelium\Local\Project\paper_version_STEM_ONLY\Simulation\thickness_36002.csv"
data = pd.read_csv(file_path)

# Plotting Limbus Avg Thickness and Periphery Avg Thickness over time
plt.figure(figsize=(12, 6))
plt.plot(data['Time'], data['Limbus Avg Thickness'], label='Limbus Avg Thickness', color='dodgerblue')
# line for the mean and standard deviation
# plt.plot((data['Time'].min(), data['Time'].max()), (data['Limbus Avg Thickness'].mean(), data['Limbus Avg Thickness'].mean()), label='Limbus Avg Thickness', color='blue')
plt.plot((data['Time'].min(), data['Time'].max()), (data['Limbus Avg Thickness'].mean() + data['Limbus Avg Thickness'].std(), data['Limbus Avg Thickness'].mean() + data['Limbus Avg Thickness'].std()), label='Limbus Avg Thickness', color='blue', linestyle='dashed')
# plt.plot(data['Time'], data['Periphery Avg Thickness'].mean(), label='Periphery Avg Thickness', color='green')
plt.plot(data['Time'], data['Periphery Avg Thickness'], label='Periphery Avg Thickness', color='limegreen')
plt.plot((data['Time'].min(), data['Time'].max()), (data['Periphery Avg Thickness'].mean() + data['Periphery Avg Thickness'].std(), data['Periphery Avg Thickness'].mean() + data['Periphery Avg Thickness'].std()), label='Periphery Avg Thickness', color='green', linestyle='dashed')

# Highlighting steady state (if applicable)
# Here we assume the tissue reaches steady state if the thickness varies within a small tolerance for several consecutive steps
# def detect_steady_state(thickness, tolerance=1000, min_consecutive=100):
#     steady_state_times = []
#     steady_count = 0
#     for i in range(1, len(thickness)):
#         if abs(thickness[i] - thickness[i - 1]) <= tolerance:
#             steady_count += 1
#             if steady_count >= min_consecutive:
#                 steady_state_times.append(i)
#         else:
#             steady_count = 0
#     return steady_state_times

# # Define tolerance and min_consecutive for steady state detection
# tolerance = 0.75
# min_consecutive = 100

# steady_state_limbus = detect_steady_state(data['Limbus Avg Thickness'], tolerance, min_consecutive)
# steady_state_periphery = detect_steady_state(data['Periphery Avg Thickness'], tolerance, min_consecutive)

# # Highlighting the steady state periods
# for time in steady_state_limbus:
#     plt.axvspan(data['Time'][time - min_consecutive], data['Time'][time], color='blue', alpha=0.1)
# for time in steady_state_periphery:
#     plt.axvspan(data['Time'][time - min_consecutive], data['Time'][time], color='green', alpha=0.1)

# Adding labels and title
plt.xlabel('Time')
plt.ylabel('Thickness')
plt.title('Tissue Thickness Over Time')
plt.legend()

# Save the plot to a file
# plt.savefig('tissue_thickness_plot.png')

# Show the plot
plt.show()