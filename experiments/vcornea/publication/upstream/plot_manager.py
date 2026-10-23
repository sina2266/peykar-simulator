import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
from pathlib import Path
from scipy.stats import probplot

class PlotManager:
   
    def __init__(self, Data_DIR, Stable_time, Max_time, SimTime_mcs, Out_DIR=None):
        self.data_dir = Path(Data_DIR)
        self.out_dir = Path(Out_DIR) if Out_DIR else self.data_dir
        self.script_dir = Path(__file__).parent

        # Define color codes and other constants
        self.color_codes = ['#55ffff','#0055ff', '#ffbe99', '#ff007f']
        self.stable_time = Stable_time 
        self.max_time = Max_time 
        self.SimTime_mcs = SimTime_mcs

        # Collect and load data
        self.Param_file, self.Count_file, self.Thickness_file = self.collect_files(Data_DIR)
        self.memb_bins = self.load_memb_data()         
        self.count_data = self.data_collection(self.Count_file)
        self.thickness_data = self.data_collection(self.Thickness_file)
    
    def collect_files(self, directory):  
        """
        Recursively collects data files from a directory based on filename patterns.

        This function searches through all subdirectories of the given directory
        for specific file types: Parameters.py, cell count CSVs, and two kinds
        of thickness data files (binned and raw).

        Args:
            directory (str or Path): The root directory to start the search from.

        Returns:
            tuple: A tuple containing four lists of file paths (as strings):
                - parameter_files
                - count_files
                - thickness_bins_files                
        """                   

        base_path = Path(directory)        
        parameter_files = [str(p) for p in base_path.rglob("Parameters.py")]
        count_files = [str(p) for p in base_path.rglob("cell_count_*.csv")]
        thickness_bins_files = [str(p) for p in base_path.rglob("thickness_rep_*.parquet")]


        return parameter_files, count_files, thickness_bins_files

    def data_collection(self, files):
        """
        Collect data from multiple files, supporting both CSV and Parquet formats.
        
        Args:
            file: List of file paths
            
        Returns:
            dict: Dictionary containing DataFrames with keys based on file path components
        """
        # Load data
        data_dict = {}       
        
        for file_path_str in files:
            # Create a Path object from the string path
            data_path = Path(file_path_str)
            file_extension = data_path.suffix.lower()

            try:
                # Read file based on its extension
                if file_extension == '.csv':
                    data = pd.read_csv(data_path)
                elif file_extension == '.parquet':
                    data = pd.read_parquet(data_path)
                else:
                    # Silently skip unsupported files, or you can print a warning
                    # print(f"Skipping unsupported file format: {file_extension}")
                    continue
                               
                grandparent_dir_name = data_path.parent.parent.name
                parent_dir_name = data_path.parent.name
                key = f"{grandparent_dir_name}_{parent_dir_name}"
                
                data_dict[key] = data

            except Exception as e:
                print(f"Error reading file {data_path}: {e}")
                continue

        return data_dict
    
    def load_memb_data(self):        
        memb_data = {}
        try:           
            memb_file = Path(self.script_dir).joinpath("membrane_height.csv")
            rows = pd.read_csv(memb_file, header=None)
            for index, row in rows.iterrows():
                memb_data[row[0]] = row[1]
                
            bin_size = 200 / 10        
            bin_indexes = list(range(10))
            bins_memb = {index: [] for index in bin_indexes}
            
            for key in memb_data.keys():           
                bin_index_memb = int(key // bin_size)          
                bins_memb[bin_index_memb].append(memb_data[key])
            for bin_index_memb in bins_memb.keys():
                bins_memb[bin_index_memb] = np.mean(bins_memb[bin_index_memb]) if bins_memb[bin_index_memb] else 0
        except Exception as e:
            print(f"Warning: Could not load membrane data: {str(e)}")            
            bins_memb = None
        return bins_memb
    
    def plot_count(self):
        # Data handling
        data_dict = self.count_data
        combined_data = pd.concat(data_dict.values())
        combined_data = combined_data[combined_data['Time'] <= self.max_time]
        combined_stable_data = combined_data[combined_data['Time'] >= self.stable_time]
        mean_data = combined_data.groupby('Time').mean().reset_index()

        # Create figure and axes
        fig, axes = plt.subplots(4, 2, figsize=(14, 12))
        axes[0, 0].axis('off')
        axes[0, 1].axis('off')
        axes[3, 0].axis('off')
        axes[3, 1].axis('off')

        ax_longitudinal = plt.subplot2grid((4, 2), (0, 0), colspan=2)  
        ax_qq_combined = plt.subplot2grid((4, 2), (3, 0), colspan=2, rowspan=1)

            # Plot the mean data on the longitudinal plot        
        for i, column in enumerate(mean_data.columns[1:5]):        
            ax_longitudinal.plot(mean_data["Time"]/24, mean_data[column], label=f'Mean {column}', color=self.color_codes[i], linewidth=2) 
        
            row = (i // 2) + 1
            col = i % 2            
            ax_hist = axes[row, col]

            # Combine all stable data for the current column
            all_stable_data = np.concatenate([data[(data['Time'] >= self.stable_time) & (data['Time'] <= self.max_time)][column].values for data in data_dict.values()])
            
            # Calculate bin edges
            bin_edges = np.arange(int(all_stable_data.min()) - 0.5, int(all_stable_data.max()) + 1.5, 1)

            # Plot individual distributions and calculate mean
            all_hists = []
            for Comb, data in data_dict.items():
                stable_data = data[(data['Time'] >= self.stable_time) & (data['Time'] <= self.max_time)]
                hist, _, _ = ax_hist.hist(stable_data[column], bins=bin_edges, color=self.color_codes[i], alpha=0.075)
                all_hists.append(hist)
                       
            ax_hist.grid(True, linewidth=0.1) 
            ax_hist.set_title(f'Variation of {column}-Cell Number in Time and Between Replicates')
            ax_hist.set_xlabel('Number of Cells per Time')
            ax_hist.set_ylabel('Frequency')           

            # Q-Q plot       
            (osm, osr), (slope, intercept, r) = probplot(combined_stable_data[column], dist="norm", plot=ax_qq_combined)
            
            lines = ax_qq_combined.get_lines()
            if len(lines) >= 2 * (i + 1):  # Ensure there are enough lines
                observed_line = lines[2 * i]
                expected_line = lines[2 * i + 1]
                # Style observed data points
                observed_line.set_color(self.color_codes[i])
                observed_line.set_alpha(0.25)
                observed_line.set_label(f'{column}')
                # Style expected line
                expected_line.set_color('black')
                expected_line.set_linestyle('dashed')
        expected_line.set_label(f'Normal Distribution')    
        ax_qq_combined.set_title('Combined Q-Q Plots')
        ax_qq_combined.grid(True, linewidth=0.1)
        ax_qq_combined.legend()
        data = data[data['Time'] <= self.max_time]

        for Comb, data in data_dict.items():        
            # Filter the data to include only times less than 720
            filtered_data = data[data['Time'] < self.max_time]  
            for i, column in enumerate(filtered_data.columns[0:4]):                     
                ax_longitudinal.plot(filtered_data['Time']/24, filtered_data[column], color=self.color_codes[i], alpha=0.075, linewidth=2)

        ax_longitudinal.axvline(self.stable_time/24, color='black', linestyle='dashed', linewidth=1, label="Stability Time")
        ax_longitudinal.legend(loc='center right')
        ax_longitudinal.grid(True, linewidth=0.1)        
        ax_longitudinal.set_title('Longitudinal Cell Count')
        ax_longitudinal.set_xlabel('Time (Days)')
        ax_longitudinal.set_ylabel('Number of Cells')

        plt.tight_layout()
        plt.subplots_adjust(hspace=0.40)
        plt.show()    
   
    def plot_thickness(self):
        data_dict = self.thickness_data
        bin_memb = self.memb_bins
       
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

            stable_bin_data = combined_data[(combined_data['Bin'] == bin_key) & (combined_data['Time'] >= self.stable_time)]

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
            
        ax[0].axvline(self.stable_time / 24, color='black', linestyle='dashed', linewidth=1, label="Stability Time")
        ax[0].set_title('Longitudinal Variation of Tissue Top Position Over Six Months')
        ax[0].set_xlabel('Time (Days)')
        ax[0].set_ylabel('Vertical Position (μm)')
        ax[0].legend(title='Bins', loc='lower right')
        ax[0].grid(True, linewidth=0.1)

        x_values = [val * 2 for val in bin_x.values()]  # Convert to full width in μm
        ax[1].plot(x_values, bin_actual, color='tab:red', linestyle='dotted', linewidth=1, label="Tissue Thickness")
        ax[1].plot(x_values, bin_relative, color='tab:blue', linestyle='dotted', linewidth=1, label="Tissue Top Position")

        ax[1].set_title('Stable Tissue Top Position and Thickness Across Sections')
        ax[1].set_xlabel('Horizontal Position (μm)')
        ax[1].set_ylabel('Vertical Position / Thickness (μm)')
        ax[1].legend(title='Bins', loc='center right')
        ax[1].grid(True, linewidth=0.1)

        plt.tight_layout()
        plt.subplots_adjust(hspace=0.20)
        # plt.savefig(f"{out_dir}/Thickness.png")
        plt.show()  
    

if __name__ == '__main__':
    slight_path= Path(__file__).resolve().parent.parent / "(Slight) 750_G/Comb0/"
    mild_path= Path(__file__).resolve().parent.parent / "(Mild) 1500_G/Comb1/"
    moderate_path= Path(__file__).resolve().parent.parent / "(Moderate) 2500_G/Comb1/"
    
    stem_only_path= Path(__file__).resolve().parent.parent / "STEM_ONLY_proper_thickness_6months/Comb0/"
   
    p_slight = PlotManager(Data_DIR=slight_path, Stable_time=5000/10, Max_time=(6*30*240)/10, SimTime_mcs=(6*30*240)/10)
    p_mild = PlotManager(Data_DIR=mild_path, Stable_time=5000/10, Max_time=(6*30*240)/10, SimTime_mcs=(6*30*240)/10)    
    p_moderate = PlotManager(Data_DIR=moderate_path, Stable_time=5000/10, Max_time=(6*30*240)/10, SimTime_mcs=(6*30*240)/10)
    
    p_stem_only = PlotManager(Data_DIR=stem_only_path, Stable_time=5000/10, Max_time=(6*30*240)/10, SimTime_mcs=(6*30*240)/10)
 
    p_stem_only.plot_count()      # Fig. 7
    p_stem_only.plot_thickness()  # Fig. 8

    p_slight.plot_count()          # Fig. 10
    p_mild.plot_count()            # Fig. 10
    p_moderate.plot_count()        # Fig. 10

