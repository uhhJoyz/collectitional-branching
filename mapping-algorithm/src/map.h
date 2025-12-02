#ifndef MAP_H
#define MAP_H
#include "types.h"
#include <vector>

u32 naive_map(unsigned char *h, void *args);
u32 partition_bounded_map(unsigned char *h, void *args);
u32 partition_hw_strict(unsigned char *h, void *args);
std::vector<long double> initial_partitions(size_t n_reducers);
std::vector<long double> initial_weights(size_t n_reducers);

void update_partitions(std::vector<long double> *partition_bounds,
                       std::vector<long double> *weights,
                       std::vector<long double> *runtimes);
static std::vector<double> zipf_cdf(u32 m, double alpha);

#endif // MAP_H
