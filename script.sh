#!/bin/sh

FREQUENCY=10
DURATION=5

usage() {
  echo "Usage: $0 [-f frequency] [-t duration] <command_to_monitor>"
  exit 1
}

if [ "$(id -u)" -ne 0 ]; then
  echo "Please run as root (with sudo)"
  exit 1
fi

while getopts "f:t:" opt; do
  case $opt in
    f) FREQUENCY="$OPTARG" ;;
    t) DURATION="$OPTARG" ;;
    *) usage ;;
  esac
done
shift $((OPTIND -1))

if [ "$#" -eq 0 ]; then
  usage
fi

MONITOR_CMD="$*"
OUTPUT_FILE="/tmp/data.csv"

echo "Starting the data generation command with frequency $FREQUENCY Hz and duration $DURATION seconds..."
sudo ./mojitos/bin/mojitos -f "$FREQUENCY" -o "$OUTPUT_FILE" -r &
MOJITOS_PID=$!
echo "MojitO/S started with PID $MOJITOS_PID"

# Run the command
$MONITOR_CMD > /dev/null 2>&1 &
CMD_PID=$!
echo "Monitoring command '$MONITOR_CMD' with PID $CMD_PID"

# Wait for the specified duration
sleep "$DURATION"

# Kill the monitored command and MojitO/S
echo "Stopping monitored command and MojitO/S after $DURATION seconds..."
kill "$CMD_PID" 2>/dev/null
sudo kill "$MOJITOS_PID" 2>/dev/null
echo "Data collection complete. File saved to $OUTPUT_FILE"

if [ ! -f "$OUTPUT_FILE" ]; then
  echo "Error: $OUTPUT_FILE not found."
  exit 1
fi

header=$(head -n 1 "$OUTPUT_FILE")
echo "Calculating statistics for each sensor in: $header"
echo

awk -v header="$header" '
  BEGIN {
    split(header, colnames)
    start_time = 0
    end_time = 0
  }
  NR == 2 { start_time = $1 }  
  { end_time = $1 }             

  NR > 1 { 
    for (i = 2; i <= NF; i++) {
      sum[i] += $i
      if (NR == 2 || $i < min[i]) min[i] = $i
      if (NR == 2 || $i > max[i]) max[i] = $i
    }
    count++
  }
  END {
    duration = end_time - start_time
    printf "Total Duration: %.2f seconds\n\n", duration

    printf "%-12s %-10s %-10s %-10s %-10s\n", "Sensor", "Min", "Max", "Mean", "Sum"
    for (i = 2; i <= NF; i++) {
      mean = sum[i] / count
      printf "%-12s %-10.2f %-10.2f %-10.2f %-10.2f\n", colnames[i], min[i], max[i], mean, sum[i]
    }
  }
' "$OUTPUT_FILE"

rm -f "$OUTPUT_FILE"
echo "Temporary file removed."
