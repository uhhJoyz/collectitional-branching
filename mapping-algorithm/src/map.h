#ifndef MAP_H
#define MAP_H
#include "types.h"

u32 naive_map(unsigned char *h, void *args);
u32 partition_bounded_map(unsigned char *h, void *args);
std::vector<long double> initial_partitions(size_t n_reducers);
std::vector<long double> initial_weights(size_t n_reducers);

#endif // MAP_H
