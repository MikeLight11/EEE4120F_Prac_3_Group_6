import matplotlib.pyplot as plt
import numpy as np

threads = [1, 2, 4, 8]
ideal = [1, 2, 4, 8]

data = {
    'energy4':  {'comp': [1.00, 1.16, 1.07, 0.53], 'total': [1.00, 2.77, 3.40, 1.95], 'tcomp': [0.000132, 0.000114, 0.000123, 0.000250]},
    'energy5':  {'comp': [1.00, 0.22, 0.17, 0.10], 'total': [1.00, 2.78, 2.36, 1.61], 'tcomp': [0.000015, 0.000069, 0.000089, 0.000149]},
    'energy6':  {'comp': [1.00, 0.22, 0.15, 0.07], 'total': [1.00, 1.11, 1.00, 0.54], 'tcomp': [0.000016, 0.000073, 0.000105, 0.000230]},
    'energy7':  {'comp': [1.00, 0.43, 0.30, 0.15], 'total': [1.00, 1.38, 1.03, 0.61], 'tcomp': [0.000029, 0.000067, 0.000097, 0.000198]},
    'energy8':  {'comp': [1.00, 0.87, 0.83, 0.44], 'total': [1.00, 1.41, 1.41, 0.83], 'tcomp': [0.000085, 0.000098, 0.000103, 0.000195]},
    'energy9':  {'comp': [1.00, 1.26, 1.44, 1.14], 'total': [1.00, 1.48, 1.73, 1.39], 'tcomp': [0.000269, 0.000214, 0.000187, 0.000236]},
    'energy10': {'comp': [1.00, 1.63, 2.51, 3.38], 'total': [1.00, 1.67, 2.52, 3.34], 'tcomp': [0.001466, 0.000901, 0.000584, 0.000434]},
}

colors = plt.cm.tab10(np.linspace(0, 0.7, len(data)))

# ── Graph 1: Computation Speedup ──────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(7, 4))
ax.plot(threads, ideal, 'k--', label='Ideal', linewidth=1.5)
for (label, vals), color in zip(data.items(), colors):
    ax.plot(threads, vals['comp'], marker='o', label=label, color=color)
ax.set_xlabel('Number of Threads')
ax.set_ylabel('Computation Speedup')
ax.set_title('Computation Speedup vs Threads (OpenMP)')
ax.set_xticks(threads)
ax.legend(fontsize=8, ncol=2)
ax.grid(True, linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('openmp_comp_speedup.png', dpi=150)
plt.show()

# ── Graph 2: Total Speedup ────────────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(7, 4))
ax.plot(threads, ideal, 'k--', label='Ideal', linewidth=1.5)
for (label, vals), color in zip(data.items(), colors):
    ax.plot(threads, vals['total'], marker='o', label=label, color=color)
ax.set_xlabel('Number of Threads')
ax.set_ylabel('Total Speedup')
ax.set_title('Total Speedup vs Threads (OpenMP)')
ax.set_xticks(threads)
ax.legend(fontsize=8, ncol=2)
ax.grid(True, linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('openmp_total_speedup.png', dpi=150)
plt.show()

# ── Graph 3: Computation Time ─────────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(7, 4))
for (label, vals), color in zip(data.items(), colors):
    ax.plot(threads, [t * 1000 for t in vals['tcomp']], marker='o', label=label, color=color)
ax.set_xlabel('Number of Threads')
ax.set_ylabel('$T_{comp}$ (ms)')
ax.set_title('Computation Time vs Threads (OpenMP)')
ax.set_xticks(threads)
ax.legend(fontsize=8, ncol=2)
ax.grid(True, linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('openmp_tcomp.png', dpi=150)
plt.show()