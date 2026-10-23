import os
import glob
import pandas as pd
import matplotlib.pyplot as plt
import argparse
from sklearn.linear_model import LinearRegression
import numpy as np
import shutil
from filelock import FileLock
import sys
from collections import defaultdict
import csv



class DataGatherer:
    def __init__(self, base_folder):       
            self.base_folder = base_folder

    def gather_csv_files_from_folders(self, folders_to_process):
        all_files = []
        for folder in folders_to_process:
            comb_folder = os.path.join(self.base_folder, folder)
            all_files.extend(glob.glob(os.path.join(comb_folder, "**", "*.csv"), recursive=True))
        return all_files
    
    def extract_params_from_path(self, path): 
        parts = path.split(os.path.sep)
        comb_part = [part for part in parts if part.startswith("Comb")]
        if not comb_part:
            print(f"Problematic path: {path}")
            raise ValueError("'Comb#' not found in path")
        comb_idx = comb_part[0][4:]
        replicate_idx = parts[parts.index(comb_part[0]) + 1]
        return comb_idx, replicate_idx
    
    def organize_csv_files(self, csv_files):    
        organized = {}
        for file in csv_files:
            comb, _ = self.extract_params_from_path(file)
            base_name = os.path.basename(file)
            key = (comb, base_name)
            if key not in organized:
                organized[key] = []
            organized[key].append(file)
        return organized
    
    def parquet_gather_files_from_folder(self, folders_to_process):
        all_files = []
        for folder in folders_to_process:
            comb_folder = os.path.join(self.base_folder, folder)
            all_files.extend(glob.glob(os.path.join(comb_folder, "**", "*.parquet"), recursive=True))
        return all_files
    
    def organize_parquet_files(self, parquet_files):    
        organized = {}
        for file in parquet_files:
            comb, _ = self.extract_params_from_path(file)
            base_name = os.path.basename(file)
            key = (comb, base_name)
            if key not in organized:
                organized[key] = []
            organized[key].append(file)
        return organized
    
    def extract_parameters(self, content):
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
    
    def compare_multiple_files(self, file_paths):
        all_parameters = {}
        for file_path in file_paths:
            with open(file_path, 'r') as f:
                content = f.read()
            params = self.extract_parameters(content)
            label = params['rep_folder'].split('/')[-2]
            for param, value in params.items():
                if param not in all_parameters:
                    all_parameters[param] = {}
                all_parameters[param][label] = value
        return all_parameters
    
    def collect_files(self, folders_to_process):
        """Collect paths to Parameters.py, Creation, Destruction, and Count files from Comb# directories."""
        parameter_files = [] 
        
        for folder in folders_to_process:
            comb_folder = os.path.join(self.base_folder, folder)
            parameter_files.extend(glob.glob(os.path.join(comb_folder,"0", "Simulation","Parameters.py")))                    
                    
        all_params_Combs = []
        for p in parameter_files:
            all_param_values = self.compare_multiple_files([p])
            df_param = pd.DataFrame(all_param_values)
            combination_data = {
            'Comb': df_param.index[0],  # Assuming 'rep_folder' contains the combination identifier
            'EGF_BASAL_HalfMaxValue': df_param['EGF_BASAL_HalfMaxValue'].values[0],
            'DensityBASAL_HalfMaxValue': df_param['DensityBASAL_HalfMaxValue'].values[0],
            'BASAL_beta_EGF': df_param['BASAL_beta_EGF'].values[0],
            'EGF_STEM_HalfMaxValue': df_param['EGF_STEM_HalfMaxValue'].values[0],
            'DensitySTEM_HalfMaxValue': df_param['DensitySTEM_HalfMaxValue'].values[0],
            'STEM_beta_EGF': df_param['STEM_beta_EGF'].values[0],
            'EGF_SUPERDiffCoef': df_param['EGF_SUPERDiffCoef'].values[0],
            }
            all_params_Combs.append(combination_data)
        return all_params_Combs
   
