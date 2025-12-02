#ifndef MODEL_H
#define MODEL_H

#include "types.h"
#include <cstddef>
#include <vector>

#define OP_VEC_ADD 0u
#define OP_VEC_DOT 1u
#define OP_MAT_MAT 2u
#define OP_MAT_VEC 3u

long double bank_level_est(size_t size, size_t operation);
long double gpu_est(size_t size, size_t operation);
long double cpu_est(size_t size, size_t operation);
std::vector<long double> model_machines(size_t n_reducers, std::vector<u32> machines, std::vector<size_t> op_codes);

#endif // MODEL_H
