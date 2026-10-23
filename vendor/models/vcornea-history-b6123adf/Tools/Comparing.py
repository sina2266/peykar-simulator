import os
import csv
import matplotlib.pyplot as plt
import seaborn as sns
import fnmatch
import pandas as pd
import glob
import numpy as np



def extract_parameters(content):
    """Extract parameters and their values from the file content."""
    parameters = {}
    lines = content.strip().split("\n")
    for line in lines:
        parts = line.split('=')
        if len(parts) == 2:
            param_name = parts[0].strip()
            param_value = parts[1].strip()
            try:
                param_value = eval(param_value)
            except:
                pass
            parameters[param_name] = param_value
    return parameters

def compare_multiple_files(file_paths):
    all_parameters = {}
    for file_path in file_paths:
        with open(file_path, 'r') as f:
            content = f.read()
        params = extract_parameters(content)
        label = params['rep_folder'].split('/')[-2]
        for param, value in params.items():
            if param not in all_parameters:
                all_parameters[param] = {}
            all_parameters[param][label] = value
    return all_parameters

def read_csv_file(file_path):
    return pd.read_csv(file_path)


def collect_files(directory):
    """Collect paths to Parameters.py, Creation, Destruction, and Count files from Comb# directories."""
    parameter_files = []
    creation_files = []
    destruction_files = []
    count_files = []
    total_growth_files = []    

    mismatches_log = []  # To keep track of directories with missing or extra files
    
    for root, dirs, files in os.walk(directory):
        for dir_name in dirs:
            if "Comb" in dir_name:
                dir_path = os.path.join(root, dir_name, "0", "Simulation")

                file_patterns = {
                    'Parameters.py': parameter_files,
                    'Creation_of_mass_*.csv': creation_files,
                    'Destruction_of_mass_*.csv': destruction_files,
                    'cell_count_*.csv': count_files,
                    'growth_*.csv': total_growth_files
                }

                all_files_found = True  # Assume that all files are found until proven otherwise
                for pattern, file_list in file_patterns.items():
                    files_in_dir = glob.glob(os.path.join(dir_path, pattern))

                    # Check if there's exactly one matching file
                    if len(files_in_dir) == 1:
                        file_list.append(files_in_dir[0])
                    else:
                        all_files_found = False
                        mismatches_log.append(f"Found {len(files_in_dir)} files matching {pattern} in {dir_name}")

                if not all_files_found:
                    # Reset the file lists for this directory to ensure no incorrect files are used
                    for _, file_list in file_patterns.items():
                        if file_list and os.path.dirname(file_list[-1]) == dir_path:
                            file_list.pop()

    # Print out the mismatches
    if mismatches_log:
        print("The following mismatches were found:")
        for log_entry in mismatches_log:
            print(log_entry)

    return parameter_files, creation_files, destruction_files, count_files, total_growth_files


