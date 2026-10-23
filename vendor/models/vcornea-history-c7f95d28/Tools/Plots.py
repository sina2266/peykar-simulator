import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
# from scipy.interpolate import UnivariateSpline, InterpolatedUnivariateSpline
import os
from pathlib import Path
import numpy as np
from tqdm import tqdm
from scipy.stats import skew, kurtosis, kstest, shapiro, norm, probplot
from scipy import stats

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
            # elif "thickness_raw_rep" in file:
            #     raw_thickness_files.append(os.path.join(root,file))

    return parameter_files, count_files, thickness_bins_files, raw_thickness_files

# def Plot_Count(Data_dict, directory):
#     stable_time = 500
#     qq_values = {}
#     combined_data = pd.concat(Data_dict.values())
#     combined_stable_data = combined_data[combined_data['Time'] >= stable_time]
    
#     # Calculate mean of all columns across all datasets
#     mean_data = combined_data.groupby('Time').mean().reset_index()
    
#     fig, axes = plt.subplots(4, 2, figsize=(14, 12))
#     axes[0, 0].axis('off')
#     axes[0, 1].axis('off')  # Hide the empty subplot
#     axes[3, 0].axis('off')
#     axes[3, 1].axis('off')

#     # Plot time series data in the first row, spanning both columns
#     ax_longitudinal = plt.subplot2grid((4, 2), (0, 0), colspan=2)  
#     ax_qq_combined = plt.subplot2grid((4, 2), (3, 0), colspan=2, rowspan=1)
#     # Plot the mean data on the longitudinal plot        
#     for i, column in enumerate(mean_data.columns[1:5]):        
#         ax_longitudinal.plot(mean_data["Time"]/24, mean_data[column], label=f'Mean {column}', color=color_codes[i], linewidth=1) 
    
#         row = (i // 2) + 1
#         col = i % 2            
#         ax_hist = axes[row, col]

#         # Combine all stable data for the current column
#         all_stable_data = np.concatenate([data[data['Time'] >= stable_time][column].values for data in Data_dict.values()])
        
#         # Calculate bin edges
#         bin_edges = np.arange(int(all_stable_data.min()) - 0.5, int(all_stable_data.max()) + 1.5, 1)

#         # Plot individual distributions and calculate mean
#         all_hists = []
#         for Comb, data in Data_dict.items():
#             stable_data = data[data['Time'] >= stable_time]
#             hist, _, _ = ax_hist.hist(stable_data[column], bins=bin_edges, color=color_codes[i], alpha=0.075)
#             all_hists.append(hist)

#         # Calculate mean histogram
#         mean_hist = np.mean(all_hists, axis=0)
        
#         ax_hist.grid(True, linewidth=0.1) 
#         ax_hist.set_title(f'Variation of {column}-Cell Number in Time and Between Replicates')
#         ax_hist.set_xlabel('Number of Cells per Time')
#         ax_hist.set_ylabel('Frequency')

#         # Calculate and display skewness and kurtosis
#         skewness = skew(all_stable_data)
#         # kurt = kurtosis(all_stable_data)
#         # ax_hist.text(0.95, 0.95, f'Skewness: {skewness:.2f}\nKurtosis: {kurt:.2f}', 
#         #              transform=ax_hist.transAxes, verticalalignment='top', horizontalalignment='right')
#         print(f'{column} Skewness: {skewness:.2f}')
#         print(f'{column} Kurtosis: {kurtosis(all_stable_data):.2f}')

#         # Kolmogorov-Smirnov test
#         ks_stat, ks_p = kstest(all_stable_data, 'norm', args=(np.mean(all_stable_data), np.std(all_stable_data)))
#         # ax_hist.text(0.95, 0.75, f'KS stat: {ks_stat:.2f}\nKS p-value: {ks_p:.2e}', 
#         #              transform=ax_hist.transAxes, verticalalignment='top', horizontalalignment='right')
#         print(f'{column} KS stat: {ks_stat:.2f}')
#         print(f'{column} KS p-value: {ks_p:.2e}')