class Classifier(DataGatherer):   
    CELL_COUNT_DIFF_THRESHOLD = {
        "Superficial": 20,
        "Wing": 55,
        "Basal": 12,
        "Stem": 7 }   
    LIMBUS_DIFF_THRESHOLD = 7
    PERIPHERY_DIFF_THRESHOLD = 7

    def __init__(self, organized_data):
        self.organized_data = organized_data

    @staticmethod
    def check_directionality(data, column, slope_threshold=0.01):
        X = np.arange(len(data)).reshape(-1, 1)
        y = data[column].values
        reg = LinearRegression().fit(X, y)
        slope = reg.coef_[0]        
        if slope > slope_threshold:
            return 'increasing', slope
        elif slope < -slope_threshold:
            return 'decreasing', slope
        else:
            return 'stable', slope
                
    def classify_data(self, files, data_type):
        flag_fail = False
        reasons = []                
        all_data = [pd.read_csv(f) for f in files]        
        combined_data = pd.concat(all_data).groupby(level=0).mean()

        if data_type == 'cell':
            columns = ['Superficial', 'Wing', 'Basal', 'Stem']
            initial_values = {col: combined_data[col].iloc[0] for col in columns}
            final_values = {col: combined_data[col].iloc[-1] for col in columns}
            diff_values = {col: final_values[col] - initial_values[col] for col in columns}
            trends = {col: self.check_directionality(combined_data, col) for col in columns}
            for col, (trend, slope) in trends.items():
                if trend in ['increasing', 'decreasing']:
                    reasons.append(f'{col} trend is {trend} ({slope})')
                    flag_fail = True
                else:
                    reasons.append(f'{col} trend is stable ({slope})')
            for col, diff in diff_values.items():
                if abs(diff) > self.CELL_COUNT_DIFF_THRESHOLD[col]:
                    reasons.append(f'Difference in {col} exceeded threshold {abs(diff)} > {self.CELL_COUNT_DIFF_THRESHOLD[col]}')
                    flag_fail = True
                else:
                    reasons.append(f'Difference in {col} is within threshold {abs(diff)} <= {self.CELL_COUNT_DIFF_THRESHOLD[col]}')
        elif data_type == 'thickness':
            initial_limbus = combined_data['Limbus Avg Thickness'].iloc[0]
            initial_periphery = combined_data['Periphery Avg Thickness'].iloc[0]
            final_limbus = combined_data['Limbus Avg Thickness'].iloc[-1]
            final_periphery = combined_data['Periphery Avg Thickness'].iloc[-1]
            diff_limbus = final_limbus - initial_limbus
            diff_periphery = final_periphery - initial_periphery
            limbus_trend, limbus_slope = self.check_directionality(combined_data, 'Limbus Avg Thickness')
            periphery_trend, periphery_slope = self.check_directionality(combined_data, 'Periphery Avg Thickness')
            if limbus_trend in ['increasing', 'decreasing']:
                reasons.append(f'Limbus trend is {limbus_trend} ({limbus_slope})')
                flag_fail = True
            else:
                reasons.append(f'Limbus trend is stable ({limbus_slope})')
            if periphery_trend in ['increasing', 'decreasing']:
                reasons.append(f'Periphery trend is {periphery_trend} ({periphery_slope})')
                flag_fail = True
            else:
                reasons.append(f'Periphery trend is stable ({periphery_slope})')
            if abs(diff_limbus) > self.LIMBUS_DIFF_THRESHOLD:
                reasons.append(f'Difference in Limbus exceeded threshold ({abs(diff_limbus)}) > {self.LIMBUS_DIFF_THRESHOLD}')
                flag_fail = True
            else:
                reasons.append(f'Difference in Limbus is within threshold ({abs(diff_limbus)}) <= {self.LIMBUS_DIFF_THRESHOLD}')
            if abs(diff_periphery) > self.PERIPHERY_DIFF_THRESHOLD:
                reasons.append(f'Difference in Periphery exceeded threshold ({abs(diff_periphery)}) > {self.PERIPHERY_DIFF_THRESHOLD}')
                flag_fail = True
            else:
                reasons.append(f'Difference in Periphery is within threshold ({abs(diff_periphery)}) <= {self.PERIPHERY_DIFF_THRESHOLD}')
        elif data_type == 'growth':
            pass
        else:
            pass
            # raise ValueError("Invalid data type provided. Must be 'cell_count' or 'thickness'.", data_type) 
        
        comb_dir = os.path.dirname(os.path.dirname(os.path.dirname(files[0])))
        
        classification = 'Success' if not flag_fail else 'Failure'
        return classification, reasons, comb_dir
    
    def classify_comb_file_type(self, comb_id, file_type, files):             
        classification = self.classify_data(files, file_type)       
        return {
            'comb_id': comb_id,
            'file_type': file_type,
            'classification': classification
        }
    
    def classify_all(self):        
        results = []        
        for (comb_id, file_type), files in self.organized_data.items():            
            base_name_without_extension = os.path.splitext(file_type)[0]            
            actual_file_type = base_name_without_extension.split('_')[0]            
            comb_file_type_results = self.classify_comb_file_type(comb_id, actual_file_type, files)
            results.append(comb_file_type_results)                    
        return results
    
    def generate_combined_report(self, results):
        # Extract all unique comb_ids
        comb_ids = set(item['comb_id'] for item in results)

        # Prepare results dictionary
        classification_results = {}

        for cid in comb_ids:
            cell_data = [item for item in results if item['comb_id'] == cid and item['file_type'] == 'cell'][0]
            thickness_data = [item for item in results if item['comb_id'] == cid and item['file_type'] == 'thickness'][0]
            
            # Classify the comb_id based on the given criteria
            if 'Success' in cell_data['classification'][0] and 'Success' in thickness_data['classification'][0]:
                classification = 'Success'
            elif 'Success' in cell_data['classification'][0] or 'Success' in thickness_data['classification'][0]:
                classification = 'Partial'
            else:
                classification = 'Fail'
            
            # Build the report for the comb_id
            report = ["--- Classification Report for {} ---".format(cid)]
            report.append("\nCell Count Classification:")
            report.append(f"Result: {cell_data['classification'][0]}")
            for reason in cell_data['classification'][1]:
                report.append(f"- {reason}")
            report.append("\nThickness Classification:")
            report.append(f"Result: {thickness_data['classification'][0]}")
            for reason in thickness_data['classification'][1]:
                report.append(f"- {reason}")
            
            # Unique filename for each comb_id
            unique_filename = f"report_for_comb_{cid}.txt"
            report_path = os.path.join(cell_data['classification'][2], unique_filename)

            with open(report_path, 'w') as file:
                file.write('\n'.join(report))
            
            # Store classification and path in the dictionary
            classification_results[cid] = {
                'classification': classification,
                'report_path': report_path
            }
        return classification_results

