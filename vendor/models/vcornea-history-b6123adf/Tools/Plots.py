import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import os
from pathlib import Path
import numpy as np
from tqdm import tqdm
from scipy.integrate import simps

def collect_files(directory):    
    parameter_files = []    
    count_files = []
    thickness_bins_files = []
    raw_thickness_files = []        
    
    for root, dirs, files in os.walk(directory):
        for file in files:
            if 'thickness_rep' in file:
                thickness_bins_files.append(os.path.join(root,file))                
            elif 'Parameters.py' in file:
                parameter_files.append(os.path.join(root,file))
            elif "cell_count_" in file:
                count_files.append(os.path.join(root,file))
            elif "thickness_raw_rep" in file:
                raw_thickness_files.append(os.path.join(root,file))

    return parameter_files, count_files, thickness_bins_files, raw_thickness_files

def Plot_Count(Data_dict, directory):
    stable_time = 500

    combined_data = pd.concat(Data_dict.values())
    combined_stable_data = combined_data[combined_data['Time'] >= stable_time]
    
    # Calculate mean of all columns across all datasets
    mean_data = combined_data.groupby('Time').mean().reset_index()
    
    fig, axes = plt.subplots(3, 2, figsize=(14, 8))
    axes[0, 0].axis('off')
    axes[0, 1].axis('off')  # Hide the empty subplot

    # Plot time series data in the first row, spanning both columns
    ax_longitudinal = plt.subplot2grid((3, 2), (0, 0), colspan=2)  
     
    # Calculate overall mean and standard deviation for each column    
    overall_stable_means = combined_stable_data.mean()
    overall_stable_stds = combined_stable_data.std()

    # Plot the mean data on the longitudinal plot        
    for i, column in enumerate(mean_data.columns[1:5]):        
        ax_longitudinal.plot(mean_data["Time"]/24, mean_data[column], label=f'Mean {column}', color=color_codes[i], linewidth=1 ) 
    
        mean_stable_val = overall_stable_means[column]
        std_stable_val = overall_stable_stds[column]

        row = (i // 2) + 1
        col = i % 2            
        ax_hist = axes[row, col]

        # Highlight the mean and std        
        ax_hist.axvline(mean_stable_val, color='black', linestyle='dashed', linewidth=1.0, label='Mean')
        ax_hist.axvspan(mean_stable_val - std_stable_val, mean_stable_val + std_stable_val, alpha=0.15, color="grey",linestyle='dashed', linewidth=1.0, label='±1 Std. Dev')
        ax_hist.legend()
        ax_hist.grid(True, linewidth=0.1) 
    
    for Comb,data in Data_dict.items():        
        for i, column in enumerate(data.columns[0:4]):           
            ax_longitudinal.plot(data['Time']/24, data[column], color=color_codes[i], alpha= 0.075, linewidth=1)
        
            row = (i // 2) + 1
            col = i % 2            
            ax_hist = axes[row, col]
            stable_data = data[data['Time'] >= stable_time]            
         
            ax_hist.hist(stable_data[column], bins=range(stable_data[column].min(),stable_data[column].max()+1, 1), color=color_codes[i], alpha= 0.075,  align="left") 

            ax_hist.set_title(f'Ensemble Stable {column} Cells Distributions')
            ax_hist.set_xlabel('Number of Cells')
            ax_hist.set_ylabel('Frequency')
    

    ax_longitudinal.axvline(stable_time/24, color='black', linestyle='dashed', linewidth=1, label="Stabilitity Time")
    ax_longitudinal.legend(loc='center right')
    ax_longitudinal.grid(True, linewidth=0.1)
    ax_longitudinal.set_title('Six Months Longitudinal Cell Count')
    ax_longitudinal.set_xlabel('Time (Days)')
    ax_longitudinal.set_ylabel('Number of Cells')

    plt.tight_layout()
    plt.subplots_adjust(hspace=0.40)
    # plt.savefig(directory + "/Cell_Count.png")
    plt.show()

def Plot_Thickness(data_dict, directory, bin_memb=None, suplemental=True):
    stable_time = 500
    unique_bins = set()
    for data in data_dict.values():
        unique_bins.update(data['Bin'].unique())
    unique_bins = sorted(unique_bins)
    color_palette = sns.color_palette("colorblind", len(unique_bins))
    bin_color_map = {bin_key: color_palette[i] for i, bin_key in enumerate(unique_bins)}
    bins_size = 200 // len(unique_bins)
    bin_x = {bin_key: [] for _, bin_key in enumerate(unique_bins)}

    for key in bin_x.keys():
        bin_x[key] = [i for i in range(key * bins_size, int((key + 1) * bins_size))]
        bin_x[key] = np.mean(bin_x[key])
    bin_actual = []
    bin_relative = []

    if suplemental:
        fig, ax = plt.subplots(3, figsize=(14, 17))
    else:
        fig, ax = plt.subplots(2, figsize=(14, 12))

    all_data = []

    for comb, data in data_dict.items():
        all_data.append(data)

        for bin_key in unique_bins:
            bin_data = data[data['Bin'] == bin_key]
            ax[0].plot(bin_data['Time'] / 24, bin_data['Height']*2,
                       alpha=0.075, linewidth=1, color=bin_color_map[bin_key])

    # Combine all data to calculate the mean
    combined_data = pd.concat(all_data)

    # Calculate mean and standard deviation for each bin over time
    mean_data = combined_data.groupby(['Time', 'Bin']).mean().reset_index()
    std_data = combined_data.groupby(['Time', 'Bin']).std().reset_index()

    # Plot the mean height for each bin
    for bin_key in unique_bins:
        bin_mean_data = mean_data[mean_data['Bin'] == bin_key]
        bin_std_data = std_data[std_data['Bin'] == bin_key]

        # Filter stable data
        stable_bin_data = combined_data[(combined_data['Bin'] == bin_key) & (combined_data['Time'] >= stable_time)]

        bin_actual.append((stable_bin_data['Height'].mean() - bin_memb[bin_key])*2)
        bin_relative.append((stable_bin_data['Height'].mean())*2)
        ax[0].plot(bin_mean_data['Time'] / 24, bin_mean_data['Height']*2, label=f'X Coord. {bin_key * bins_size}-{int((bin_key + 1) * bins_size) - 1}',
                   linewidth=1, color=bin_color_map[bin_key])
        
        # Scatter plot using stable data
        # jitter = 0.4
        # x_positions = np.random.normal(int(bin_x[bin_key]), jitter, size=len(stable_bin_data))
        # y_values_adjusted = (stable_bin_data['Height'] - bin_memb[bin_key]) * 2
        # y_values_actual = (stable_bin_data['Height']) * 2
        # ax[1].scatter(x_positions, y_values_adjusted,marker='.', color=bin_color_map[bin_key], alpha=0.1, s=1, edgecolor=None)
        # ax[1].scatter(x_positions, y_values_actual, marker='.',color=bin_color_map[bin_key], alpha=0.1, s=1, edgecolor=None)

        # Box plot of the stable data
        ax[1].boxplot((stable_bin_data['Height']-bin_memb[bin_key])*2, positions=[int(bin_x[bin_key])], widths=3, patch_artist=True,
                      boxprops=dict(facecolor=bin_color_map[bin_key], color='black', linewidth=0.5),
                      medianprops=dict(color='black', linewidth=0.5), whiskerprops=dict(color='black', linewidth=0.5),
                      capprops=dict(color='black', linewidth=0.5), flierprops=dict(marker='.',color='red', markersize=1,markeredgewidth=0.5, linewidth=0.5, alpha=0.5))
        ax[1].boxplot((stable_bin_data['Height'])*2, positions=[int(bin_x[bin_key])], widths=3, patch_artist=True,
                      boxprops=dict(facecolor=bin_color_map[bin_key], color='black', linewidth=0.5),
                      medianprops=dict(color='black', linewidth=0.5), whiskerprops=dict(color='black', linewidth=0.5),
                      capprops=dict(color='black', linewidth=0.5), flierprops=dict(marker='.',color='red', markersize=1,markeredgewidth=0.2, linewidth=0.5, alpha=0.5))
        
    ax[0].axvline(stable_time / 24, color='black', linestyle='dashed', linewidth=1, label="Stability Time")
    ax[0].set_title('Longitudinal Relative Height Variation of Tissue Sections Mean Over Six Months')
    ax[0].set_xlabel('Time (Days)')
    ax[0].set_ylabel('Relative Mean Height of Highest Cells (μm)')
    ax[0].legend(title='Bins', loc='lower right')
    ax[0].grid(True, linewidth=0.1)

    ax[1].plot(list(bin_x.values()), bin_actual, color='black', linestyle='dotted', linewidth=1, label="Tissue Actual Thickness")
    ax[1].plot(list(bin_x.values()), bin_relative, color='blue', linestyle='dotted', linewidth=1, label="Tissue Relative Height")

    ax[1].set_title('Mean Epithelium Relative Height and Actual Thickness Across Sections After Stabilization')
    ax[1].set_xlabel('Mean X Coordinate (lattice units)')
    ax[1].set_ylabel('Height/Thickness (μm)')
    ax[1].legend(title='Bins', loc='center right')
    ax[1].grid(True, linewidth=0.1)
    
    if suplemental:
        for bin_key in unique_bins:
            bin_data = combined_data[combined_data['Bin'] == bin_key]
            bin_data_stable = bin_data[bin_data['Time'] >= stable_time]        
            adjusted_height = (bin_data_stable['Height'] - bin_memb[bin_key]) * 2
            
            # Plot original heights
            sns.kdeplot((bin_data_stable['Height'])*2, ax=ax[2], label=f'Rel. H. X {bin_key * bins_size}-{int((bin_key + 1) * bins_size) - 1}',
                        linestyle='--', alpha=0.5, color=bin_color_map[bin_key])
            
            # Plot adjusted heights
            sns.kdeplot(adjusted_height, ax=ax[2], label=f'Act. Thic. X {bin_key * bins_size}-{int((bin_key + 1) * bins_size) - 1}',
                        fill=True, alpha=0.5, color=bin_color_map[bin_key])
        
        ax[2].set_title('Stable Height Distribution of Tissue Sections')
        ax[2].set_xlabel('Thickness (μm)')
        ax[2].set_ylabel('Density')
        ax[2].legend(title='Bins', loc='upper left')
        ax[2].grid(True, linewidth=0.1)

    plt.tight_layout()
    plt.subplots_adjust(hspace=0.20)
    # plt.savefig(f"{directory}/Thickness.png")
    plt.show()

def Plot_raw_data(Data_dict, directory, bins_number=10, Memb_Data=None ):
    thickness_data = {}
    bin_size = 200 / bins_number        
    bin_indexes = list(range(bins_number))
    bins = {index: [] for index in bin_indexes}

    timeset= set()
    for data in Data_dict.values():
        timeset.update(data['Time'].unique())
    timeset = sorted(timeset)
    
    for Comb, data in tqdm(Data_dict.items(), desc="Processing Raw Data Combinations"):
        thickness_data[Comb] = {}
        rows = []
        for time in timeset:
            time_data = data[data['Time'] == time]
            # Reset bins for each time point
            bins = {index: [] for index in bin_indexes}
            for index, row in time_data.iterrows():
                x = row['xCOM']
                y = row['yCOM']
                bin_index = int(x // bin_size)
                if bin_index in bins:
                    bins[bin_index].append(y)

            for bin_index in bin_indexes:
                cells = bins[bin_index]
                mean_top_height = np.mean(cells) if cells else 0
                rows.append({
                    'Time': time,
                    'Bin': bin_index,
                    'Height': mean_top_height
                })

        df = pd.DataFrame(rows)
        thickness_data[Comb] = df

    # if not (Memb_Data is None):
    bins_MEMB = {index: [] for index in bin_indexes}
    for key in Memb_Data.keys():           
        bin_index_MEMB = int(key // bin_size)          
        bins_MEMB[bin_index_MEMB].append(Memb_Data[key])
    for bin_index_memb in bins_MEMB.keys():
        bins_MEMB[bin_index_memb] = np.mean(bins_MEMB[bin_index_memb]) if bins_MEMB[bin_index_memb] else 0 

    Plot_Thickness(thickness_data,directory, bins_MEMB, )  
    

# Main function
# Load the data from files
directory = r"D:\STEM_ONLY_1"
Param_files, Count_files, Thickness_files, Raw_Thickness_files = collect_files(directory)

color_codes = ['#55ffff','#0055ff', '#ffbe99', '#ff007f',]
Param_data={}
for data_path in Param_files:    
    data = pd.read_csv(data_path)
    Param_data[str(Path(data_path).parts[-4])+"_"+str(Path(data_path).parts[-3])] = data
Count_data={}
for data_path in Count_files:    
    data = pd.read_csv(data_path)
    Count_data[str(Path(data_path).parts[-4])+"_"+str(Path(data_path).parts[-3])] = data
Thickness_data = {}
for data_path in Thickness_files:    
    data = pd.read_parquet(data_path)
    Thickness_data[str(Path(data_path).parts[-4])+"_"+str(Path(data_path).parts[-3])] = data
Raw_Thickness_data = {}
for data_path in Raw_Thickness_files:    
    data = pd.read_parquet(data_path)
    Raw_Thickness_data[str(Path(data_path).parts[-4])+"_"+str(Path(data_path).parts[-3])] = data
Memb_Data = {}
rows = pd.read_csv(r"D:\STEM_ONLY_1\membrane_height.csv", header=None)
for index, row in rows.iterrows():
    # print(row[0], row[1])
    Memb_Data[row[0]] = row[1]

bin_size = 200 / 5        
bin_indexes = list(range(5))
bins_MEMB = {index: [] for index in bin_indexes}

for key in Memb_Data.keys():           
    bin_index_MEMB = int(key // bin_size)          
    bins_MEMB[bin_index_MEMB].append(Memb_Data[key])
for bin_index_memb in bins_MEMB.keys():
    bins_MEMB[bin_index_memb] = np.mean(bins_MEMB[bin_index_memb]) if bins_MEMB[bin_index_memb] else 0


# Plot_Count(Count_data, directory)
# Plot_Thickness(Thickness_data, directory, bins_MEMB,)
# Plot_raw_data(Raw_Thickness_data, directory, 10, Memb_Data)