def save_to_csv(filepath, data, specific_params=None):
    """Save the parameter data to a CSV file."""
    with open(filepath, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        
        # If specific parameters are provided, filter the data, else use all parameters
        if specific_params:
            data = {k: v for k, v in data.items() if k in specific_params}
        
        # Write the header row
        headers = ['Comb'] + list(data.keys())
        writer.writerow(headers)
        
        # Get all Comb labels
        comb_labels = list(next(iter(data.values())).keys())        
        # Write each row
        for label in comb_labels:
            row = [label] + [data[param].get(label, '') for param in data]
            writer.writerow(row)

def plot_dataframe(dataframe, param_name, path):
    # Extract necessary data directly from the DataFrame
    param_values = dataframe[param_name].tolist()
    egf_comb_values = dataframe['EGF_BASAL_HalfMaxValue'].tolist()
    density_comb_values = dataframe['DensityBASAL_HalfMaxValue'].tolist()
    beta_comb_values = dataframe['BASAL_beta_EGF'].tolist()
    count_means = dataframe['Ensemble_Count_Mean'].tolist()
    count_stds = dataframe['Ensemble_Count_Std'].tolist()
    creation_means = dataframe['Ensemble_Creation_Mean'].tolist()
    creation_stds = dataframe['Ensemble_Creation_Std'].tolist()
    destruction_means = dataframe['Ensemble_Destruction_Mean'].tolist()
    destruction_stds = dataframe['Ensemble_Destruction_Std'].tolist()

    # Plotting with error bars
    plt.figure(figsize=(10, 6))
    plt.errorbar(param_values, count_means, yerr=count_stds, label='Count Mean', marker='o', capsize=5, linestyle='none')
    plt.errorbar(param_values, creation_means, yerr=creation_stds, label='Creation Mean', marker='s', capsize=5, linestyle='none')
    plt.errorbar(param_values, destruction_means, yerr=destruction_stds, label='Destruction Mean', marker='^', capsize=5, linestyle='none')

    if param_name == 'EGF_BASAL_HalfMaxValue':
        plt.title(f'Values of EGF_BASAL with Density_basal={density_comb_values[0]} and Beta_basal={beta_comb_values[0]}')
        title = (f'Values of EGF_BASAL with Density_basal={density_comb_values[0]} and Beta_basal={beta_comb_values[0]}')
    elif param_name == 'DensityBASAL_HalfMaxValue':
        plt.title(f'Values of Density_BASAL with EGF_basal={egf_comb_values[0]} and Beta_basal={beta_comb_values[0]}')
        title = (f'Values of Density_BASAL with EGF_basal={egf_comb_values[0]} and Beta_basal={beta_comb_values[0]}')
    elif param_name == 'BASAL_beta_EGF':
        plt.title(f'Values of Beta_BASAL with EGF_basal={egf_comb_values[0]} and Density_basal={density_comb_values[0]}')
        title = (f'Values of Beta_BASAL with EGF_basal={egf_comb_values[0]} and Density_basal={density_comb_values[0]}')

    # plt.title(f'Mean Values vs {param_name} with Std. Deviation')
    plt.xlabel(param_name)
    plt.ylabel('Mean Value')
    plt.legend()
    plt.grid(True)
    save_plot(title, path)
    # plt.show()

def plot_dataframe_with_subplots(dataframe, param_name, path):
    # Extract data
    param_values = dataframe[param_name].tolist()
    egf_comb_values = dataframe['EGF_BASAL_HalfMaxValue'].tolist()
    density_comb_values = dataframe['DensityBASAL_HalfMaxValue'].tolist()
    beta_comb_values = dataframe['BASAL_beta_EGF'].tolist()
    count_initial = dataframe['Ensemble_Initial_Value'].tolist()
    count_final = dataframe['Ensemble_Final_Value'].tolist()
    count_means = dataframe['Ensemble_Count_Mean'].tolist()
    count_stds = dataframe['Ensemble_Count_Std'].tolist()
    creation_means = dataframe['Ensemble_Creation_Mean'].tolist()
    creation_stds = dataframe['Ensemble_Creation_Std'].tolist()
    destruction_means = dataframe['Ensemble_Destruction_Mean'].tolist()
    destruction_stds = dataframe['Ensemble_Destruction_Std'].tolist()

    # Create figure and axes
    fig, (ax1, ax2) = plt.subplots(nrows=2, ncols=1, figsize=(10, 8), sharex=True, gridspec_kw={'height_ratios': [2, 1], 'hspace': 0.05})

    # Plot on the top subplot (for larger values)
    ax1.errorbar(param_values, count_means, yerr=count_stds, label='Wing Count Mean', marker='o', capsize=5, color='b', linestyle='none')
    ax1.scatter(param_values, count_initial, label='Wing Count Initial', marker='o', color='r', alpha=0.5)
    ax1.scatter(param_values, count_final, label='Wing Count Final', marker='o', color='g',  alpha=0.5)
    # ax1.set_ylim(8, 20)  # set the limits for y-axis (adjust as needed)
    ax1.grid(True)
    ax1.legend()
    ax1.set_ylabel('Mean Wing Cell Count')

    # Plot on the bottom subplot (for smaller values)
    ax2.errorbar(param_values, creation_means, yerr=creation_stds, label='Volume Creation', marker='s', capsize=5, color='g', linestyle='none')
    ax2.errorbar(param_values, destruction_means, yerr=destruction_stds, label='Volume Destruction ', marker='^', capsize=5, color='r', linestyle='none')
    ax2.set_ylim(-1, 1)  # set the limits for y-axis (adjust as needed)
    ax2.grid(True)
    ax2.legend()
    ax2.set_xlabel(param_name)
    ax2.set_ylabel('Mean Volume')

    # Hide the spines between the two plots
    ax1.spines['bottom'].set_visible(False)
    ax2.spines['top'].set_visible(False)
    ax1.xaxis.tick_top()
    ax1.tick_params(labeltop=False)
    ax2.xaxis.tick_bottom()

    # Adjust title position
    if param_name == 'EGF_BASAL_HalfMaxValue':
        fig.suptitle(f'Values of EGF_BASAL_HalfMaxValue with DensityBASAL_HalfMaxValue={density_comb_values[0]} and BASAL_beta_EGF={beta_comb_values[0]}', y=0.90)
        title = (f'Values of EGF_BASAL_HalfMaxValue with DensityBASAL_HalfMaxValue={density_comb_values[0]} and BASAL_beta_EGF={beta_comb_values[0]}')
    elif param_name == 'DensityBASAL_HalfMaxValue':
        fig.suptitle(f'Values of DensityBASAL_HalfMaxValue with EGF_BASAL_HalfMaxValue={egf_comb_values[0]} and BASAL_beta_EGF={beta_comb_values[0]}', y=0.90)
        title = (f'Values of DensityBASAL_HalfMaxValue with EGF_BASAL_HalfMaxValue={egf_comb_values[0]} and BASAL_beta_EGF={beta_comb_values[0]}')
    elif param_name == 'BASAL_beta_EGF':
        fig.suptitle(f'Values of BASAL_beta_EGF with EGF_BASAL_HalfMaxValue={egf_comb_values[0]} and DensityBASAL_HalfMaxValue={density_comb_values[0]}', y=0.90)
        title = (f'Values of BASAL_beta_EGF with EGF_BASAL_HalfMaxValue={egf_comb_values[0]} and DensityBASAL_HalfMaxValue={density_comb_values[0]}')
    # elif param_name == 'EGF_STEM_HalfMaxValue':
    #     fig.suptitle(f'Values of EGF_STEM with Density_stem={density_comb_values[0]} and Beta_stem={beta_comb_values[0]}', y=0.90)
    # elif param_name == 'DensitySTEM_HalfMaxValue':
    #     fig.suptitle(f'Values of Density_STEM with EGF_stem={egf_comb_values[0]} and Beta_stem={beta_comb_values[0]}', y=0.90)
    # elif param_name == 'STEM_beta_EGF':
    #     fig.suptitle(f'Values of Beta_STEM with EGF_stem={egf_comb_values[0]} and Density_stem={density_comb_values[0]}', y=0.90)
    plt.tight_layout()
    save_plot(title, path)
    # plt.show()

def save_plot(title, path):
    # Make the title safe for use as a filename
    if title:
        filename = title.replace(' ', '_').replace('=', '_').replace('/', '_') + '.png'
    else:
        filename = 'plot.png'
    # filepath = os.path.join(r'C:\Users\joelv\OneDrive\Desktop\Plots from data', filename)
    filepath = os.path.join(path, filename)  
    plt.savefig(filepath, bbox_inches='tight')
    print(f"Plot saved as: {filepath}")

# def plot_heatmap(dataframe, path, specific_params=None):
#     # Pivot the dataframe to get the heatmap structure
#     heatmap_data = dataframe.pivot(index='EGF_BASAL_HalfMaxValue', columns='DensityBASAL_HalfMaxValue', values='Ensemble_Count_Mean')
    
#     plt.figure(figsize=(40, 8))
#     sns.heatmap(heatmap_data, cmap='viridis', annot=False, vmax=180, vmin=100, fmt=".2f", cbar_kws={'label': 'Ensemble_Count_Mean'})
    
#     plt.title("Average Growth (Ensemble_Count_Mean) for different EGF_BASAL_HalfMaxValue and DensityBASAL_HalfMaxValue")
#     title = "Average Growth (Ensemble_Count_Mean) for different EGF_BASAL_HalfMaxValue and DensityBASAL_HalfMaxValue"
#     save_plot(title, path)
#     # plt.show()

def plot_heatmap(dataframe, path, index_col, columns_col, values_col):
    # Pivot the dataframe to get the heatmap structure
    heatmap_data = dataframe.pivot(index=index_col, columns=columns_col, values=values_col)
    
    plt.figure(figsize=(40, 8))
    
    # Adding colorbar label using cbar_kws argument
    sns.heatmap(heatmap_data, cmap='viridis', annot=False, 
                cbar_kws={'label': values_col})
    
    plt.title(f"Average Growth ({values_col}) for different {index_col} and {columns_col}")
    title = f"Average Growth {values_col} for different {index_col} and {columns_col}"
    # plt.title(plot_title)
    save_plot(title, path)

def plot_heatmaps_for_each_diffcoef(dataframe, path, unique_diffcoefs,diffname, index_col_basal,index_col_stem, columns_col_basal,columns_col_stem, values_color):
    """
    Plot heatmaps for each unique value of EGF_SUPERDiffCoef, side by side for basal and stem.
    """
    for diffcoef in unique_diffcoefs:
        filtered_df = dataframe[dataframe[f'{diffname}'] == diffcoef]
        
        # Plot for Basal
        plt.figure(figsize=(20, 8))
        plt.subplot(1, 2, 1)
        heatmap_data_basal = filtered_df.pivot(index=index_col_basal, columns=columns_col_basal, values=values_color)
        sns.heatmap(heatmap_data_basal, cmap='viridis', annot=False, cbar_kws={'label': values_color})
        plt.title(f" {values_color} for Basal -{index_col_basal} and {columns_col_basal} at EGF_SUPERDiffCoef = {diffcoef}")

        # Plot for Stem
        plt.subplot(1, 2, 2)
        heatmap_data_stem = filtered_df.pivot(index=index_col_stem, columns=columns_col_stem, values=values_color)
        sns.heatmap(heatmap_data_stem, cmap='viridis', annot=False, cbar_kws={'label': values_color})
        plt.title(f" {values_color} for Stem -{index_col_stem} and {columns_col_stem} at EGF_SUPERDiffCoef = {diffcoef}")

        # Save the plot
        title = f"EGF_SUPERDiffCoef {diffcoef} Basal{index_col_basal} vs {columns_col_basal} and Stem{index_col_stem} vs {columns_col_stem}"
        save_plot(title, path)
        plt.close()



if __name__ == "__main__":
    # Path to the directory with Comb# folders
    directory_path = "/u/jvanin/vCornea/Processed_Data/Output_Version_10/Toguchi_L27_02282024_1125"
    
    # Collecting all Parameters.py file paths
    # file_paths = collect_parameter_files(directory_path)
    param_files, creation_files, destruction_files, count_files, growth_files = collect_files(directory_path)
    data = []

    for p,c,g,l,t in zip(param_files, count_files, creation_files, destruction_files, growth_files):     
            all_param_values = compare_multiple_files([p])        
            df_param = pd.DataFrame(all_param_values)

            df_count = read_csv_file(c)
            ensenble_count = df_count[(df_count['Time'] >= 25) & (df_count['Time'] <= 60)] # scale this to 25-100hours
            ensenble_intial_value = ensenble_count['Wing'].values[0]
            ensenble_final_value = ensenble_count['Wing'].values[-1]            
            ensenble_count_mean = ensenble_count['Wing'].mean()
            ensenble_count_std = ensenble_count['Wing'].std()

            df_creation = read_csv_file(g)
            ensenble_creation = df_creation[(df_creation['Time'] >= 250) & (df_creation['Time'] <= 600)] # scale this to 250-1000MCS
            ensenble_creation_mean = ensenble_creation['Volume'].mean()
            ensenble_creation_std = ensenble_creation['Volume'].std()

            df_destruction = read_csv_file(l)
            ensenble_destruction = df_destruction[(df_destruction['Time'] >= 250) & (df_destruction['Time'] <= 600)]
            ensenble_destruction_mean = ensenble_destruction['Volume'].mean()
            ensenble_destruction_std = ensenble_destruction['Volume'].std()

            df_growth = read_csv_file(t)
            df_growth['TG_total'] = df_growth['Basal Average Total'] + df_growth['Stem Average Total']
            ensenble_growth = df_growth[(df_growth['Time'] >= 25) & (df_growth['Time'] <= 60)]
            ensenble_growth_TG_BASAL_mean = ensenble_growth['Basal Average Total'].mean()
            ensenble_growth_TG_BASAL_std = ensenble_growth['Basal Average Total'].std()
            ensemble_growth_TG_STEM_mean = ensenble_growth['Stem Average Total'].mean()
            ensemble_growth_TG_STEM_std = ensenble_growth['Stem Average Total'].std()
            ensemble_growth_TG_total_mean = df_growth['TG_total'].mean()
            ensemble_growth_TG_total_std = df_growth['TG_total'].std()

            
            combination_data = {
            'Comb': df_param.index[0],  # Assuming 'rep_folder' contains the combination identifier
            'Path': p,
            'Path_count': c,
            'Path_creation': g,
            'Path_destruction': l,
            'Path_growth': t,            
            'EGF_BASAL_HalfMaxValue': df_param['EGF_BASAL_HalfMaxValue'].values[0],
            'DensityBASAL_HalfMaxValue': df_param['DensityBASAL_HalfMaxValue'].values[0],
            'BASAL_beta_EGF': df_param['BASAL_beta_EGF'].values[0],
            'EGF_STEM_HalfMaxValue': df_param['EGF_STEM_HalfMaxValue'].values[0],
            'DensitySTEM_HalfMaxValue': df_param['DensitySTEM_HalfMaxValue'].values[0],
            'STEM_beta_EGF': df_param['STEM_beta_EGF'].values[0], 
            'EGF_SUPERDiffCoef': df_param['EGF_SUPERDiffCoef'].values[0],
            'EGF_GlobalDecay': df_param['EGF_GlobalDecay'].values[0],
            'EGF_FieldUptakeBASAL': df_param['EGF_FieldUptakeBASAL'].values[0],
            'EGF_FieldUptakeSTEM' : df_param['EGF_FieldUptakeSTEM'].values[0],
            'EGF_FieldUptakeSuper': df_param['EGF_FieldUptakeSuper'].values[0],
            'EGF_FieldUptakeWing' : df_param['EGF_FieldUptakeWing'].values[0],           
            'Ensemble_Initial_Value': ensenble_intial_value,
            'Ensemble_Final_Value': ensenble_final_value,           
            'Ensemble_Count_Mean': ensenble_count_mean,
            'Ensemble_Creation_Mean': ensenble_creation_mean,
            'Ensemble_Destruction_Mean': ensenble_destruction_mean,
            'Ensemble_Count_Std': ensenble_count_std,
            'Ensemble_Creation_Std': ensenble_creation_std,
            'Ensemble_Destruction_Std': ensenble_destruction_std,
            'Ensemble_Growth_TG_BASAL_Mean': ensenble_growth_TG_BASAL_mean,
            'Ensemble_Growth_TG_BASAL_Std': ensenble_growth_TG_BASAL_std,
            'Ensemble_Growth_TG_STEM_Mean': ensemble_growth_TG_STEM_mean,
            'Ensemble_Growth_TG_STEM_Std': ensemble_growth_TG_STEM_std,
            'Ensemble_Growth_TG_Total_Mean': ensemble_growth_TG_total_mean,
            'Ensemble_Growth_TG_Total_Std': ensemble_growth_TG_total_std
            }        
            # Append this dictionary to the data list
            data.append(combination_data)

    df = pd.DataFrame(data)     
    

    uniqueBasal_EGF_halfmax_value = df['EGF_BASAL_HalfMaxValue'].unique()
    uniqueBasal_Density_halfmax_value = df['DensityBASAL_HalfMaxValue'].unique()
    uniqueBasal_beta_value = df['BASAL_beta_EGF'].unique()
    uniqueStem_EGF_halfmax_value = df['EGF_STEM_HalfMaxValue'].unique()
    uniqueStem_Density_halfmax_value = df['DensitySTEM_HalfMaxValue'].unique()
    uniqueStem_beta_value = df['STEM_beta_EGF'].unique()
    uniqueEGF_SUPERDiffCoef = df['EGF_SUPERDiffCoef'].unique()
    uniqueEGF_GlobalDecay = df['EGF_GlobalDecay'].unique()
    

    # Specify the columns you're interested in
    # columns_to_check = ['DensityBASAL_HalfMaxValue', 'BASAL_beta_EGF', 'EGF_STEM_HalfMaxValue', 'DensitySTEM_HalfMaxValue', 'STEM_beta_EGF']

    Basal_EGF_halfmax_value_filter = df.sort_values(by=['DensityBASAL_HalfMaxValue', 'BASAL_beta_EGF', 'EGF_STEM_HalfMaxValue', 'DensitySTEM_HalfMaxValue', 'STEM_beta_EGF', 'EGF_SUPERDiffCoef', 'EGF_GlobalDecay'])
    Basal_Density_halfmax_value_filter = df.sort_values(by=['EGF_BASAL_HalfMaxValue', 'BASAL_beta_EGF', 'EGF_STEM_HalfMaxValue', 'DensitySTEM_HalfMaxValue', 'STEM_beta_EGF', 'EGF_SUPERDiffCoef', 'EGF_GlobalDecay'])
    Basal_beta_value_filter = df.sort_values(by=['EGF_BASAL_HalfMaxValue', 'DensityBASAL_HalfMaxValue', 'EGF_STEM_HalfMaxValue', 'DensitySTEM_HalfMaxValue', 'STEM_beta_EGF', 'EGF_SUPERDiffCoef', 'EGF_GlobalDecay'])
    Stem_EGF_halfmax_value_filter = df.sort_values(by=['EGF_BASAL_HalfMaxValue', 'DensityBASAL_HalfMaxValue', 'BASAL_beta_EGF', 'DensitySTEM_HalfMaxValue', 'STEM_beta_EGF', 'EGF_SUPERDiffCoef', 'EGF_GlobalDecay'])
    Stem_Density_halfmax_value_filter = df.sort_values(by=['EGF_BASAL_HalfMaxValue', 'DensityBASAL_HalfMaxValue', 'BASAL_beta_EGF', 'EGF_STEM_HalfMaxValue', 'STEM_beta_EGF', 'EGF_SUPERDiffCoef', 'EGF_GlobalDecay'])
    Stem_beta_value_filter = df.sort_values(by=['EGF_BASAL_HalfMaxValue', 'DensityBASAL_HalfMaxValue', 'BASAL_beta_EGF', 'EGF_STEM_HalfMaxValue', 'DensitySTEM_HalfMaxValue', 'EGF_SUPERDiffCoef', 'EGF_GlobalDecay'])
    EGF_SUPERDiffCoef_filter = df.sort_values(by=['EGF_BASAL_HalfMaxValue', 'DensityBASAL_HalfMaxValue', 'BASAL_beta_EGF', 'EGF_STEM_HalfMaxValue', 'DensitySTEM_HalfMaxValue', 'STEM_beta_EGF', 'EGF_GlobalDecay'])
    EGF_GlobalDecay_filter = df.sort_values(by=['EGF_BASAL_HalfMaxValue', 'DensityBASAL_HalfMaxValue', 'BASAL_beta_EGF', 'EGF_STEM_HalfMaxValue', 'DensitySTEM_HalfMaxValue', 'STEM_beta_EGF', 'EGF_SUPERDiffCoef'])
   
    # Single valeu plots 
    #plot_heatmap(df, directory_path, index_col='EGF_BASAL_HalfMaxValue', columns_col='DensityBASAL_HalfMaxValue', values_col='Ensemble_Count_Mean')
    #plot_heatmap(df, directory_path, index_col='EGF_BASAL_HalfMaxValue', columns_col='DensityBASAL_HalfMaxValue', values_col='Ensemble_Growth_TG_BASAL_Mean')
    #plot_heatmap(df, directory_path, index_col='EGF_BASAL_HalfMaxValue', columns_col='DensityBASAL_HalfMaxValue', values_col='Ensemble_Growth_TG_Total_Mean')

    # Filter for 10% deviation of the initial cell count
    filtered_df = df[(df['Ensemble_Count_Mean'] >= 140*0.9) & (df['Ensemble_Count_Mean'] <= 140*1.1)]
    
    filtered_df.to_csv(os.path.join(directory_path, 'Possible_values_10pc_countMean.csv'), index=False)
    df.to_csv(os.path.join(directory_path, 'All_values.csv'), index=False)
    
    # Filter the columns
    filtered_parameters = [
        'EGF_GlobalDecay', 'EGF_SUPERDiffCoef', 'STEM_beta_EGF', 'DensitySTEM_HalfMaxValue',
        'EGF_STEM_HalfMaxValue', 'BASAL_beta_EGF', 'DensityBASAL_HalfMaxValue', 'EGF_BASAL_HalfMaxValue'        
    ]
    df_filtered = filtered_df[filtered_parameters]

    # Save to a new CSV for running longger simulations
    df_filtered.to_csv(os.path.join(directory_path,"Combs_for_long_sims.csv"), index=False)

    # Using the function to plot based on especific parameters for both Stem and Basal
    #plot_heatmaps_for_each_diffcoef(df, directory_path, uniqueEGF_SUPERDiffCoef,'EGF_SUPERDiffCoef',
                                   #  'EGF_BASAL_HalfMaxValue', 'EGF_STEM_HalfMaxValue',
                                    #   'DensityBASAL_HalfMaxValue', 'DensitySTEM_HalfMaxValue',
                                      #   'Ensemble_Count_Mean')
    
    #plot_heatmaps_for_each_diffcoef(df, directory_path, uniqueEGF_SUPERDiffCoef,'EGF_SUPERDiffCoef',
                                    # 'EGF_BASAL_HalfMaxValue', 'EGF_STEM_HalfMaxValue',
                                     #  'DensityBASAL_HalfMaxValue', 'DensitySTEM_HalfMaxValue',
                                     #    'Ensemble_Growth_TG_Total_Mean')

    # B_EGF_0= Basal_EGF_halfmax_value_filter.iloc[105:120] 
    # B_Density_0= Basal_Density_halfmax_value_filter.iloc[105:120]
    # B_beta_0= Basal_beta_value_filter.iloc[0:9]
    # S_EGF_0= Stem_EGF_halfmax_value_filter.iloc[105:120]
    # S_Density_0= Stem_Density_halfmax_value_filter.iloc[0:15]
    # S_beta_0= Stem_beta_value_filter.iloc[0:9]

    # plot_dataframe(B_EGF_0, 'EGF_BASAL_HalfMaxValue')
    # plot_dataframe(B_Density_0, 'DensityBASAL_HalfMaxValue')
    # plot_dataframe(B_beta_0, 'BASAL_beta_EGF')
    # plot_dataframe(S_EGF_0, 'EGF_STEM_HalfMaxValue')
    # plot_dataframe(S_Density_0, 'DensitySTEM_HalfMaxValue')
    # plot_dataframe(S_beta_0, 'STEM_beta_EGF')

    # plot_dataframe_with_subplots(B_EGF_0, 'EGF_BASAL_HalfMaxValue')
    # plot_dataframe_with_subplots(B_Density_0, 'DensityBASAL_HalfMaxValue')
    # plot_dataframe_with_subplots(B_beta_0, 'BASAL_beta_EGF')
    # plot_dataframe_with_subplots(S_EGF_0, 'EGF_STEM_HalfMaxValue')
    # plot_dataframe_with_subplots(S_Density_0, 'DensitySTEM_HalfMaxValue')
    # plot_dataframe_with_subplots(S_beta_0, 'STEM_beta_EGF')
   
    # plot_dataframe(B_EGF_0)
    # plot_dataframe_with_subplots(B_EGF_0)


    