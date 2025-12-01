#ifdef UTILS_H
#define UTILS_H
#include "types.h
#include <vector>

std::vector<u32> read_mappings(std::string file_path);
void serialize_mappings(std::vector<u32> machines);

#endif //UTILS_H
