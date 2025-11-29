#include "map.h"
#include "types.h"
#include "openssl/sha.h"
#include <vector>
#include <iostream>

u32 naive_map(unsigned char *h, void *args)
{
  u32 n_reducers = ((u32 *)args)[0];
  return u32(h[0] % n_reducers);
}

u32 partition_bounded_map(unsigned char *h, void *args)
{
  u32 n_reducers = (u32)((size_t *)args)[0];
  std::vector<long double> *partition_bounds = ((std::vector<long double> **)args)[1];
  long double val = static_cast<long double>(*(reinterpret_cast<u64 *>(h))) / UINT64_MAX;
  size_t i = 0;

  // linear scan because n_reducers is small
  while (val > partition_bounds->at(i) && i < partition_bounds->size() - 1)
  {
    i++;
  }
  return i;
}

// adapted from two sources:
// https://en.wikipedia.org/wiki/Proportional–integral–derivative_controller
// https://www.digikey.com/en/maker/tutorials/2024/implementing-a-pid-controller-algorithm-in-python
u32 pid_controller(float err, float prev_err, float k_p, float k_i, float k_d, float *integral)
{
  (*integral) += err;
  float derivative = err - prev_err;
  // return control value
  return k_p * err + k_i * (*integral) + k_d * derivative;
}

std::vector<long double> initial_partitions(size_t n_reducers)
{
  std::vector<long double> partition_bounds(n_reducers);
  for (size_t i = 0; i <= n_reducers - 1; i++)
  {
    partition_bounds[i] = static_cast<long double>(i) / n_reducers;
  }
  return partition_bounds;
}

std::vector<long double> initial_weights(size_t n_reducers)
{
  std::vector<long double> weights(n_reducers);
  std::fill(weights.begin(), weights.end(), 1.0f / n_reducers);
  return weights;
}

void update_partitions(std::vector<long double> *partition_bounds,
                       std::vector<long double> *weights,
                       std::vector<u32> *runtime_diff_proportion)
{
  size_t n_reducers = partition_bounds->size();
  for (size_t i = 0; i < n_reducers; i++)
  {
    // TODO: implement load balancing
  }
}
