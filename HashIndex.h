#include <iostream>
#include <vector>
#include <unordered_map>
#include "Page.h"
#include <stdio.h>


class HashIndex {
  public:
    unsigned int number_buckets;
    double total_page_read_speed = 0;
    unsigned int total_page_read = 0;
  private:
    std::vector<Page*> primary_buckets;

    //used to maintain a deterministic order of overflow pages
    std::vector<Page*> overflow_pages;

    //overflow page -> parent
    std::unordered_map<Page*, Page*> map_for_prev_page;

    //parent -> overflow page
    std::unordered_map<Page*, Page*> overflow_map;
    float load_capacity;


    //static constexpr double KNUTH_NUMBER = 1054997077.39;

public:
  HashIndex(float load_capacity);
  uint32_t hash(uint64_t);
  std::pair<bool,uint64_t> search(uint64_t, std::ifstream&);
  std::pair<bool,uint64_t> search(uint64_t, FILE*);
  std::pair<bool,uint64_t> search(uint64_t, int);
  void build_index(std::string, std::string);
  static void debugRead(std::string);
  std::vector<DataEntry> parse_idx_file(std::string path);
  ~HashIndex();
private:
  static uint64_t search(uint64_t, unsigned int);
  static uint32_t hash(uint64_t, unsigned int);
  int merge(std::vector<Page*>&, std::vector<Page*>&);
  bool page_has_overflow(Page*);
};
