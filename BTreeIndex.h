#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <utility>
#include "BTreePage.h"

/**
 * @file BTreeIndex.h
 * @brief this header file will contain all required definitions
 * and basic utility functions for Btree Index
 */
using namespace std;

class BTreeIndex {
  public:
    BTreePage root;
    vector<vector<BTreePage*>> tree;
    vector<uint64_t> keys_per_level;
    vector<uint64_t> fanout_per_level;
    unsigned int max_level = 20; //arbitrarily set-> max level of a tree

public:


  BTreeIndex();
  vector<DataEntry> parse_idx_file(string);
  vector<DataEntry> parse_idx_file_get_all(string);
  vector<DataEntry> parse_key_file(std::string path);
  vector<pair<DataEntry, int>> parse_sample_range_probe_idx(string path);

  void build_tree(vector<DataEntry>);
  void setPageOffset();
  void BfsDebugPrint();
  void addNodeToTree(int, BTreePage*);
  void flush(string);
  void probe(uint64_t key, vector<BTreePage*> stream);
  pair<bool, uint64_t> probe(uint64_t, int, int);

  pair<bool, uint64_t> probe_bin(uint64_t, int, off_t);

 vector<pair<uint64_t, uint64_t>> range_probe_gt(uint64_t, int, int, off_t );
 vector<pair<uint64_t, uint64_t>> range_probe_lt(uint64_t , int , int );
 vector<pair<uint64_t, uint64_t>> range_probe_endpts(uint64_t, uint64_t, int, int, off_t);

  vector<pair<uint64_t, uint64_t>> range_probe_bin(int , off_t , off_t );
  ~BTreeIndex();

};


