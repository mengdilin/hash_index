#include <iostream>
#include <vector>
#include <unordered_map>
#include "Page.h"
#include <stdio.h>

/**
 * @file HashIndex.h
 * @brief Format of HashIndex: unsigned int, Page, Page, Page, Page
 * unsigned int indicates the number of primary buckets in the hash index.
 * Page is a data structure defined in Page.h and Page.cpp
 * Pages are synonymous with buckets. The entries in a primary page and its
 * overflow pages are sorted in ascending order such that the entry with the
 * smallest key for the hash is always the first entry in the primary page and
 * the entry with the largest key for the hash is always the last entry in the
 * primary/overflow page
 */

class HashIndex {
  public:
    //number of primary buckets in the index
    unsigned int number_buckets;

  private:
    //a vector of all primary buckets/Pages
    std::vector<Page*> primary_buckets;

    //used to maintain a deterministic order of overflow pages
    std::vector<Page*> overflow_pages;

    //used to maintain overflow page -> parent relationship in index building
    std::unordered_map<Page*, Page*> map_for_prev_page;

    //used to maintain parent -> overflow page relationship in index building
    std::unordered_map<Page*, Page*> overflow_map;
    float load_capacity;


public:
  HashIndex(float load_capacity);
  uint32_t hash(uint64_t);
  //fastest probing method using pread
  std::pair<bool,uint64_t> search(uint64_t, int);
  void build_index(std::string, std::string);
  std::vector<DataEntry> parse_idx_file(std::string path);
  std::vector<DataEntry> parse_key_file(std::string path);

  ~HashIndex();
private:
  static uint64_t search(uint64_t, unsigned int);
  static uint32_t hash(uint64_t, unsigned int);
  int merge(std::vector<Page*>&, std::vector<Page*>&);
  bool page_has_overflow(Page*);
};
