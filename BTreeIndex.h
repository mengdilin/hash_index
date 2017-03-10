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
    vector<int> keys_per_level;
    unsigned int max_level = 19; //arbitrarily set

public:
  BTreeIndex();
  vector<DataEntry> parse_idx_file(string);
  void build_tree(vector<DataEntry>);
  void debugPrint();
  void addNodeToTree(int, BTreePage*);
  ~BTreeIndex();

};


