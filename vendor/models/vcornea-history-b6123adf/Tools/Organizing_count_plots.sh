#!/bin/bash

tobe_copied="/u/jvanin/vCornea/Processed_Data/Output_Version_4/Output_version_v4_long_5param/Filtered"
central_location="/u/jvanin/vCornea/Processed_Data/Output_Version_4/Output_version_v4_long_5param/Filtered/Filtered_count_plots"
central_thickness="/u/jvanin/vCornea/Processed_Data/Output_Version_4/Output_version_v4_long_5param/Filtered/Filtered_Thickness_plots"

# Copy all cell_count_*.png files from Output_v3_expl to Output_v3_expl_count_cell_plots
find "$tobe_copied"/ -type f -name "cell_count_*.png" -exec cp {} "$central_location"/ \;
find "$tobe_copied"/ -type f -name "thickness_*.png" -exec cp {} "$central_thickness"/ \;

# Directories for columns and rows
columns_dir="$central_location/columns"
rows_dir="$central_location/rows"

mkdir -p "$columns_dir"
mkdir -p "$rows_dir"

for file in $central_location/cell_count_*.png; do
    # Extract the DensityBASAL_HalfMaxValue and EGF_BASAL_HalfMaxValue values from the filename
    density_value=$(echo "$file" | grep -oP "DensityBASAL_HalfMaxValue=\K[0-9.]+")
    egf_value=$(echo "$file" | grep -oP "EGF_BASAL_HalfMaxValue=\K[0-9.]+")
    
    # Create directories for these values if they don't exist
    mkdir -p "$columns_dir/$density_value"
    mkdir -p "$rows_dir/$egf_value"
    
    # Copy the file to its corresponding directory in columns and rows
    cp "$file" "$columns_dir/$density_value/"
    cp "$file" "$rows_dir/$egf_value/"
done