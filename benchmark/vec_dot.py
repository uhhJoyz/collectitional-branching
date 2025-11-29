import pandas as pd
import numpy as np
import time
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import torch

if torch.cuda.is_available():
    device = torch.device("cuda")
    backend = "CUDA"
elif torch.backends.mps.is_available():
    device = torch.device("mps")
    backend = "MPS"
else:
    device = torch.device("cpu")
    backend = "CPU"

print(f"Using device: {device} ({backend})")

sizes = np.array([2**16, 2**17, 2**18, 2**19, 2**20, 2**21, 2**22])
cpu_times = []
gpu_times = []

print(f"{'Size':>10} | {'CPU (s)':>10} | {'GPU (s)':>10}")
print("-" * 36)

for n in sizes:
    a_cpu = np.random.rand(n).astype(np.float32)
    b_cpu = np.random.rand(n).astype(np.float32)
    start = time.perf_counter()
    c_cpu = np.dot(a_cpu, b_cpu)
    end = time.perf_counter()
    cpu_elapsed = end - start
    cpu_times.append(cpu_elapsed)
    print(f"{n:>10} | {cpu_elapsed:>10.6f}", end="")

    a_gpu = torch.rand(n, dtype=torch.float32, device=device)
    b_gpu = torch.rand(n, dtype=torch.float32, device=device)
    if device.type == "mps":
        torch.mps.synchronize()
    start = time.perf_counter()
    c_gpu = torch.dot(a_gpu, b_gpu)
    if device.type == "mps":
        torch.mps.synchronize()
    elif device.type == "cuda":
        torch.cuda.synchronize()
    end = time.perf_counter()
    gpu_elapsed = end - start
    gpu_times.append(gpu_elapsed)
    print(f" | {gpu_elapsed:>10.6f}")

df = pd.DataFrame({
    'Size': sizes,
    'CPU_Time_s': cpu_times,
    'GPU_Time_s': gpu_times,
})

def power_law(x, a, b):
    return a * np.power(x, b)

x = df['Size'].values
y_cpu = df['CPU_Time_s'].values
y_gpu = df['GPU_Time_s'].values

cpu_params, _ = curve_fit(power_law, x, y_cpu, maxfev=10000)
gpu_params, _ = curve_fit(power_law, x, y_gpu, maxfev=10000)

a_cpu, b_cpu = cpu_params
a_gpu, b_gpu = gpu_params

print(f"\nCPU fit: time = {a_cpu:.3e} * size^{b_cpu:.3f}")
print(f"GPU fit: time = {a_gpu:.3e} * size^{b_gpu:.3f}")

x_fit = np.linspace(x.min(), x.max(), 200)
y_fit_cpu = power_law(x_fit, a_cpu, b_cpu)
y_fit_gpu = power_law(x_fit, a_gpu, b_gpu)

plt.figure(figsize=(8, 5))
plt.scatter(x, y_cpu, label="CPU (data)", color="tab:blue")
plt.scatter(x, y_gpu, label=f"{backend} (data)", color="tab:orange")
plt.plot(x_fit, y_fit_cpu, '--', color="tab:blue", label=f"CPU fit: y={a_cpu:.2e}x^{b_cpu:.2f}")
plt.plot(x_fit, y_fit_gpu, '--', color="tab:orange", label=f"{backend} fit: y={a_gpu:.2e}x^{b_gpu:.2f}")
plt.xscale('log')
plt.yscale('log')
plt.xlabel("Vector size (N)")
plt.ylabel("Time (seconds, log scale)")
plt.title(f"CPU vs {backend} Vector Dot Benchmark (PyTorch)")
plt.legend()
plt.grid(True, which="both", ls="--", alpha=0.5)
plt.tight_layout()

plt.savefig("../results/vec_dot_benchmark_torch.png", dpi=200)
df.to_csv("../results/vec_dot_results.csv", index=False)