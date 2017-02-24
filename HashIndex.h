#include <iostream>
#include <vector>
#include <unordered_map>
#include "Page.h"

class HashIndex {
  public:
    unsigned int number_buckets;
  private:
    std::vector<Page*> primary_buckets;

    //used to maintain a deterministic order of overflow pages
    std::vector<Page*> overflow_pages;

    //overflow page -> parent
    std::unordered_map<Page*, Page*> map_for_prev_page;

    //parent -> overflow page
    std::unordered_map<Page*, Page*> overflow_map;
    float load_capacity;
    //static const int PAGE_SIZE = 4096;
    static const int PAGE_SIZE = 48;

    //static constexpr double KNUTH_NUMBER = 1054997077.39;

public:
  HashIndex(float load_capacity);
  uint32_t hash(uint64_t);
  static std::pair<bool,uint64_t> search(uint64_t, std::string);
  void build_index(std::string, std::string);
  static void debugRead(std::string);
  std::vector<DataEntry> parse_idx_file(std::string path);
  ~HashIndex();
private:
  static uint64_t search(uint64_t, unsigned int);
  static uint32_t hash(uint64_t, unsigned int);
  Page* get_overflow_page(Page*);
  int merge(std::vector<Page*>&, std::vector<Page*>&);
  bool page_has_overflow(Page*);
};
