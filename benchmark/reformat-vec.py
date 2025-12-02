import numpy as np
import pandas as pd


def read_series(df):
    sizes = df["Size"].to_numpy()
    gpu_times = df["GPU_Time_s"].to_numpy() * 1000.0
    cpu_times = df["CPU_Time_s"].to_numpy() * 1000.0
    return sizes, gpu_times, cpu_times


def fit_duo(sizes, gpu_times, cpu_times, name=None):
    fit_df(sizes, gpu_times, (name if name is not None else "") + " (GPU)")
    fit_df(sizes, cpu_times, (name if name is not None else "") + " (CPU)")


def fit_df(sizes, times, name=None):
    x = np.array(sizes, dtype=np.float64)
    y = np.array(times)

    masked = y > 0
    logx, logy = np.log(x[masked]), np.log(y[masked])
    b, loga = np.polyfit(logx, logy, 1)

    a = np.exp(loga)

    print(
        f"GPU fit {'(' + name + ')' if name is not None else ""}: time = {a:.3e} * size^{b:.3f}"
    )


if __name__ == "__main__":
    fit_duo(
        *read_series(pd.read_csv("../results/vec_add_results.csv", header=0)),
        name="vector_add",
    )
    fit_duo(
        *read_series(pd.read_csv("../results/vec_dot_results.csv", header=0)),
        name="vector_dot",
    )
