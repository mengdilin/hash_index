#include <iostream>
#include <vector>
#include "Page.h"

class HashIndex {
  public:
    unsigned int number_buckets;
  private:
    std::vector<Page*> primary_buckets;
    std::vector<Page*> overflow_pages;
    float load_capacity;
    static const int PAGE_SIZE = 4096;
    //static constexpr double KNUTH_NUMBER = 1054997077.39;

public:
  HashIndex(float load_capacity);
  uint32_t hash(uint64_t);
  static uint64_t search(uint64_t, std::string);
  void build_index(std::string);
  static void debugRead(std::string);
  std::vector<DataEntry> parse_idx_file(std::string path);
  ~HashIndex();
private:
  static uint64_t search(uint64_t, unsigned int);
  static uint32_t hash(uint64_t, unsigned int);
  void add_entry_to_bucket(uint32_t, DataEntry);
  Page* get_overflow_page(Page*);
};
