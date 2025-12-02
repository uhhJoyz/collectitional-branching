#include "utils.h"
#include "types.h"
#include <vector>
#include <fstream>

std::vector<u32> read_mappings(std::string file_path)
{
  std::string line;
  std::ifstream file(file_path);
  std::vector<u32> vec;
  if (file.is_open())
  {
    while (std::getline(file, line))
    {
      if (line.empty())
        continue;
      vec.push_back(static_cast<u32>(std::stoul(line)));
    }
    file.close();
    return vec;
  }
  else
  {
    throw std::runtime_error("Unable to open file: " + file_path);
  }
}

void serialize_mappings(std::vector<u32> machines, std::string file_path)
{
  std::ofstream file;
  file.open(file_path);
  for (u32 i = 0; i < machines.size(); i++)
  {
    file << machines[i];
    if (i != machines.size() - 1)
      file << "\n";
  }
}

long double max_val(std::vector<long double> vec)
{
  long double max_val = vec[0];
  for (size_t i = 1; i < vec.size(); i++)
  {
    if (vec[i] > max_val)
    {
      max_val = vec[i];
    }
  }
  return max_val;
}
