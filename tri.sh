#!/bin/sh

# Set frequency and duration
FREQUENCY=10000
DURATION=5

# Directory containing the sorting scripts
SORT_DIR="/Tri"

# Loop through each sorting script in the directory
for sort_script in "$SORT_DIR"/*.py; do
  # Extract the base name of the script without the directory and extension
  base_name=$(basename "$sort_script" .py)
  
  # Run the script.sh with specified frequency and duration on each sorting script
  output_file="${base_name}.txt"
  echo "Running $sort_script and saving output to $output_file..."
  
  sudo ./script.sh -f "$FREQUENCY" -t "$DURATION" "$sort_script" > "$output_file"
  
  # Check if the execution was successful
  if [ $? -eq 0 ]; then
    echo "Execution of $sort_script completed successfully, output saved in $output_file"
  else
    echo "Execution of $sort_script failed"
  fi
done
