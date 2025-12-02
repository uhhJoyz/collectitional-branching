#ifndef UTILS_H
#define UTILS_H
#include "types.h"
#include <vector>
#include <string>

std::vector<u32> read_mappings(std::string file_path);
void serialize_mappings(std::vector<u32> machines, std::string file_path);
long double max_val(std::vector<long double> vec);

#endif //UTILS_H
