#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include "DataEntry.h"

using namespace std;

class BTreePage {
  public:
    constexpr static unsigned int PAGE_SIZE = 64;

    //page_size/16 -1 (1 for counter + extra rid)
    constexpr static unsigned int MAX_KEY_PER_PAGE = PAGE_SIZE*1.0/(2*sizeof(uint64_t))-1;
    constexpr static unsigned int fan_out = MAX_KEY_PER_PAGE+1;
    //const static unsigned int PAGE_SIZE = 4096;
    uint64_t pageNum = 0;
    int numChildRefs = 0;
    vector<BTreePage*> children;
    vector<uint64_t> keys;
    vector<uint64_t> rids;
    BTreePage* parent;
    unsigned int level = -1;


    //static constexpr double KNUTH_NUMBER = 1054997077.39;

public:
  BTreePage();
  pair<bool,uint64_t> find(uint64_t key);
  void setParent(BTreePage*);
  void addKey(uint64_t key);
  void addChild(BTreePage*);
  bool isFull();
  ofstream& flush(ofstream&);
  static void read(FILE*, BTreePage&, bool);
  ~BTreePage();
};
