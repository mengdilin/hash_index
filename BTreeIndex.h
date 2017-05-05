#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <utility>
#include "BTreePage.h"

using namespace std;

class BTreeIndex {
  public:
    BTreePage root;
    vector<vector<BTreePage*>> tree;
    vector<uint64_t> keys_per_level;
    vector<uint64_t> fanout_per_level;
    unsigned int max_level = 20; //fits 824633720832 keys at leaf

public:
  BTreeIndex();
  vector<DataEntry> parse_idx_file(string);
  vector<DataEntry> parse_idx_file_get_all(string);
  void build_tree(vector<DataEntry>);
  void setPageOffset();
  void debugPrint();
  void BfsDebugPrint();
  void debugPrint(BTreePage*);
  void addNodeToTree(int, BTreePage*);
  void test_page_read(FILE*);
  void flush(string);
  void probe(uint64_t key, vector<BTreePage*> stream);
  vector<BTreePage*> get_simulated_stream();
  pair<bool, uint64_t> probe(uint64_t, FILE* );
  pair<bool, uint64_t> probe(uint64_t, int, int);

  pair<bool, uint64_t> probe_bin(uint64_t, int, off_t);

 vector<pair<uint64_t, uint64_t>> range_probe_gt(uint64_t, int, int, off_t );
 vector<pair<uint64_t, uint64_t>> range_probe_lt(uint64_t , int , int );
 vector<pair<uint64_t, uint64_t>> range_probe_endpts(uint64_t, uint64_t, int, int, off_t);

  vector<pair<uint64_t, uint64_t>> range_probe_bin(int , off_t , off_t );
  ~BTreeIndex();

};

