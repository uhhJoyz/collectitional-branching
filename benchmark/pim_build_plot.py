import numpy as np
import pandas as pd
import sys


def read_series(df):
    sizes = df["problem size"].to_numpy()
    bl_times = df["bl runtime"].to_numpy() + df["bl copy time"].to_numpy()
    bs_times = df["bs runtime"].to_numpy() + df["bs copy time"].to_numpy()
    return sizes, bl_times, bs_times


def fit_duo(sizes, bl_times, bs_times, name=None):
    fit_df(sizes, bl_times, (name if name is not None else "") + " (BL)")
    fit_df(sizes, bs_times, (name if name is not None else "") + " (BS)")


def fit_df(sizes, times, name=None):
    x = np.array(sizes, dtype=np.float64)
    y = np.array(times)

    masked = y > 0
    logx, logy = np.log(x[masked]), np.log(y[masked])
    b, loga = np.polyfit(logx, logy, 1)

    a = np.exp(loga)

    print(
        f"PIM fit {'(' + name + ')' if name is not None else ""}: time = {a:.3e} * size^{b:.3f}"
    )


if __name__ == "__main__":
    if len(sys.argv) != 2:
        fit_duo(
            *read_series(pd.read_csv("../results/pim/pim_vec_add.csv", header=0)),
            name="vector_add",
        )
        fit_duo(
            *read_series(pd.read_csv("../results/pim/pim_vec_dot.csv", header=0)),
            name="vector_dot",
        )
        fit_duo(
            *read_series(pd.read_csv("../results/pim/pim_mat_vec.csv", header=0)),
            name="matrix_vector",
        )
        fit_duo(
            *read_series(pd.read_csv("../results/pim/pim_mat_mat.csv", header=0)),
            name="matrix_matrix",
        )
    else:
        fit_duo(*read_series(pd.read_csv(sys.argv[1], header=False)))
