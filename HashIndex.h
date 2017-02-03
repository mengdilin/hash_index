#include <iostream>
#include <vector>
#include "Page.h"

class HashIndex {
  public:
    int number_buckets;
  private:
    std::vector<Page*> primary_buckets;
    std::vector<Page*> overflow_pages;
    float load_capacity;
    static const double KNUTH_NUMBER = 1054997077.39;

public:
  HashIndex(float load_capacity);
  uint32_t hash(uint64_t);
  uint64_t search(uint64_t);
  void build_index(std::string);
  static void debugRead(std::string);
  std::vector<DataEntry> parse_idx_file(std::string path);
  ~HashIndex();
};
