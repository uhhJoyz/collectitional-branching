import numpy as np
import torch
import pandas as pd
import matplotlib.pyplot as plt
from scipy.stats import linregress
from utils.timer import measure_time

MAX_DIM = 2 ** 16
MAX_BYTES = 4 * 1024**3

def _fits_in_memory_elems(n_elems):
    return (n_elems * 4) <= MAX_BYTES

def benchmark_matvec(device="cpu", N=512, trials=10):
    if N > MAX_DIM:
        return {"op": "matvec", "device": device, "size": N, "skipped": True,
                "reason": f"dimension {N} > MAX_DIM {MAX_DIM}"}
    if not _fits_in_memory_elems(N * N + N):
        return {"op": "matvec", "device": device, "size": N, "skipped": True,
                "reason": "would exceed MAX_BYTES"}

    if device == "cpu":
        A = np.random.rand(N, N).astype(np.float32)
        x = np.random.rand(N).astype(np.float32)

        def run():
            _ = A.dot(x)

        mean, std = measure_time(run, synchronize=None, trials=trials)
        return {"op": "matvec", "device": "cpu", "size": N, "mean_ms": mean, "std_ms": std, "skipped": False}

    elif device == "gpu":
        device_torch = torch.device("mps" if torch.backends.mps.is_available() else "cpu")
        A = torch.rand((N, N), dtype=torch.float32, device=device_torch)
        x = torch.rand((N,), dtype=torch.float32, device=device_torch)

        def run():
            _ = torch.matmul(A, x)

        def sync():
            if device_torch.type == "mps":
                torch.mps.synchronize()

        mean, std = measure_time(run, synchronize=sync, trials=trials)
        return {"op": "matvec", "device": device_torch.type, "size": N, "mean_ms": mean, "std_ms": std, "skipped": False}

    else:
        raise ValueError("device must be 'cpu' or 'gpu' or 'mps'")


sizes = [2**i for i in range(6, 13)]
cpu_times = []
gpu_times = []

print(f"{'Size':>10} | {'CPU (ms)':>10} | {'GPU (ms)':>10}")
print("-" * 36)

for n in sizes:
    res_cpu = benchmark_matvec("cpu", n)
    res_gpu = benchmark_matvec("gpu", n)

    cpu_times.append(res_cpu["mean_ms"])
    gpu_times.append(res_gpu["mean_ms"])

    print(f"{n:>10} | {res_cpu['mean_ms']:>10.4f} | {res_gpu['mean_ms']:>10.4f}")


def power_law_fit(x, y):
    logx, logy = np.log(x), np.log(y)
    slope, intercept, r_value, _, _ = linregress(logx, logy)
    a, b = np.exp(intercept), slope
    return a, b, r_value**2


a_cpu, b_cpu, r2_cpu = power_law_fit(np.array(sizes), np.array(cpu_times))
a_gpu, b_gpu, r2_gpu = power_law_fit(np.array(sizes), np.array(gpu_times))

print(f"\nCPU fit: time = {a_cpu:.3e} * n^{b_cpu:.3f} (R²={r2_cpu:.3f})")
print(f"GPU fit: time = {a_gpu:.3e} * n^{b_gpu:.3f} (R²={r2_gpu:.3f})")

x_fit = np.linspace(min(sizes), max(sizes), 200)
y_cpu_fit = a_cpu * np.power(x_fit, b_cpu)
y_gpu_fit = a_gpu * np.power(x_fit, b_gpu)

plt.figure(figsize=(8, 5))
plt.plot(sizes, cpu_times, "o-", label="CPU (NumPy)")
plt.plot(sizes, gpu_times, "s-", label="GPU (PyTorch MPS)")
plt.plot(x_fit, y_cpu_fit, "--", color="blue", alpha=0.6, label="CPU Fit")
plt.plot(x_fit, y_gpu_fit, "--", color="orange", alpha=0.6, label="GPU Fit")

plt.xscale("log")
plt.yscale("log")
plt.xlabel("Vector/Matrix dimension (N)")
plt.ylabel("Mean time (ms, log scale)")
plt.title("CPU vs GPU Matrix-Vector Multiplication Benchmark (PyTorch)")
plt.legend()
plt.grid(True, which="both", ls="--", alpha=0.5)
plt.tight_layout()

plt.savefig("../results/matvec_benchmark_torch.png")

df = pd.DataFrame({
    "Size": sizes,
    "CPU_Time_ms": cpu_times,
    "GPU_Time_ms": gpu_times
})
df.to_csv("../results/matvec_results.csv", index=False)
