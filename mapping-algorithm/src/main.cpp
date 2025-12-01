#include "openssl/sha.h"
#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>

#include "hash.h"
#include "types.h"
#include "map.h"
#define BENCH_SIZE 10000000

int main(int argc, char *argv[])
{
  auto start = std::chrono::high_resolution_clock::now();

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Timing: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
            << " ms" << std::endl;

  std::vector<std::vector<u32>> data_vecs(64);
  for (u32 i = 0; i < 64; i++)
  {
    data_vecs[i].resize(16);
    for (u32 j = 0; j < 16; j++)
    {
      data_vecs[i][j] = u32(rand());
    }
  }

  std::vector<long double> partition_bounds = initial_partitions(16);

  std::vector<std::array<unsigned char, SHA256_DIGEST_LENGTH>> hashes;
  vectors_to_hashes(&data_vecs, &hashes);

  std::vector<size_t> hardware_codes(64);
  for (size_t i = 0; i < hardware_codes.size(); i++)
  {
    hardware_codes[i] = rand() % 256;
  }

  std::vector<u32> machines = hashes_to_machine(&hashes, 16, &partition_bounds,
                                                &hardware_codes, partition_hw_strict);

  std::vector<long double> weights = initial_weights(16);
  std::vector<u32> runtimes = {225, 180, 150, 200, 175, 190, 160, 20,
                               10, 195, 170, 155, 205, 215, 220, 230};

  update_partitions(&partition_bounds, &weights, &runtimes);

  return 0;
}