#         # Shapiro-Wilk test
#         sw_stat, sw_p = shapiro(all_stable_data)
#         # ax_hist.text(0.95, 0.55, f'SW stat: {sw_stat:.2f}\nSW p-value: {sw_p:.2e}', 
#         #              transform=ax_hist.transAxes, verticalalignment='top', horizontalalignment='right')
#         print(f'{column} SW stat: {sw_stat:.2f}')
#         print(f'{column} SW p-value: {sw_p:.2e}')

#         result = stats.anderson(all_stable_data, dist='norm')
#         print(f'{column} A stat:', result.statistic)
#         print(f'{column} A Crit:', result.critical_values)
#         print(f'{column} A signi:', result.significance_level)

#         # print(f'{column} Median:', np.median(all_stable_data))
#         # print(f'{column} Variance:', np.var(all_stable_data))
#         # print(f'{column} STD:', np.std(all_stable_data))

#         # ax_qq_combined = axes[3, 0]
#         # Q-Q plot       
#         (osm, osr), (slope, intercept, r) = probplot(combined_stable_data[column], dist="norm", plot=ax_qq_combined)
#         # Store Q-Q plot values
        
#         lines = ax_qq_combined.get_lines()
#         if len(lines) >= 2 * (i + 1):  # Ensure there are enough lines
#             # Access the corresponding lines for the current plot
#             observed_line = lines[2 * i]
#             expected_line = lines[2 * i + 1]
#             # Change the color of the scatter plot points (observed values)
#             observed_line.set_color(color_codes[i])
#             observed_line.set_alpha(0.25)
#             # observed_line.set_markerfacecolor(color_codes[i])
#             # observed_line.set_markeredgecolor(color_codes[i])
#             observed_line.set_label(f'{column}')
#             # Change the color of the theoretical line (expected values)
#             expected_line.set_color('black')
#             expected_line.set_linestyle('dashed')
#             # Calculate differences between observed and theoretical quantiles
#             differences = np.abs(osm - osr)
#             mean_difference = np.mean(differences)
#             max_difference = np.max(differences)
#             print(f'{column} Q-Q Slope: {slope:.2f}, Intercept: {intercept:.2f}, R: {r:.2f}')
#             print(f'{column} Q-Q Mean difference: {mean_difference:.2f}, Max difference: {max_difference:.2f}')
#             print()
            
#         # ax_box = axes[3, 1]    
#         # sns.boxplot(x='Time', y=column, data=combined_stable_data, ax=ax_box, color=color_codes[i], linewidth=0.5)
#         # ax_box.set_title(f'Boxplot of {column} Over Time')
#         # ax_box.set_xlabel('Time')
#         # ax_box.set_ylabel('Number of Cells')

#     expected_line.set_label(f'Normal Distribution')    
#     ax_qq_combined.set_title('Combined Q-Q Plots')
#     ax_qq_combined.grid(True, linewidth=0.1)
#     ax_qq_combined.legend()
    
#     for Comb, data in Data_dict.items():        
#         for i, column in enumerate(data.columns[0:4]):           
#             ax_longitudinal.plot(data['Time']/24, data[column], color=color_codes[i], alpha=0.075, linewidth=1)

#     ax_longitudinal.axvline(stable_time/24, color='black', linestyle='dashed', linewidth=1, label="Stability Time")
#     ax_longitudinal.legend(loc='center right')
#     ax_longitudinal.grid(True, linewidth=0.1)
#     ax_longitudinal.set_title('Longitudinal Cell Count')
#     ax_longitudinal.set_xlabel('Time (Days)')
#     ax_longitudinal.set_ylabel('Number of Cells')

