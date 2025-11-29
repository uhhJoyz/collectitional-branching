#ifndef HASH_H
#define HASH_H
#include "types.h"
#include <vector>
#include "openssl/sha.h"

// template <typename T>
// class Hasher {
//   public:
//   void hash_vector(std::vector<T> *v, unsigned char* hash_out);
// };

void mask_hash(unsigned char* hash, char mask);
std::string hash_to_string(unsigned char* hash, u32 len);
// template <typename T> void sha256_hash_vector(std::vector<T> &v, unsigned char* hash_out);
void sha256_hash_str(const std::string &s_input, unsigned char* hash_out);
void sha256_hash_veci(std::vector<u32> *in_vec, unsigned char* out_hash);
std::vector<u32> hashes_to_machine(std::vector<std::array<unsigned char, SHA256_DIGEST_LENGTH>> *in_hashes,
                                   const size_t n_reducers, const std::vector<long double> *partition_bounds,
                                   u32 (*map)(unsigned char *, void *));
u32 compare_veci(std::vector<u32> *in_vec1, std::vector<u32> *in_vec2);
void vectors_to_hashes(std::vector<std::vector<u32>> *in_vecs,
                       std::vector<std::array<unsigned char, SHA256_DIGEST_LENGTH>> *out_hashes);

#endif // HASH_H
