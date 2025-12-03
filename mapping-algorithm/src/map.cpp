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

  if (val < partition_bounds->at(1))
  {
    return 0u;
  }
  // linear scan because n_reducers is small
  while (val > partition_bounds->at(i) && i < partition_bounds->size() - 1)
  {
    i++;
  }
  if (val <= partition_bounds->at(i))
  {
    i--;
  }

  return i;
}

// adapted from two sources:
// https://en.wikipedia.org/wiki/Proportional–integral–derivative_controller
// https://www.digikey.com/en/maker/tutorials/2024/implementing-a-pid-controller-algorithm-in-python
// unused & not included in header file
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

// in this use-case, this operation is performed offline
// and therefore doesn't need to be fast
void update_partitions(std::vector<long double> *partition_bounds,
                       std::vector<long double> *weights,
                       std::vector<long double> *runtimes)
{
  u32 total_runtime = 0;
  for (size_t i = 0; i < runtimes->size(); i++)
    total_runtime += runtimes->at(i);

  std::vector<long double> sec_per_unit(weights->size());
  size_t n_reducers = partition_bounds->size();
  for (size_t i = 0; i < n_reducers; i++)
    sec_per_unit.at(i) = runtimes->at(i) != 0 ? 1.0l / ((static_cast<long double>(runtimes->at(i)) / static_cast<long double>(total_runtime)) / weights->at(i))
                                              : weights->at(i);

  long double total_weights = 0.0;
  for (auto w : sec_per_unit)
    total_weights += w;

  for (size_t i = 0; i < n_reducers; i++)
    weights->at(i) = sec_per_unit.at(i) / total_weights;

  long double running_sum = 0.0;
  for (size_t i = 1; i < n_reducers; i++)
  {
    running_sum += weights->at(i - 1);
    partition_bounds->at(i) = running_sum;
  }
}

u32 partition_hw_strict(unsigned char *h, void *args)
{
  u32 n_reducers = (u32)((size_t *)args)[0];
  std::vector<long double> *partition_bounds = (std::vector<long double> *)((size_t *)args)[1];
  size_t operation_code = *((size_t *)((size_t *)args)[2]);

  long double hardware_factor = 2.0f;
  long double hardware_offset = 0.5f;
  if (operation_code < 2u)
  {
    hardware_factor = 2.0f;
    hardware_offset = 0.0f;
  }

  long double val = (static_cast<long double>(*(reinterpret_cast<size_t *>(h))) / UINT64_MAX) / hardware_factor + hardware_offset;
  size_t i = 0;

  if (val < partition_bounds->at(1))
  {
    return 0u;
  }
  // linear scan because n_reducers is small (in simulation)
  while (val > partition_bounds->at(i) && i < partition_bounds->size() - 1)
  {
    i++;
  }
  if (val <= partition_bounds->at(i))
  {
    i--;
  }

  return i;
}
