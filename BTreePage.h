#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include "DataEntry.h"

using namespace std;

class BTreePage {
  public:
    constexpr static unsigned int PAGE_SIZE = 40;

    constexpr static unsigned int MAX_KEY_PER_PAGE = (PAGE_SIZE*1.0/(sizeof(uint64_t))-1)/2.0;
    constexpr static unsigned int fan_out = MAX_KEY_PER_PAGE+1;
    //const static unsigned int PAGE_SIZE = 4096;
    vector<BTreePage*> children;
    vector<DataEntry> keys;
    BTreePage* parent;
    unsigned int counter = 0;
    unsigned int level = -1;

    //static constexpr double KNUTH_NUMBER = 1054997077.39;

public:
  BTreePage();
  void setParent(BTreePage*);
  void addKey(DataEntry);
  void addChild(BTreePage*);
  bool isFull();
  ~BTreePage();
};
