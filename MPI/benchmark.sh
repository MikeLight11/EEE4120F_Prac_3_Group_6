#!/bin/bash

# --- Configuration ---
EXE="./wariara_freights_route_mpi"
OUTPUT_CSV="benchmark_results.csv"
NREPEAT=10
THREADS="1 2 3 4 5 6 7 8"
INPUTS="4 5 6 7 8 9 10"

echo "InputFile,Processes,AvgCompTime,Speedup" > $OUTPUT_CSV
mkdir -p ./temp_logs
mkdir -p ./output

for input in $INPUTS; do
    echo "========================================"
    echo "Testing energy${input}"
    echo "========================================"

    BASE_TIME=0

    for p in $THREADS; do
        TOTAL_TIME=0
        echo -n "  -> Processes: $p | Runs: "

        for (( i=1; i<=$NREPEAT; i++ )); do
            echo -n "$i "

            RESULT=$(mpirun -np $p $EXE \
                -i input/energy${input} \
                -o /dev/null 2>&1)

            # "Comp Time (Tcomp):  0.000074 seconds"
            #   $1    $2     $3       $4        $5
            TIME=$(echo "$RESULT" | grep "Comp Time" | awk '{print $4}')

            # Guard against empty TIME
            if [ -z "$TIME" ]; then
                echo ""
                echo "    WARNING: No time extracted on run $i"
                TIME=0
            fi

            TOTAL_TIME=$(awk "BEGIN {print $TOTAL_TIME + $TIME}")
        done

        AVG_TIME=$(awk "BEGIN {printf \"%.6f\", $TOTAL_TIME / $NREPEAT}")

        if [ "$p" -eq "1" ]; then
            BASE_TIME=$AVG_TIME
            SPEEDUP="1.00"
        else
            SPEEDUP=$(awk "BEGIN {printf \"%.2f\", $BASE_TIME / $AVG_TIME}")
        fi

        echo "| Avg: ${AVG_TIME}s | Speedup: ${SPEEDUP}x"
        echo "energy${input},$p,$AVG_TIME,$SPEEDUP" >> $OUTPUT_CSV
    done
    echo ""
done

echo "Done! Final results stored in $OUTPUT_CSV"