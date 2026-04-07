import matplotlib.pyplot as plt
import numpy as np

threads = [1, 2, 4, 8]
ideal   = [1, 2, 4, 8]

data = {
    'energy4':  {'comp': [1.00, 0.24, 0.20, 0.12], 'total': [1.00, 1.04, 0.99, 0.74], 'tcomp': [0.020, 0.084, 0.102, 0.169]},
    'energy5':  {'comp': [1.00, 0.25, 0.18, 0.11], 'total': [1.00, 0.66, 0.53, 0.38], 'tcomp': [0.013, 0.052, 0.072, 0.114]},
    'energy6':  {'comp': [1.00, 0.26, 0.15, 0.09], 'total': [1.00, 0.57, 0.38, 0.24], 'tcomp': [0.011, 0.042, 0.071, 0.127]},
    'energy7':  {'comp': [1.00, 0.50, 0.26, 0.15], 'total': [1.00, 1.08, 0.69, 0.43], 'tcomp': [0.019, 0.038, 0.072, 0.129]},
    'energy8':  {'comp': [1.00, 0.90, 0.75, 0.46], 'total': [1.00, 0.94, 0.80, 0.54], 'tcomp': [0.055, 0.061, 0.073, 0.120]},
    'energy9':  {'comp': [1.00, 1.31, 1.40, 1.00], 'total': [1.00, 1.26, 1.35, 0.96], 'tcomp': [0.157, 0.120, 0.112, 0.157]},
    'energy10': {'comp': [1.00, 1.77, 2.71, 2.57], 'total': [1.00, 1.74, 2.62, 2.48], 'tcomp': [0.853, 0.482, 0.315, 0.332]},
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
    ax.plot(threads, vals['tcomp'], marker='o', label=label, color=color)
ax.set_xlabel('Number of Threads')
ax.set_ylabel('$T_{comp}$ (ms)')
ax.set_title('Computation Time vs Threads (OpenMP)')
ax.set_xticks(threads)
ax.legend(fontsize=8, ncol=2)
ax.grid(True, linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('openmp_tcomp.png', dpi=150)
plt.show()