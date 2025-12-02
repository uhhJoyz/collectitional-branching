#include "openssl/sha.h"
#include <iostream>
#include <vector>
#include <array>
#include <chrono>
#include <string.h>

#include "hash.h"
#include "types.h"
#include "map.h"
#include "model.h"
#include "utils.h"
#define BENCH_SIZE 1000
#define BENCH_ITERS 10

void benchmark_timings(u32 (*map)(unsigned char *, void *), std::string file_path)
{
  size_t n_reducers = 16;

  std::vector<std::vector<u32>> data_vecs(BENCH_SIZE);
  for (u32 i = 0; i < BENCH_SIZE; i++)
  {
    data_vecs[i].resize(16);
    for (u32 j = 0; j < 16; j++)
    {
      data_vecs[i][j] = u32(rand());
    }
  }

  std::vector<long double> partition_bounds = initial_partitions(n_reducers);

  std::vector<std::array<unsigned char, SHA256_DIGEST_LENGTH>> hashes;
  vectors_to_hashes(&data_vecs, &hashes);

  std::vector<size_t> hardware_codes(BENCH_SIZE);
  for (size_t i = 0; i < hardware_codes.size(); i++)
  {
    hardware_codes[i] = rand() % 4;
  }

  std::vector<u32> machines;

  auto start = std::chrono::high_resolution_clock::now();

  for (size_t iter = 0; iter < BENCH_ITERS; iter++)
    machines = hashes_to_machine(&hashes, n_reducers, &partition_bounds,
                                 &hardware_codes, map);

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Timing: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << " ms" << std::endl;

  std::vector<long double> weights = initial_weights(n_reducers);
  std::vector<long double> runtimes = model_machines(n_reducers, machines, hardware_codes);
  for (size_t i = 0; i < runtimes.size(); i++)
    std::cout << "Runtime for machine " << i << ": " << runtimes[i] << std::endl;

  if (map == partition_bounded_map)
  {
    update_partitions(&partition_bounds, &weights, &runtimes);
  }

  std::vector<u32> machine_counts(n_reducers, 0);
  for (size_t i = 0; i < partition_bounds.size(); i++)
  {
    machine_counts[machines[i]] += 1;
  }

  for (size_t i = 0; i < machine_counts.size(); i++)
  {
    std::cout << "Machine " << i << " has " << machine_counts[i]
              << " assignments, weight: " << partition_bounds[i] << std::endl;
  }

  for (size_t i = 0; i < hardware_codes.size(); i++)
  {
    hardware_codes[i] = rand() % 4;
  }

  machines = hashes_to_machine(&hashes, n_reducers, &partition_bounds,
                               &hardware_codes, map);
  runtimes = model_machines(n_reducers, machines, hardware_codes);
  serialize_mappings(machines, file_path);
}

int main(int argc, char *argv[])
{
  // benchmark_timings(naive_map, "naive_mappings.txt");
  benchmark_timings(partition_hw_strict, "partition_hw_strict_mappings.txt");
  benchmark_timings(partition_bounded_map, "partition_bounded_mappings.txt");
  return 0;
}
