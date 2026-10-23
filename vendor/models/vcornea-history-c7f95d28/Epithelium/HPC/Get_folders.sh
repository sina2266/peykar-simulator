#!/bin/bash

# Source directory where the folders are currently located
SOURCE_DIR="/u/jvanin/vCornea/Processed_Data/Output_Version_6/"

# Destination directory where the folders will be copied
DESTINATION_DIR="/u/jvanin/vCornea/Processed_Data/Output_Version_6/Filtered_sec"

# Array of folder names to be copied

declare -a FOLDERS=(
'Comb6'
'Comb18'
'Comb24'
'Comb47'
'Comb49'
'Comb81'
'Comb122'
'Comb137'
'Comb158'
'Comb175'
'Comb179'
'Comb187'
'Comb366'
'Comb380'
'Comb418'
'Comb477'
'Comb487'
'Comb553'
'Comb564'
'Comb580'
)

# Loop through each folder and copy it to the destination
for FOLDER in "${FOLDERS[@]}"; do
  cp -r "${SOURCE_DIR}${FOLDER}" "${DESTINATION_DIR}"
done

echo "All specified folders have been copied."