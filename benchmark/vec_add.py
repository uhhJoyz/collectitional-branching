import pandas as pd
import numpy as np
import time
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import torch

if torch.cuda.is_available():
    device = torch.device("cuda")
    sync = torch.cuda.synchronize
    gpu_backend = "CUDA"
elif torch.backends.mps.is_available():
    device = torch.device("mps")
    sync = torch.mps.synchronize
    gpu_backend = "MPS"
else:
    device = torch.device("cpu")
    sync = lambda: None
    gpu_backend = "CPU"

print("GPU backend:", gpu_backend)

sizes = np.array([2**16, 2**17, 2**18, 2**19, 2**20, 2**21, 2**22])

cpu_times = []
gpu_times = []

print(f"{'Size':>10} | {'CPU (s)':>10} | {'GPU (s)':>10}")
print("-" * 36)

for n in sizes:
    a_cpu = torch.rand(n, dtype=torch.float32, device="cpu")
    b_cpu = torch.rand(n, dtype=torch.float32, device="cpu")

    start = time.perf_counter()
    c_cpu = a_cpu + b_cpu
    end = time.perf_counter()
    cpu_elapsed = end - start
    cpu_times.append(cpu_elapsed)

    print(f"{n:>10} | {cpu_elapsed:>10.6f}", end="")

    a_gpu = torch.rand(n, dtype=torch.float32, device=device)
    b_gpu = torch.rand(n, dtype=torch.float32, device=device)

    sync()
    start = time.perf_counter()
    c_gpu = a_gpu + b_gpu
    sync()
    end = time.perf_counter()

    gpu_elapsed = end - start
    gpu_times.append(gpu_elapsed)
    print(f" | {gpu_elapsed:>10.6f}")

def power_law(x, a, b):
    return a * np.power(x, b)

x = np.array(sizes, dtype=np.float64)
y_cpu = np.array(cpu_times)
y_gpu = np.array(gpu_times)

mask_cpu = (y_cpu > 0)
mask_gpu = (y_gpu > 0)

logx_cpu, logy_cpu = np.log(x[mask_cpu]), np.log(y_cpu[mask_cpu])
logx_gpu, logy_gpu = np.log(x[mask_gpu]), np.log(y_gpu[mask_gpu])

b_cpu, loga_cpu = np.polyfit(logx_cpu, logy_cpu, 1)
b_gpu, loga_gpu = np.polyfit(logx_gpu, logy_gpu, 1)

a_cpu = np.exp(loga_cpu)
a_gpu = np.exp(loga_gpu)

print(f"\nCPU fit: time = {a_cpu:.3e} * size^{b_cpu:.3f}")
print(f"GPU fit: time = {a_gpu:.3e} * size^{b_gpu:.3f}")

x_fit = np.linspace(x.min(), x.max(), 200)
y_fit_cpu = power_law(x_fit, a_cpu, b_cpu)
y_fit_gpu = power_law(x_fit, a_gpu, b_gpu)

plt.figure(figsize=(8, 5))
plt.plot(sizes, cpu_times, 'o-', label='CPU (PyTorch)')
plt.plot(sizes, gpu_times, 's-', label=f'GPU ({gpu_backend})')
plt.plot(x_fit, y_fit_cpu, '--', color='blue', alpha=0.6, label='CPU Fit')
plt.plot(x_fit, y_fit_gpu, '--', color='orange', alpha=0.6, label='GPU Fit')

plt.xscale('log')
plt.yscale('log')
plt.xlabel("Vector size (N)")
plt.ylabel("Time (seconds, log scale)")
plt.title("CPU vs GPU Vector Addition Benchmark (PyTorch)")
plt.legend()
plt.grid(True, which="both", ls="--", alpha=0.5)
plt.tight_layout()

plt.savefig("../results/vec_add_benchmark_torch.png")

df = pd.DataFrame({
    'Size': sizes,
    'CPU_Time_s': cpu_times,
    'GPU_Time_s': gpu_times,
})
df.to_csv("../results/vec_add_results.csv", index=False)

print("\nResults saved to '../results/' ✅")

plt.xscale('log')
plt.yscale('log')
plt.xlabel("Vector size (N)")
plt.ylabel("Time (seconds, log scale)")
plt.title("CPU vs GPU Vector Addition Benchmark (PyTorch)")
plt.legend()
plt.grid(True, which="both", ls="--", alpha=0.5)
plt.tight_layout()

plt.savefig("../results/vec_add_benchmark_torch.png")

df = pd.DataFrame({
    'Size': sizes,
    'CPU_Time_s': cpu_times,
    'GPU_Time_s': gpu_times,
})
df.to_csv("../results/vec_add_results.csv", index=False)
