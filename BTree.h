#include <cstring>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "BTreePage.h"

class BTree {
public:
  static const int PAGE_SIZE = 4096;
private:
  std::unordered_map<int, std::vector<Page*> > level_map;
  float load_capacity;
  int max_level;
  uint64_t total_records;
  int fan_out;

public:
 BTree(float);
 std::vector<Page*> build_level(std::vector<Page*> );
 std::vector<DataEntry> parse_idx_file(std::string);
 std::vector<Page*> get_leaf_pages(std::vector<DataEntry>);
 void set_max_level(uint64_t);


};
