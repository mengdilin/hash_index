#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include "BTreePage.h"

using namespace std;

class BTreeIndex {
  public:
    BTreePage root;
    vector<vector<BTreePage*>> tree;
    vector<uint64_t> keys_per_level;
    vector<uint64_t> fanout_per_level;
    unsigned int max_level = 19; //arbitrarily set

public:
  BTreeIndex();
  vector<DataEntry> parse_idx_file(string);
  void build_tree(vector<DataEntry>);
  void setPageOffset();
  void debugPrint();
  void BfsDebugPrint();
  void debugPrint(BTreePage*);
  void addNodeToTree(int, BTreePage*);
  void flush(string);
  void probe(uint64_t key, vector<BTreePage*> stream);
  vector<BTreePage*> get_simulated_stream();
  ~BTreeIndex();

};