#     plt.tight_layout()
#     plt.subplots_adjust(hspace=0.40)
#     # plt.savefig(directory + "/Cell_Count.png")
#     plt.show()
def Plot_Count(Data_dict, directory):
    stable_time = 15*24  # Time in hours
    max_time = 720  # Define the maximum time limit
    qq_values = {}
    
    # Filter the data to include only up to the max_time
    combined_data = pd.concat(Data_dict.values())
    combined_data = combined_data[combined_data['Time'] <= max_time]
    
    combined_stable_data = combined_data[combined_data['Time'] >= stable_time]
    
    # Calculate mean of all columns across all datasets
    mean_data = combined_data.groupby('Time').mean().reset_index()
    
    fig, axes = plt.subplots(4, 2, figsize=(14, 12))
    axes[0, 0].axis('off')
    axes[0, 1].axis('off')  # Hide the empty subplot
    axes[3, 0].axis('off')
    axes[3, 1].axis('off')

    # Plot time series data in the first row, spanning both columns
    ax_longitudinal = plt.subplot2grid((4, 2), (0, 0), colspan=2)  
    # ax_longitudinal = plt 
    ax_qq_combined = plt.subplot2grid((4, 2), (3, 0), colspan=2, rowspan=1)
    
    # Plot the mean data on the longitudinal plot        
    for i, column in enumerate(mean_data.columns[1:5]):        
        ax_longitudinal.plot(mean_data["Time"]/24, mean_data[column], label=f'Mean {column}', color=color_codes[i], linewidth=2) 
    
        row = (i // 2) + 1
        col = i % 2            
        ax_hist = axes[row, col]

        # Combine all stable data for the current column
        all_stable_data = np.concatenate([data[(data['Time'] >= stable_time) & (data['Time'] <= max_time)][column].values for data in Data_dict.values()])
        
        # Calculate bin edges
        bin_edges = np.arange(int(all_stable_data.min()) - 0.5, int(all_stable_data.max()) + 1.5, 1)

        # Plot individual distributions and calculate mean
        all_hists = []
        for Comb, data in Data_dict.items():
            stable_data = data[(data['Time'] >= stable_time) & (data['Time'] <= max_time)]
            hist, _, _ = ax_hist.hist(stable_data[column], bins=bin_edges, color=color_codes[i], alpha=0.075)
            all_hists.append(hist)

        # Calculate mean histogram
        mean_hist = np.mean(all_hists, axis=0)
        
        ax_hist.grid(True, linewidth=0.1) 
        ax_hist.set_title(f'Variation of {column}-Cell Number in Time and Between Replicates')
        ax_hist.set_xlabel('Number of Cells per Time')
        ax_hist.set_ylabel('Frequency')

        # Calculate and display skewness and kurtosis
        skewness = skew(all_stable_data)
        print(f'{column} Skewness: {skewness:.2f}')
        print(f'{column} Kurtosis: {kurtosis(all_stable_data):.2f}')

        # Kolmogorov-Smirnov test
        ks_stat, ks_p = kstest(all_stable_data, 'norm', args=(np.mean(all_stable_data), np.std(all_stable_data)))
        print(f'{column} KS stat: {ks_stat:.2f}')
        print(f'{column} KS p-value: {ks_p:.2e}')

        # Shapiro-Wilk test
        sw_stat, sw_p = shapiro(all_stable_data)
        print(f'{column} SW stat: {sw_stat:.2f}')
        print(f'{column} SW p-value: {sw_p:.2e}')

        result = stats.anderson(all_stable_data, dist='norm')
        print(f'{column} A stat:', result.statistic)
        print(f'{column} A Crit:', result.critical_values)
        print(f'{column} A signi:', result.significance_level)

        # Q-Q plot       
        (osm, osr), (slope, intercept, r) = probplot(combined_stable_data[column], dist="norm", plot=ax_qq_combined)
        
        lines = ax_qq_combined.get_lines()
        if len(lines) >= 2 * (i + 1):  # Ensure there are enough lines
            observed_line = lines[2 * i]
            expected_line = lines[2 * i + 1]
            observed_line.set_color(color_codes[i])
            observed_line.set_alpha(0.25)
            observed_line.set_label(f'{column}')
            expected_line.set_color('black')
            expected_line.set_linestyle('dashed')
            differences = np.abs(osm - osr)
            mean_difference = np.mean(differences)
            max_difference = np.max(differences)
            print(f'{column} Q-Q Slope: {slope:.2f}, Intercept: {intercept:.2f}, R: {r:.2f}')
            print(f'{column} Q-Q Mean difference: {mean_difference:.2f}, Max difference: {max_difference:.2f}')
            print()
            
    expected_line.set_label(f'Normal Distribution')    
    ax_qq_combined.set_title('Combined Q-Q Plots')
    ax_qq_combined.grid(True, linewidth=0.1)
    ax_qq_combined.legend()
    data = data[data['Time'] <= max_time]

    for Comb, data in Data_dict.items():        
        # Filter the data to include only times less than 720
        filtered_data = data[data['Time'] < 720]
        for i, column in enumerate(filtered_data.columns[0:4]):                     
            ax_longitudinal.plot(filtered_data['Time']/24, filtered_data[column], color=color_codes[i], alpha=0.075, linewidth=2)

    ax_longitudinal.axvline(stable_time/24, color='black', linestyle='dashed', linewidth=1, label="Stability Time")
    ax_longitudinal.legend(loc='center right')
    ax_longitudinal.grid(True, linewidth=0.1)
    # ax_longitudinal.title('Longitudinal Cell Count')
    # ax_longitudinal.xlabel('Time (Days)')
    # ax_longitudinal.ylabel('Number of Cells')
    ax_longitudinal.set_title('Longitudinal Cell Count')
    ax_longitudinal.set_xlabel('Time (Days)')
    ax_longitudinal.set_ylabel('Number of Cells')

    plt.tight_layout()
    plt.subplots_adjust(hspace=0.40)
    plt.show()
def Plot_Thickness(data_dict, directory, bin_memb=None, suplemental=False):
    stable_time = 500  # Time in hours
    unique_bins = set()
    for data in data_dict.values():
        unique_bins.update(data['Bin'].unique())
    unique_bins = sorted(unique_bins)
    color_palette = sns.color_palette("colorblind", len(unique_bins))
    bin_color_map = {bin_key: color_palette[i] for i, bin_key in enumerate(unique_bins)}
    bins_size = 200 // len(unique_bins)  # Size in μm
    bin_x = {bin_key: [] for _, bin_key in enumerate(unique_bins)}

    for key in bin_x.keys():
        bin_x[key] = [i for i in range(key * bins_size, int((key + 1) * bins_size))]
        bin_x[key] = np.mean(bin_x[key])  # Position in μm
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
            ax[0].plot(bin_data['Time'] / 24, bin_data['Height']*2,  # Convert hours to days, half-thickness to full thickness
                       alpha=0.075, linewidth=1, color=bin_color_map[bin_key])

    combined_data = pd.concat(all_data)
    mean_data = combined_data.groupby(['Time', 'Bin']).mean().reset_index()
    std_data = combined_data.groupby(['Time', 'Bin']).std().reset_index()

    for bin_key in unique_bins:
        bin_mean_data = mean_data[mean_data['Bin'] == bin_key]
        bin_std_data = std_data[std_data['Bin'] == bin_key]

        stable_bin_data = combined_data[(combined_data['Bin'] == bin_key) & (combined_data['Time'] >= stable_time)]

        bin_actual.append((stable_bin_data['Height'].mean() - bin_memb[bin_key])*2)  # Full thickness in μm
        bin_relative.append((stable_bin_data['Height'].mean())*2)  # Full height in μm
        ax[0].plot(bin_mean_data['Time'] / 24, bin_mean_data['Height']*2, label=f'X Coord. {bin_key * bins_size}-{int((bin_key + 1) * bins_size) - 1}',
                   linewidth=1, color=bin_color_map[bin_key])

        # Adjust box plot positions
        box_position = bin_x[bin_key] * 2  # Multiply by 2 to match the new x-axis scale

        ax[1].boxplot((stable_bin_data['Height']-bin_memb[bin_key])*2, positions=[box_position], widths=6, patch_artist=True,
                      boxprops=dict(facecolor='tab:red', color='black', linewidth=0.5),
                      medianprops=dict(color='black', linewidth=0.5), whiskerprops=dict(color='black', linewidth=0.5),
                      capprops=dict(color='black', linewidth=0.5), flierprops=dict(marker='.',color='red', markersize=1,markeredgewidth=0.5, linewidth=0.5, alpha=0.5))
        ax[1].boxplot((stable_bin_data['Height'])*2, positions=[box_position], widths=6, patch_artist=True,
                      boxprops=dict(facecolor='tab:blue', color='black', linewidth=0.5),
                      medianprops=dict(color='black', linewidth=0.5), whiskerprops=dict(color='black', linewidth=0.5),
                      capprops=dict(color='black', linewidth=0.5), flierprops=dict(marker='.',color='red', markersize=1,markeredgewidth=0.5, linewidth=0.5, alpha=0.5))
        
    ax[0].axvline(stable_time / 24, color='black', linestyle='dashed', linewidth=1, label="Stability Time")
    ax[0].set_title('Longitudinal Variation of Tissue Top Position Over Six Months')
    ax[0].set_xlabel('Time (Days)')
    ax[0].set_ylabel('Vertical Position (μm)')
    ax[0].legend(title='Bins', loc='lower right')
    ax[0].grid(True, linewidth=0.1)

    x_values = [val * 2 for val in bin_x.values()]  # Convert to full width in μm
    ax[1].plot(x_values, bin_actual, color='tab:red', linestyle='dotted', linewidth=1, label="Tissue Thickness")
    ax[1].plot(x_values, bin_relative, color='tab:blue', linestyle='dotted', linewidth=1, label="Tissue Top Position")

    ax[1].set_title('Stable Tissue Top Possition and Thickness Across Sections')
    ax[1].set_xlabel('Horizontal Position (μm)')
    ax[1].set_ylabel('Vertical Position / Thickness (μm)')
    ax[1].legend(title='Bins', loc='center right')
    ax[1].grid(True, linewidth=0.1)
    
    if suplemental:
        for bin_key in unique_bins:
            bin_data = combined_data[combined_data['Bin'] == bin_key]
            bin_data_stable = bin_data[bin_data['Time'] >= stable_time]        
            adjusted_height = (bin_data_stable['Height'] - bin_memb[bin_key]) * 2  # Full thickness in μm
            
            sns.kdeplot((bin_data_stable['Height'])*2, ax=ax[2], label=f'Rel. H. X {bin_key * bins_size}-{int((bin_key + 1) * bins_size) - 1}',
                        linestyle='--', alpha=0.5, color=bin_color_map[bin_key])
            
            sns.kdeplot(adjusted_height, ax=ax[2], label=f'Act. Thic. X {bin_key * bins_size}-{int((bin_key + 1) * bins_size) - 1}',
                        fill=True, alpha=0.5, color=bin_color_map[bin_key])
        
        ax[2].set_title('Stable Tissue Top Possition and Thickness Distribution of Tissue Sections')
        ax[2].set_xlabel('Thickness (μm)')
        ax[2].set_ylabel('Density')
        ax[2].legend(title='Bins', loc='upper left')
        ax[2].grid(True, linewidth=0.1)

    plt.tight_layout()
    plt.subplots_adjust(hspace=0.20)
    # plt.savefig(f"{directory}/Thickness.png")
    plt.show()

# def Plot_raw_data(Data_dict, directory, bins_number=10, Memb_Data=None ):
#     thickness_data = {}
#     bin_size = 200 / bins_number        
#     bin_indexes = list(range(bins_number))
#     bins = {index: [] for index in bin_indexes}

#     timeset= set()
#     for data in Data_dict.values():
#         timeset.update(data['Time'].unique())
#     timeset = sorted(timeset)
    
#     for Comb, data in tqdm(Data_dict.items(), desc="Processing Raw Data Combinations"):
#         thickness_data[Comb] = {}
#         rows = []
#         for time in timeset:
#             time_data = data[data['Time'] == time]
#             # Reset bins for each time point
#             bins = {index: [] for index in bin_indexes}
#             for index, row in time_data.iterrows():
#                 x = row['xCOM']
#                 y = row['yCOM']
#                 bin_index = int(x // bin_size)
#                 if bin_index in bins:
#                     bins[bin_index].append(y)

#             for bin_index in bin_indexes:
#                 cells = bins[bin_index]
#                 mean_top_height = np.mean(cells) if cells else 0
#                 rows.append({
#                     'Time': time,
#                     'Bin': bin_index,
#                     'Height': mean_top_height
#                 })

#         df = pd.DataFrame(rows)
#         thickness_data[Comb] = df

#     # if not (Memb_Data is None):
#     bins_MEMB = {index: [] for index in bin_indexes}
#     for key in Memb_Data.keys():           
#         bin_index_MEMB = int(key // bin_size)          
#         bins_MEMB[bin_index_MEMB].append(Memb_Data[key])
#     for bin_index_memb in bins_MEMB.keys():
#         bins_MEMB[bin_index_memb] = np.mean(bins_MEMB[bin_index_memb]) if bins_MEMB[bin_index_memb] else 0 

#  Plot_Thickness(thickness_data,directory, bins_MEMB, ) 
def plot_statistics_over_time(data_dict, stable_time=500):
    combined_data = pd.concat(data_dict.values())
    time_points = combined_data['Time'].unique()
    
    statistics = {'Time': [], 'Cell Type': [], 'Median': [], 'Variance': [], 'Skewness': [], 'Kurtosis': []}
    
    for t in time_points:
        if t >= stable_time:
            for column in combined_data.columns[0:4]:
                data_at_time = combined_data[combined_data['Time'] == t][column]
                statistics['Time'].append(t)
                statistics['Cell Type'].append(column)
                statistics['Median'].append(np.median(data_at_time))
                statistics['Variance'].append(np.var(data_at_time))
                statistics['Skewness'].append(skew(data_at_time))
                statistics['Kurtosis'].append(kurtosis(data_at_time))
    
    stats_df = pd.DataFrame(statistics)
    
    fig, axs = plt.subplots(2, 2, figsize=(14, 12))
    axs = axs.flatten()
    stat_types = ['Median', 'Variance', 'Skewness', 'Kurtosis']
    
    for i, stat in enumerate(stat_types):
        ax = axs[i]
        for j, column in enumerate(combined_data.columns[0:4]):
            data_to_plot = stats_df[stats_df['Cell Type'] == column]
            ax.plot(data_to_plot['Time'], data_to_plot[stat], label=column, color=color_codes[j], alpha=0.5)
        ax.set_title(f'{stat} Over Time')
        ax.set_xlabel('Time')
        ax.set_ylabel(stat)
        ax.legend()
    
    plt.tight_layout()
    plt.show()



# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Main function
# Load the data from files
# directory = r"D:\STEM_ONLY_1\Comb1"
# directory = r"D:\Injury_1\Control\Comb1"

# directory = r"D:\Injury_1\750_G"
# directory = r"D:\Injury_1\750_U"
# directory = r"D:\Injury_1\1000_G\Comb1"
# directory = r"D:\Injury_1\1000_U"
# directory = r"D:\Injury_1\1500_G\Comb1"
# directory = r"D:\Injury_1\1500_U"
# directory = r"D:\Injury_1\2000_G\Comb1"
# directory = r"D:\Injury_1\2000_U"
# directory = r"D:\Injury_1\2500_G\Comb1"
# directory = r"D:\Injury_1\2500_U"
# directory = r"D:\STEM_ONLY_1\Comb1"
directory = r"D:\Injury_1\CHEMICAL_Injury_Gau_1500_2000_2600_08252024_2046\Comb1"
# directory = r"D:\Injury_1\STEM_ONLY_proper_thickness_6months_08252024_2058\Comb0"



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
# Raw_Thickness_data = {}
# for data_path in Raw_Thickness_files:    
#     data = pd.read_parquet(data_path)
#     Raw_Thickness_data[str(Path(data_path).parts[-4])+"_"+str(Path(data_path).parts[-3])] = data
Memb_Data = {}
rows = pd.read_csv(r"D:\STEM_ONLY_1\membrane_height.csv", header=None)
for index, row in rows.iterrows():
    # print(row[0], row[1])
    Memb_Data[row[0]] = row[1]

bin_size = 200 / 10        
bin_indexes = list(range(10))
bins_MEMB = {index: [] for index in bin_indexes}

for key in Memb_Data.keys():           
    bin_index_MEMB = int(key // bin_size)          
    bins_MEMB[bin_index_MEMB].append(Memb_Data[key])
for bin_index_memb in bins_MEMB.keys():
    bins_MEMB[bin_index_memb] = np.mean(bins_MEMB[bin_index_memb]) if bins_MEMB[bin_index_memb] else 0

Plot_Count(Count_data, directory)
# plot_statistics_over_time(Count_data)

Plot_Thickness(Thickness_data, directory, bins_MEMB,)
# Plot_raw_data(Raw_Thickness_data, directory, 10, Memb_Data)

