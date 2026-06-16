#!/bin/bash

# Define absolute or relative tracking variables
LOG_FILE="data/logs.txt"
ANALYZER_BIN="./log_analyzer"
REPORT_OUT="report.csv"

echo "🔄 [$(date)] Initializing automated background log evaluation..."

# 1. Safety Check: Verify the data stream and binary exist
if [ ! -f "$LOG_FILE" ]; then
    echo "⚠️ WARNING: Log file target missing. Awaiting incoming data streams..."
    exit 1
fi

if [ ! -f "$ANALYZER_BIN" ]; then
    echo "❌ FATAL: Compiled log_analyzer binary not found! Please compile main.cpp first."
    exit 1
fi

# 2. Execute the C++ engine passively
echo "🚀 Processing system log sequences..."
$ANALYZER_BIN > /dev/null

# 3. Check if the CSV report generation succeeded
if [ -f "$REPORT_OUT" ]; then
    echo "✅ Success: Production metrics aggregated."
    
    # Optional: Create a timestamped archive duplicate for historical analysis
    TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
    cp "$REPORT_OUT" "data/daily_report_$TIMESTAMP.csv"
    echo "💾 Archive report stored as: data/daily_report_$TIMESTAMP.csv"
else
    echo "❌ ERROR: C++ engine failed to output a report metrics payload."
fi

echo "💤 Execution sequence ended. Entering standby mode."