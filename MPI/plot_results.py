import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# 1. Load the data
try:
    df = pd.read_csv('benchmark_results.csv')
except FileNotFoundError:
    print("Error: benchmark_results.csv not found.")
    exit()

# 2. Configure plot aesthetics
plt.rcParams['axes.grid'] = True
plt.rcParams['grid.linestyle'] = '--'
plt.rcParams['grid.alpha'] = 0.5

# --- Plot 1: Computation Speedup ---
plt.figure(figsize=(9, 6))
plt.plot([1, 8], [1, 8], color='black', linestyle='--', linewidth=1.5, label='Ideal')

for input_file in df['InputFile'].unique():
    subset = df[df['InputFile'] == input_file]
    plt.plot(subset['Processes'], subset['CompSpeedup'], 
             marker='o', markersize=6, label=input_file)

plt.title('Computation Speedup vs Processes (MPI)', fontsize=14)
plt.xlabel('Number of Processes (p)', fontsize=12)
plt.ylabel('Speedup Factor ($S_p$)', fontsize=12)
plt.xticks([1, 2, 4, 8])
plt.yticks(range(0, 10))
plt.ylim(0, 9)
plt.legend(ncol=2, loc='upper left', fontsize='small')
plt.tight_layout()
plt.savefig('MPI_CompSpeedup.png')

# --- Plot 2: Total Speedup (NEW) ---
plt.figure(figsize=(9, 6))
plt.plot([1, 8], [1, 8], color='black', linestyle='--', linewidth=1.5, label='Ideal')

for input_file in df['InputFile'].unique():
    subset = df[df['InputFile'] == input_file]
    plt.plot(subset['Processes'], subset['TotalSpeedup'], 
             marker='s', markersize=6, label=input_file) # used square markers to differentiate

plt.title('Total Speedup (Init + Comp) vs Processes', fontsize=14)
plt.xlabel('Number of Processes (p)', fontsize=12)
plt.ylabel('Total Speedup Factor ($S_p$)', fontsize=12)
plt.xticks([1, 2, 4, 8])
plt.yticks(range(0, 10))
plt.ylim(0, 9)
plt.legend(ncol=2, loc='upper left', fontsize='small')
plt.tight_layout()
plt.savefig('MPI_TotalSpeedup.png')

# --- Plot 3: Average Total Time (Logarithmic) ---
plt.figure(figsize=(9, 6))

for input_file in df['InputFile'].unique():
    subset = df[df['InputFile'] == input_file]
    plt.plot(subset['Processes'], subset['AvgTotalTime'], 
             marker='o', markersize=6, label=input_file)

plt.title('Average Total Time vs Processes (MPI)', fontsize=14)
plt.xlabel('Number of Processes (p)', fontsize=12)
plt.ylabel('Time (Seconds) - Log Scale', fontsize=12)
plt.xticks([1, 2, 4, 8])
plt.yscale('log')

ax = plt.gca()
ax.yaxis.set_major_formatter(ticker.ScalarFormatter())
ax.yaxis.get_major_formatter().set_scientific(False)
ax.yaxis.get_major_formatter().set_useOffset(False)

plt.legend(ncol=2, loc='upper right', fontsize='small')
plt.tight_layout()
plt.savefig('MPI_AvgTotalTime.png')

print("All three plots (Comp Speedup, Total Speedup, and Total Time) generated.")