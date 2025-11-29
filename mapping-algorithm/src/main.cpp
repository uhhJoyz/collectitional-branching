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

  const std::vector<long double> partition_bounds = initial_partitions(16);

  std::vector<std::array<unsigned char, SHA256_DIGEST_LENGTH>> hashes;
  vectors_to_hashes(&data_vecs, &hashes);
  for (u32 i = 0; i < hashes.size(); i++)
  {
    std::cout << hash_to_string(hashes[i].data(), SHA256_DIGEST_LENGTH) << std::endl;
  }

  std::vector<u32> machines = hashes_to_machine(&hashes, 16, &partition_bounds, naive_map);

  for (u32 i = 0; i < machines.size(); i++)
  {
    std::cout << "Machine: " << machines[i] << std::endl;
  }

  size_t args[2] = {16, (size_t)&partition_bounds};
  for (u32 i = 0; i < 8; i++)
  {
    std::cout << partition_bounded_map((unsigned char *)hashes[i].data(), (void *)args) << std::endl;
  }

  return 0;
}
