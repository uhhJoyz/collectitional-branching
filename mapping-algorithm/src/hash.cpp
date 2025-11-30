#include "hash.h"
#include "types.h"
#include "openssl/sha.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <array>

// hashing implementation (loosely) based on:
//   https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c
// accepts a string input and outputs to the provided char buffer
void sha256_hash_str(const std::string &s_input, unsigned char *out_hash)
{
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, s_input.c_str(), s_input.size());
  SHA256_Final(out_hash, &sha256);
}

void mask_hash(unsigned char *hash, char mask)
{
  hash[0] = hash[0] & mask;
}

void force_hash(unsigned char *hash, char mask)
{
  hash[0] = hash[0] | mask;
}

std::string hash_to_string(unsigned char *hash, u32 len)
{
  std::stringstream ss_out;
  for (u32 i = 0; i < len; ++i)
  {
    ss_out << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }
  return ss_out.str();
}

void sha256_hash_veci(std::vector<u32> *in_vec, unsigned char *out_hash)
{
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, in_vec->data(), in_vec->size() * sizeof(int));
  SHA256_Final(out_hash, &sha256);
};

void vectors_to_hashes(std::vector<std::vector<u32>> *in_vecs,
                       std::vector<std::array<unsigned char, SHA256_DIGEST_LENGTH>> *out_hashes)
{
  out_hashes->resize(in_vecs->size());
#pragma omp parallel for
  for (size_t i = 0; i < in_vecs->size(); i++)
  {
    sha256_hash_veci(&in_vecs->at(i), out_hashes->at(i).data());
  }
}

std::vector<u32> hashes_to_machine(std::vector<std::array<unsigned char, SHA256_DIGEST_LENGTH>> *in_hashes,
                                   const size_t n_reducers, const std::vector<long double> *partition_bounds,
                                   const std::vector<size_t> *hardware_codes,
                                   u32 (*map)(unsigned char *, void *))
{
  std::vector<u32> out_reducer_indices(in_hashes->size());

  // #pragma omp parallel for
  for (size_t i = 0; i < in_hashes->size(); i++)
  {
    size_t args[3] = {n_reducers, (size_t)partition_bounds,
                      // skips the dereference if hardware_codes is a nullptr
                      hardware_codes != nullptr ? (size_t)&hardware_codes->at(i) : (size_t)nullptr};

    out_reducer_indices[i] = map((unsigned char *)&in_hashes->at(i), (void *)&args);
  }
  return out_reducer_indices;
}

u32 compare_veci(std::vector<u32> *in_vec1, std::vector<u32> *in_vec2)
{
  if (in_vec1->size() < in_vec2->size())
  {
    return 1;
  }
  else if (in_vec1->size() < in_vec2->size())
  {
    return -1;
  }

  for (size_t i = 0; i < in_vec1->size(); i++)
  {
    if (in_vec1->at(i) < in_vec2->at(i))
    {
      return 1;
    }
    else if (in_vec1->at(i) > in_vec2->at(i))
    {
      return -1;
    }
  }
  // if they are exactly equal, return 1
  return 1;
}
