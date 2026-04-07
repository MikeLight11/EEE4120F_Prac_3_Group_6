#!/bin/bash

# --- Configuration ---
EXE="./wariara_freights_route_mpi"
OUTPUT_CSV="benchmark_results.csv"
NREPEAT=10          # Should be >= 5 for trimming to be meaningful
TRIM=2              # Number of values to drop from EACH end (top and bottom)
THREADS="1 2 4 8"
INPUTS="4 5 6 7 8 9 10"

echo "InputFile,Processes,AvgSetupTime,AvgCompTime,AvgTotalTime,SetupSpeedup,CompSpeedup,TotalSpeedup" > $OUTPUT_CSV
mkdir -p ./temp_logs
mkdir -p ./output

for input in $INPUTS; do
    echo "========================================"
    echo "Testing energy${input}"
    echo "========================================"

    BASE_SETUP=0
    BASE_COMP=0
    BASE_TOTAL=0

    for p in $THREADS; do
        SETUP_TIMES=()
        COMP_TIMES=()
        TOTAL_TIMES=()
        echo -n "  -> Processes: $p | Runs: "

        for (( i=1; i<=$NREPEAT; i++ )); do
            echo -n "$i "

            RESULT=$(mpirun -np $p $EXE \
                -i input/energy${input} \
                -o /dev/null 2>&1)

            SETUP=$(echo "$RESULT" | grep "Init Time" | awk '{print $4}')
            COMP=$(echo "$RESULT"  | grep "Comp Time" | awk '{print $4}')

            if [ -z "$SETUP" ]; then echo ""; echo "    WARNING: No setup time on run $i"; SETUP=0; fi
            if [ -z "$COMP"  ]; then echo ""; echo "    WARNING: No comp time on run $i";  COMP=0;  fi

            TOTAL=$(awk "BEGIN {printf \"%.6f\", $SETUP + $COMP}")


        echo "energy${input},$p,$AVG_SETUP,$AVG_COMP,$AVG_TOTAL,$SETUP_SPEEDUP,$COMP_SPEEDUP,$TOTAL_SPEEDUP" >> $OUTPUT_CSV
    done
    echo ""
done
