#include <cstring>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "BTreePage.h"

class BTree {
public:
  static const int PAGE_SIZE = 4096;
private:
  int max_level;
  std::unordered_map<Page*, std::vector<Page>> page_to_vector;
  std::vector<std::vector<Page*>> btree;
  uint64_t total_entries;
  const static int fan_out = Page::MAX_ENTRIES;

public:
 BTree();
 std::vector<Page*> build_level(std::vector<DataEntry>);
 std::vector<DataEntry> parse_idx_file(std::string);


};
