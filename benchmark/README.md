This directory contains benchmarking programs for CPU and GPU times which provide a curve of best fit.

Due to time constraints and GPU access, there is an additional GPU/CPU parsing script called `reformat-vec.py`. After running the original GPU benchmarks, run this file to generate a curve of best fit calibrated to the vector operations.

The PIM benchmarks were conducted using the benchmark suite entitled PIMbench, found here [PIMeval/PIMbench](https://github.com/UVA-LavaLab/PIMeval-PIMbench). These benchmarked results were formatted in `.csv` files and stored in the `<repo root>/results/pim` folder. Original awk-parsed terminal output from these benchmarks can be found in the `_pim_results.txt` file contained within the aforementioned directory.
