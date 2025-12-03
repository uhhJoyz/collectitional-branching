#include "model.h"
#include "types.h"

#include <vector>
#include <cstddef>
#include <iostream>
#include <math.h>

std::vector<long double> model_machines(size_t n_reducers, std::vector<u32> machines, std::vector<size_t> op_codes)
{
  std::vector<long double> runtimes(n_reducers, 0.0l);
  std::vector<std::vector<size_t>> machine_ops(n_reducers, std::vector<size_t>(4));
  for (size_t i = 0; i < machines.size(); i++)
  {
    machine_ops[machines[i]][op_codes[i]]++;
  }

#pragma omp parallel for
  for (size_t i = 0; i < n_reducers; i++)
  {
    long double (*model)(size_t size, size_t operation);
    if (i >= n_reducers / 2)
    {
      model = gpu_est;
    }
    else
    {
      model = bank_level_est;
    }
    long double time = 0.0l;
    time += model(machine_ops[i][OP_VEC_ADD]*16, OP_VEC_ADD);
    time += model(machine_ops[i][OP_VEC_DOT]*16, OP_VEC_DOT);
    time += model(machine_ops[i][OP_MAT_MAT]*16, OP_MAT_MAT);
    time += model(machine_ops[i][OP_MAT_VEC]*16, OP_MAT_VEC);
    runtimes[i] = time;
  }

  return runtimes;
}

long double bank_level_est(size_t size, size_t operation)
{
  switch (operation)
  {
  case OP_VEC_ADD:
    return 1.266e-07 * pow((float)size, 0.999);
  case OP_VEC_DOT:
    return 1.393e-07 * pow((float)size, 0.996);
  case OP_MAT_VEC:
    return 6.208e-02 * pow((float)size, 0.999);
  case OP_MAT_MAT:
    return 8.043e-02 * pow((float)size, 0.998);
  default:
    // return largest possible value if none selected
    return -1u;
  }
}

long double gpu_est(size_t size, size_t operation)
{
  switch (operation)
  {
  case OP_VEC_ADD:
    return 3.981e-07 * pow((float)size, 0.771);
  case OP_VEC_DOT:
    return 3.045e-05 * pow((float)size, 0.488);
  case OP_MAT_VEC:
    return 2.798e-06 * pow((float)size, 1.850);
  case OP_MAT_MAT:
    return 1.749e-04 * pow((float)size, 0.796);
  default:
    // return largest possible value if none selected
    return -1u;
  }
}

long double cpu_est(size_t size, size_t operation)
{
  switch (operation)
  {
  case OP_VEC_ADD:
    return 1.509e-06 * pow((float)size, 0.684);
  case OP_VEC_DOT:
    return 1.820e-07 * pow((float)size, 1.030);
  case OP_MAT_VEC:
    return 1.128e-06 * pow((float)size, 0.991);
  case OP_MAT_MAT:
    return 2.781e-05 * pow((float)size, 0.952);
  default:
    // return largest possible value if none selected
    return -1u;
  }
}