class Plotter(DataGatherer):
    """
    The `Plotter` class provides functionality to visualize and save plots based on datasets of different parameters.
    
    Attributes:
    - organized_files: A dictionary of file basenames mapped to corresponding file lists.
    - output_folder: The directory where the generated plots will be saved.
    
    Methods:
    - plot_specific_parameters: Plots specific parameters from the provided data.
    - plot_data: Generates plots based on the parameters in the provided datasets, 
                 categorizing the data by 'thickness', 'growth', or other types. 
                 For each category, it plots individual data, computes the mean, 
                 and visualizes the mean values. The plots are then saved to the specified output directory.
    """
    def __init__(self, organized_csv_files, organized_parquet_files, organized_parameters, output_folder):
        """
        Initialize the Plotter instance with organized files and output folder.

        Parameters:
        - organized_csv_files (dict): Dictionary of files organized by their categorization.
        - output_folder (str): Directory where the plots will be saved.
        """
        self.organized_csv_files = organized_csv_files
        self.organized_parquet_files = organized_parquet_files
        self.organized_parameters = organized_parameters
        self.output_folder = output_folder
        # self.base_folder = DataGatherer.base_folder
    
    def plot_specific_parameters(self, data, ax, y_columns, title):
        """
        Plot specified parameters on the given axis.

        Parameters:
        - data (pd.DataFrame): DataFrame containing data to plot.
        - ax (matplotlib.Axes): Axes object where the plot will be generated.
        - y_columns (list): List of columns to plot.
        - title (str): Title of the plot.
        """
        # Plot each specified column with an alpha value
        for col in y_columns:
            if col in data.columns:
                ax.plot(data["Time"], data[col], label=col, alpha=0.5)  # each replicate
        
        # Plot the mean of specified columns
        mean_data = data[y_columns].mean(axis=1)
        ax.plot(data["Time"], mean_data, 'k--', label='Mean', linewidth=2)  # mean line
        ax.set_title(title)
        ax.legend()
    
    def plot_data(self):
        """
        Plot the organized data based on the type of the file (thickness, growth, etc.).
        """
        try:
            # Get the values of the parameters used for the current combination
            # params_Combs = self.collect_files(directory=self.output_folder)
            params_Combs = self.organized_parameters
            # egf_comb_values = params_Combs['EGF_BASAL_HalfMaxValue'].tolist()
            # density_comb_values = params_Combs['DensityBASAL_HalfMaxValue'].tolist()
            # beta_comb_values = params_Combs['BASAL_beta_EGF'].tolist()
            
            colormap = plt.cm.tab10.colors

            # Loop through each category of organized files
            for (comb, file_basename), file_list in self.organized_csv_files.items():
                all_data = []
                y_columns = []
                y_colors = {}

                # Decide on the plotting structure based on file type
                if 'thickness' in file_basename:
                    fig, axs = plt.subplots(3, 1, figsize=(10, 15))
                elif 'growth' in file_basename:
                    fig, axs = plt.subplots(4, 2, figsize=(15, 20))
                elif 'cell' in file_basename:
                    plt.figure(figsize=(10, 6))
                else:
                    pass

                # Loop through each file in the current category
                for file in file_list:
                    # print(file)            
                    data = pd.read_csv(file)
                    all_data.append(data)
                    x = data["Time"]
                    y_columns = [col for col in data.columns if col != "Time"]

                    # Assign colors to columns
                    for idx, col in enumerate(y_columns):
                        if col not in y_colors:
                            y_colors[col] = colormap[idx % len(colormap)]

                    # Plotting for thickness data
                    if 'thickness' in file_basename:
                        # Columns to be plotted for thickness data
                        limbus_params = ['Limbus Avg Thickness', 'Limbus Left Thickness', 'Limbus Right Thickness']
                        periphery_params = ['Periphery Avg Thickness', 'Periphery Left Thickness', 'Periphery Right Thickness']
                        
                        for ax, params in zip(axs, [y_columns, limbus_params, periphery_params]):
                            for col in params:
                                if col in y_colors:
                                    ax.plot(x, data[col], color=y_colors[col], alpha=0.2)                                    

                    # Plotting for growth data
                    elif 'growth' in file_basename:
                        # Columns to be plotted for growth data
                        stem_params_group = [
                            ['Stem Average Total', 'Stem Average EGF', 'Stem Average Density'],
                            ['Stem Average EGF', 'Stem Average EGFseen', 'Stem Average Total'],
                            ['Stem Average Density', 'Stem Average Pressure', 'Stem Average Total'],
                            ['Stem Average TargetVolume', 'Stem Average Total']
                        ]
                        basal_params_group = [
                            [param.replace('Stem', 'Basal') for param in group]
                            for group in stem_params_group
                        ]

                        # Plotting the individual stem data with transparency
                        for data in all_data:
                            for i, stem_params in enumerate(stem_params_group):
                                ax = axs[i][0]  # Stem on the left column
                                for col in stem_params:
                                    ax.plot(x, data[col], color=y_colors[col], alpha=0.2)
                                if i in [1, 2, 3]:
                                    ax.set_yscale("log")

                        # Plotting the individual basal data with transparency
                        for data in all_data:
                            for i, basal_params in enumerate(basal_params_group):
                                ax = axs[i][1]  # Basal on the right column
                                for col in basal_params:
                                    ax.plot(x, data[col], color=y_colors[col], alpha=0.2)
                                if i in [1, 2, 3]:
                                    ax.set_yscale("log")

                    # Plotting for any other type of data
                    elif 'cell' in file_basename:
                        for idx, col in enumerate(y_columns):
                            plt.plot(x, data[col], color=colormap[idx % len(colormap)], alpha=0.2)
                    else:
                        pass
                # Compute the mean data outside the file loop
                mean_data = pd.concat(all_data).groupby(level=0).mean()

                current_comb_data = next(item for item in params_Combs if item["Comb"] == f'Comb{comb}')
                egf_comb_value = current_comb_data['EGF_BASAL_HalfMaxValue']
                density_comb_value = current_comb_data['DensityBASAL_HalfMaxValue']
                beta_comb_value = current_comb_data['BASAL_beta_EGF']
                egf_comb_value_stem = current_comb_data['EGF_STEM_HalfMaxValue']
                density_comb_value_stem = current_comb_data['DensitySTEM_HalfMaxValue']
                beta_comb_value_stem = current_comb_data['STEM_beta_EGF']
                efg_super_diff_coef = current_comb_data['EGF_SUPERDiffCoef']


                # Plotting mean data for 'thickness' files
                if 'thickness' in file_basename:
                    for ax, params, title in zip(axs, [y_columns, limbus_params, periphery_params], ["All Parameters", "Limbus", "Periphery"]):
                        for col in params:
                            if col in y_colors:
                                ax.plot(x, mean_data[col], label=col, color=y_colors[col])
                        ax.set_title(title)
                        ax.legend()
                    
                    plt.xlabel("Time (Hour)")
                    plt.ylabel("Center of mass position difference (voxels)")
                    comb, rep = self.extract_params_from_path(file_list[0])
                    # plt.suptitle(file_basename + f' Combination {comb} Values of EGF_BASAL_HalfMaxValue={egf_comb_value} DensityBASAL_HalfMaxValue={density_comb_value} BASAL_beta_EGF={beta_comb_value}', y=1.02)
                    plt.suptitle(f"{file_basename} Combination {comb}\n"
                        f"Values of EGF_BASAL_HalfMaxValue={egf_comb_value}\n"
                        f"DensityBASAL_HalfMaxValue={density_comb_value}\n"
                        f"BASAL_beta_EGF={beta_comb_value}\n"
                        f"EGF_STEM_HalfMaxValue={egf_comb_value_stem}\n"
                        f"DensitySTEM_HalfMaxValue={density_comb_value_stem}\n"
                        f"STEM_beta_EGF={beta_comb_value_stem}\n"
                        f"EGF_SUPERDiffCoef={efg_super_diff_coef}")
                    plt.tight_layout(h_pad=2.5)
                    plot_filename = file_basename + f' Combination {comb} Values of EGF_BASAL_HalfMaxValue={egf_comb_value} DensityBASAL_HalfMaxValue={density_comb_value} BASAL_beta_EGF={beta_comb_value} EGF_STEM_HalfMaxValue={egf_comb_value_stem} DensitySTEM_HalfMaxValue={density_comb_value_stem} STEM_beta_EGF={beta_comb_value_stem} EGF_SUPERDiffCoef={efg_super_diff_coef}.png'
                    comb_output_folder = os.path.join(self.output_folder, f'Comb{comb}')                   
                    plt.savefig(os.path.join(comb_output_folder, plot_filename), bbox_inches='tight') 
                    plt.close()
                # Plotting mean data for 'growth' files
                elif 'growth' in file_basename:
                    # Plotting the mean data for 'Stem'
                    for i, stem_params in enumerate(stem_params_group):
                        ax = axs[i][0]  # Stem on the left column
                        for col in stem_params:
                            ax.plot(x, mean_data[col], label=col, color=y_colors[col])
                        ax.legend()
                        ax.set_title("Stem - " + ", ".join(stem_params))
                        if i in [1, 2, 3]:  # for second, third, and fourth rows
                            ax.set_yscale("log")

                    # Plotting the mean data for 'Basal'
                    for i, basal_params in enumerate(basal_params_group):
                        ax = axs[i][1]  # Basal on the right column
                        for col in basal_params:
                            ax.plot(x, mean_data[col], label=col, color=y_colors[col])
                        ax.legend()
                        ax.set_title("Basal - " + ", ".join(basal_params))
                        if i in [1, 2, 3]:  # for second, third, and fourth rows
                            ax.set_yscale("log")
                    plt.tight_layout()
                    plot_filename = file_basename + f' Combination {comb} Values of EGF_BASAL_HalfMaxValue={egf_comb_value} DensityBASAL_HalfMaxValue={density_comb_value} BASAL_beta_EGF={beta_comb_value} EGF_STEM_HalfMaxValue={egf_comb_value_stem} DensitySTEM_HalfMaxValue={density_comb_value_stem} STEM_beta_EGF={beta_comb_value_stem} EGF_SUPERDiffCoef={efg_super_diff_coef}.png'
                    comb_output_folder = os.path.join(self.output_folder, f'Comb{comb}')                   
                    plt.savefig(os.path.join(comb_output_folder, plot_filename), bbox_inches='tight') 
                    plt.close()
                # Plotting mean data for any other type of files
                elif 'cell' in file_basename:
                    for idx, col in enumerate(y_columns):
                        plt.plot(x, mean_data[col], label=col, color=colormap[idx % len(colormap)])

                    # Compute the mean of the entire 'cell' dataset
                    # overall_mean = mean_data[y_columns].mean(axis=1)
                    # plt.plot(x, overall_mean, 'k--', label="Overall Mean")  # Plotting the overall mean as a dashed line
                    # wing_cells_mean = mean_data["Wing"].mean(axis=0)
                    # plt.plot(x, wing_cells_mean, 'k--', label="Wing TimeSeries Mean")  # Plotting the mean as a dashed line
                    wing_cells_mean_value = mean_data["Wing"].mean(axis=0)
                    wing_cells_mean_array = [wing_cells_mean_value] * len(x)
                    plt.plot(x, wing_cells_mean_array, 'k--', label="Wing Time-Series Mean")  # Plotting the mean as a dashed line

                    plt.xlabel("Time (Hour)")
                    plt.ylabel("Cell Count")
                    comb, rep = self.extract_params_from_path(file_list[0])
                    # plt.title(file_basename + f' Combination {comb} Values of EGF_BASAL_HalfMaxValue={egf_comb_value} DensityBASAL_HalfMaxValue={density_comb_value} BASAL_beta_EGF={beta_comb_value}')                    
                    plt.title(f"{file_basename} Combination {comb}\n"
                        f"Values of EGF_BASAL_HalfMaxValue={egf_comb_value}\n"
                        f"DensityBASAL_HalfMaxValue={density_comb_value}\n"
                        f"BASAL_beta_EGF={beta_comb_value}\n"
                        f"EGF_STEM_HalfMaxValue={egf_comb_value_stem}\n"
                        f"DensitySTEM_HalfMaxValue={density_comb_value_stem}\n"
                        f"STEM_beta_EGF={beta_comb_value_stem}\n"
                        f"EGF_SUPERDiffCoef={efg_super_diff_coef}")
                    plt.legend()
                    plot_filename = file_basename + f' Combination {comb} Values of EGF_BASAL_HalfMaxValue={egf_comb_value} DensityBASAL_HalfMaxValue={density_comb_value} BASAL_beta_EGF={beta_comb_value} EGF_STEM_HalfMaxValue={egf_comb_value_stem} DensitySTEM_HalfMaxValue={density_comb_value_stem} STEM_beta_EGF={beta_comb_value_stem} EGF_SUPERDiffCoef={efg_super_diff_coef}.png'
                    comb_output_folder = os.path.join(self.output_folder, f'Comb{comb}')                   
                    plt.savefig(os.path.join(comb_output_folder, plot_filename), bbox_inches='tight') 
                    plt.close()
                else:
                    pass
                print(file_basename)

                # Saving the plot to the specified directory
                # plot_filename = file_basename + f' Combination {comb} Values of EGF_BASAL_HalfMaxValue={egf_comb_value} DensityBASAL_HalfMaxValue={density_comb_value} BASAL_beta_EGF={beta_comb_value}.png'
                # comb_output_folder = os.path.join(self.output_folder, f'Comb{comb}')
                # # if not os.path.exists(comb_output_folder):  # Create the combination folder if it doesn't exist
                # #     os.makedirs(comb_output_folder)
                # plt.savefig(os.path.join(comb_output_folder, plot_filename), bbox_inches='tight')
            
        except FileNotFoundError:
            sys.stderr.write(f"Error: File {file} not found.")
        except pd.errors.EmptyDataError:
            sys.stderr.write(f"Error: File {file} contains no data.")
        except Exception as e:
            sys.stderr.write(f"An error occurred: {e}")
        finally:
            plt.close()

    def plot_parquet(self):        
        
        try:
            markers = ['o', 's', '^', 'D', 'p', '*', 'v', '<', '>', 'H', '+', 'x', '|', '_']

            for (comb, file_basename), file_list in self.organized_parquet_files.items():
                all_data = []           
           
                for file in file_list:                
                    df_parquet = pd.read_parquet(file)
                    all_data.append(df_parquet)
                    comb, rep = self.extract_params_from_path(file)

                    if 'mass' in file_basename:
                        
                        # event_types = df_parquet["Event Type"].unique()
                        # Epected data from event_types
                            # ['Basal Growth Vol' 'Stem Growth Vol' 'Superficial Loss Vol'
                            #  'Superficial Slough Vol' 'Basal Before Differentiation Vol'
                            #  'Wing After Differentiation Vol' 'Stem Before Differentiation Vol'
                            #  'Basal After Differentiation Vol' 'Wing Before Differentiation Vol'
                            #  'Superficial After Differentiation Vol' 'Basal Before Mitosis Vol'
                            #  'Basal After Mitosis Vol' 'Stem Before Mitosis Vol'
                            #  'Stem After Mitosis Vol']

                        # # Wing After Differentiation Vol SUBSET
                        wing_after_diff_subset = df_parquet[df_parquet["Event Type"] == 'Wing After Differentiation Vol']           # len 2737   unique 2618 time 2347
                        # Basal after differentiation Vol SUBSET
                        # basal_after_diff_subset = df_parquet[df_parquet["Event Type"] == 'Basal After Differentiation Vol']           
                        # # Superficial After Differentiation Vol SUBSET
                        super_after_diff_subset = df_parquet[df_parquet["Event Type"] == 'Superficial After Differentiation Vol']   # len 2618   unique 2737 time 2177

                        # Basal growth single cell SUBSET
                        basal_growth_subset = df_parquet[df_parquet["Event Type"] == 'Basal Growth Vol']                            # len 551966 unique 1527 time 14602
                        # Stem growth single cell SUBSET
                        stem_growth_subset = df_parquet[df_parquet["Event Type"] == 'Stem Growth Vol']                              # len 55424  unique 170  time 14602
                        # Superficial loss single cell SUBSET
                        superficial_loss_subset = df_parquet[df_parquet["Event Type"] == 'Superficial Loss Vol']                    # len 632946 unique 2777 time 14481
                        # Superficial slough single cell SUBSET
                        superficial_slough_subset = df_parquet[df_parquet["Event Type"] == 'Superficial Slough Vol'] 
                        
                        # # Basal After Mitosis Vol
                        # basal_after_mitosis_subset = df_parquet[df_parquet["Event Type"] == 'Basal After Mitosis Vol']              # 
                        # # Stem After Mitosis Vol
                        # stem_after_mitosis_subset = df_parquet[df_parquet["Event Type"] == 'Stem After Mitosis Vol']                 
                        
                        # Generate dictionaries with unique time points and all the cells values for that time point
                        # dict_time_basal_growth = self.generate_time_volume_dict(basal_growth_subset)
                        dict_time_basal_growth_optimized = self.generate_time_volume_dict_optimized(basal_growth_subset)                      
                        # dict_time_stem_growth = self.generate_time_volume_dict(stem_growth_subset)
                        dict_time_stem_growth_optimized = self.generate_time_volume_dict_optimized(stem_growth_subset)                      
                        # dict_time_superficial_loss = self.generate_time_volume_dict(superficial_loss_subset)
                        dict_time_superficial_loss_optimized = self.generate_time_volume_dict_optimized(superficial_loss_subset)                       
                        # dict_time_superficial_slough = self.generate_time_volume_dict(superficial_slough_subset)
                        dict_time_superficial_slough_optimized = self.generate_time_volume_dict_optimized(superficial_slough_subset)                        
                        # dict_time_wing_after_diff = self.generate_time_volume_dict(wing_after_diff_subset)
                        dict_time_wing_after_diff_optimized = self.generate_time_volume_dict_optimized(wing_after_diff_subset)
                        # dict_time_basal_after_diff = self.generate_time_volume_dict(basal_after_diff_subset)  
                        # dict_time_basal_after_diff_optimized = self.generate_time_volume_dict_optimized(basal_after_diff_subset)                        
                        # dict_time_super_after_diff = self.generate_time_volume_dict(super_after_diff_subset)
                        dict_time_super_after_diff_optimized = self.generate_time_volume_dict_optimized(super_after_diff_subset)

                        # Summing the volumes for growth combined 
                        # Initialize a defaultdict with int to automatically handle missing keys with a default value of 0
                        dict_sum_time_growth = defaultdict(int)

                        # Add the volumes 
                        for time, volume in dict_time_basal_growth_optimized.items():
                            dict_sum_time_growth[time] += volume
                        for time, volume in dict_time_stem_growth_optimized.items():                            
                            dict_sum_time_growth[time] += volume
                        for time, volume in dict_time_wing_after_diff_optimized.items():
                            dict_sum_time_growth[time] += volume
                        # for time, volume in dict_time_basal_after_diff_optimized.items():
                        #     dict_sum_time_growth[time] += volume

                        dict_sum_growth_difference = self.compute_difference_from_previous(dict_sum_time_growth)                        

                        # Initialize a defaultdict with int to automatically handle missing keys with a default value of 0                        
                        dict_sum_time_combined_loss = dict_time_superficial_loss_optimized.copy()
                         # Subtract the superficial slough volumes
                        for time, volume in dict_time_superficial_slough_optimized.items():
                            dict_sum_time_combined_loss[time] = dict_sum_time_combined_loss.get(time, 0) - volume
                        for time, volume in dict_time_super_after_diff_optimized.items():
                            dict_sum_time_combined_loss[time] = dict_sum_time_combined_loss.get(time, 0) - volume

                        dict_sum_loss_difference = self.compute_difference_from_previous(dict_sum_time_combined_loss) 

                        # Initialize the difference dictionary with values from the growth dictionary                        
                        dict_growth_vs_loss = dict_sum_growth_difference.copy()
                        # Subtract the superficial loss values                        
                        for time, volume in dict_sum_loss_difference.items():
                            # If the time point exists in the growth dictionary, subtract the loss value                        
                            dict_growth_vs_loss[time] = (dict_growth_vs_loss.get(time, 0) - volume)                           

                        # Computing cumulative values
                        cum_growth_dict = self.compute_cumulative_sum(dict_sum_growth_difference)
                        cum_loss_dict = self.compute_cumulative_sum(dict_sum_loss_difference)
                        cum_diff_dict = self.compute_cumulative_sum(dict_growth_vs_loss)

                       
                    # Plotting cumulative values                       

                        mean_difference = sum(dict_growth_vs_loss.values()) / len(dict_growth_vs_loss)                        
                        mean_grow = sum(dict_sum_growth_difference.values()) / len(dict_sum_growth_difference)
                        mean_Sup = sum(dict_sum_loss_difference.values()) / len(dict_sum_loss_difference)
                        npstd_difference = np.std(list(dict_growth_vs_loss.values()))
                        npstd_growth = np.std(list(dict_sum_growth_difference.values()))
                        npstd_Sup = np.std(list(dict_sum_loss_difference.values()))
                        # print(mean_difference)
                        # print(npstd_difference)
                        # print(mean_grow)
                        # print(npstd_growth)
                        # print( mean_Sup)
                        # print(npstd_Sup)

                        # # Plotting the data                        
                        # fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(20, 7))
                        # # Cummulative Superficial Loss
                        # times, volumes = zip(*sorted(cum_loss_dict.items()))
                        # ax1.plot(times, volumes, label='Cummulative Superficial Loss')
                        # # Cummulative Growth Combined
                        # times, volumes = zip(*sorted(cum_growth_dict.items()))
                        # ax1.plot(times, volumes, label='Cummulative Growth Combined', alpha=0.75)
                        # # Growth - Loss
                        # times, volumes = zip(*sorted(cum_diff_dict.items()))
                        # ax1.plot(times, volumes, label='abs(Cummulative Loss - Growth)', alpha=0.75)                        
                        
                        # # Superficial Loss
                        times, volumes = zip(*sorted(dict_sum_loss_difference.items()))
                        self.save_times_volumes_to_csv(os.path.join(os.path.join(self.output_folder, f'Comb{comb}/{rep}/Simulation'),
                                                                    f'Destruction_of_mass_Comb{comb}_Rep{rep}.csv'), times, volumes)
                        # ax2.plot(times, volumes, label='Superficial Loss')
                        # # Growth Combined
                        times, volumes = zip(*sorted(dict_sum_growth_difference.items()))
                        self.save_times_volumes_to_csv(os.path.join(os.path.join(self.output_folder, f'Comb{comb}/{rep}/Simulation'),
                                                                     f'Creation_of_mass_Comb{comb}_Rep{rep}.csv'), times, volumes)
                        # ax2.plot(times, volumes, label='Growth Combined', alpha=0.5)
                        # # Growth - Loss
                        times, volumes = zip(*sorted(dict_growth_vs_loss.items()))                        
                        self.save_times_volumes_to_csv(os.path.join(os.path.join(self.output_folder, f'Comb{comb}/{rep}/Simulation'),
                                                                                 f'Growth-Loss_Comb{comb}_Rep{rep}.csv'), times, volumes)
                        # ax2.plot(times, volumes, label='(Growth - Loss)', alpha=0.5)

                        # # Assuming mean_difference and mean_Sup are defined somewhere in your code
                        # ax2.axhline(y=mean_difference, color='r', linestyle='--', label='Mean Difference')
                        # ax2.axhline(y=mean_Sup, color='purple', linestyle='--', label='Mean Superficial Loss')

                        # Add legends to each plot
                        # ax1.legend() 
                        # ax1.set_xlabel('Time (MCS)')
                        # ax1.set_ylabel('Volume (voxels)')
                        # ax1.set_title(f' Commulative Cells Volumes Comb{comb} rep{rep} ')                        
                        # ax1.grid(True)
                        
                        # ax2.legend()
                        # ax2.set_xlabel('Time (MCS)')
                        # ax2.set_ylabel('Volume (voxels)')
                        # ax2.set_title(f' Volume Created and Destroyed Comb{comb} rep{rep} ')                        
                        # ax2.grid(True)
                        # plt.tight_layout()                        

                        # # Saving the plot to the specified directory
                        # plot_filename = file_basename + f' Combination {comb} rep {rep}.png'
                        # comb_output_folder = os.path.join(self.output_folder, f'Comb{comb}/{rep}/Simulation')                        
                        # plt.savefig(os.path.join(comb_output_folder, plot_filename), bbox_inches='tight')
                        # plt.close()
                        # csv_filename = f'Growth-Loss Comb{comb} Rep{rep}.csv'
                        # csv_output_folder = os.path.join(self.output_folder, f'Comb{comb}/{rep}/')
                        # csv_filepath = os.path.join(csv_output_folder, csv_filename)
                        # with open(csv_filepath, mode='w', newline='') as file:
                        #     writer = csv.writer(file)
                        #     # Write header
                        #     writer.writerow(["Time", "Volume"])
                        #     # Write data
                        #     for time, volume in dict_growth_vs_loss.items():
                        #         writer.writerow([time, volume])                        

        except FileNotFoundError:
            sys.stderr.write(f"Error: File {file} not found.")
        return

    def generate_time_volume_dict(self, df_subset):
            dict_time_volume = {}
            for index, row in df_subset.iterrows():
                time = row['Time']
                volume = row['Volume']
                if time in dict_time_volume:
                    dict_time_volume[time].append(volume)
                else:
                    dict_time_volume[time] = [volume]
            return dict_time_volume
    
    def generate_time_volume_dict_optimized(self, df_subset):
        # Group by 'Time' and sum the 'Volume' for each time
        grouped = df_subset.groupby('Time')['Volume'].sum()
        # Convert the Series to a dictionary
        dict_time_volume = grouped.to_dict()
        return dict_time_volume
    
    def compute_cumulative_sum(self, data_dict):
        # Sort the dictionary based on the time steps (keys)
        sorted_times = sorted(data_dict.keys())
        
        cumulative_sum = 0
        cumulative_dict = {}
        
        for time in sorted_times:
            cumulative_sum += data_dict[time]
            cumulative_dict[time] = cumulative_sum

        return cumulative_dict
    
    def save_times_volumes_to_csv(self, filename, times, volumes):
       
        with open(filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(["Time", "Volume"])  # header row
            for time, volume in zip(times, volumes):
                writer.writerow([time, volume])
 
    def compute_difference_from_previous(self, data_dict):       
        # Sort items by the time key
        sorted_items = sorted(data_dict.items())
        
        # Compute differences
        differences = {}
        for i, (time, volume) in enumerate(sorted_items):
            if i == 0:
                # No previous data for the first timestep, so set difference to 0
                differences[time] = 0
            else:
                differences[time] = (volume - sorted_items[i-1][1])
        
        return differences
    
    # def generate_time_volume_change_dict(self, df_subset, event_type='Superficial Loss Vol'):
    #     # A nested dictionary structure: {time: {cell_id: volume}}
    #     dict_time_cell_volume = {}
    #     for index, row in df_subset.iterrows():
    #         time = row['Time']
    #         cell_id = row['Cell ID']
    #         volume = row['Volume']
            
    #         if time not in dict_time_cell_volume:
    #             dict_time_cell_volume[time] = {}
            
    #         dict_time_cell_volume[time][cell_id] = volume

    #     # Calculate the volume change
    #     dict_time_volume_change = {}
    #     sorted_times = sorted(dict_time_cell_volume.keys())
    #     for i, time in enumerate(sorted_times):
    #         if i == 0:  # Skip the first time point as we don't have a previous point for comparison
    #             continue
    #         prev_time = sorted_times[i - 1]
    #         dict_time_volume_change[time] = {}
    #         for cell_id in dict_time_cell_volume[time]:
    #             current_volume = dict_time_cell_volume[time].get(cell_id, 0)
    #             previous_volume = dict_time_cell_volume[prev_time].get(cell_id, 0)
                
    #             if event_type == 'Superficial Loss Vol':
    #                 # volume_change = current_volume - previous_volume
    #                 volume_change = previous_volume - current_volume
    #             else:  # Assuming the event_type is 'growth' for now
    #                 # volume_change = previous_volume - current_volume
    #                 volume_change = current_volume - previous_volume

    #             dict_time_volume_change[time][cell_id] = volume_change

    #     # Convert the nested dictionary to the desired format: {time: [volume_changes]}
    #     dict_time_volume_list = {time: list(volumes.values()) for time, volumes in dict_time_volume_change.items()}
        
    #     return dict_time_volume_list

class FileOrganizer:
    
    def move_comb_folders(self, classification_results):
        # Go through each comb_id and its results
        for cid, data in classification_results.items():
            classification = data['classification']
            comb_dir = os.path.dirname(data['report_path'])

            # Create the target directory based on classification
            target_dir = os.path.join(os.path.dirname(os.path.dirname(data['report_path'])), classification)
            if not os.path.exists(target_dir):
                with FileLock(f"{target_dir}.lock"):
                    # Make the directory if it doesn't exist after obtaining the lock
                    if not os.path.exists(target_dir):
                        os.makedirs(target_dir)
            # Compute the destination path
            destination_path = os.path.join(target_dir, os.path.basename(comb_dir))
            # Move the comb folder to the target directory
            with FileLock(f"{comb_dir}.lock"):
                if os.path.exists(comb_dir):
                    shutil.move(comb_dir, destination_path)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process specific Comb# folders.")
    parser.add_argument("folders", nargs='+', help="List of Comb# folders to process")
    args = parser.parse_args()    
    # print(args)
    print(args.folders)
    base_path = "/u/jvanin/vCornea/Processed_Data/Output_Version_6/Long_year"
    
    gatherer = DataGatherer(base_folder=base_path)
    
    param_files = gatherer.collect_files(args.folders)
    csv_files = gatherer.gather_csv_files_from_folders(args.folders)
    organized_files = gatherer.organize_csv_files(csv_files)    
    parquet_files = gatherer.parquet_gather_files_from_folder(args.folders)
    parquet_organized_files = gatherer.organize_parquet_files(parquet_files)
       
    
    classifier = Classifier(organized_data=organized_files)
    results = classifier.classify_all()    
    classification = classifier.generate_combined_report(results)
    

    plotter = Plotter(organized_csv_files=organized_files, organized_parquet_files=parquet_organized_files, organized_parameters=param_files, output_folder=base_path)
    
    plotter.plot_data() 
    plotter.plot_parquet()

    # Organizing the folders
    # FileOrganizer().move_comb_folders(classification)
    

