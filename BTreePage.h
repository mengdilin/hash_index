#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include "DataEntry.h"

using namespace std;

class BTreePage {
  public:

    //size in bytes of a btree index page
    constexpr static unsigned int PAGE_SIZE = 4096;

    //page_size/16 -1 (1 for counter + extra rid)
    constexpr static unsigned int MAX_KEY_PER_PAGE = PAGE_SIZE*1.0/(2*sizeof(uint64_t))-1;
    constexpr static unsigned int fan_out = MAX_KEY_PER_PAGE+1;

    //page offset for the current page
    uint64_t pageNum = 0;

    //number of children in current page
    int numChildRefs = 0;

    //pointers to the children page
    vector<BTreePage*> children;

    //keys in the current page
    vector<uint64_t> keys;

    /* pageNum of the children page, if current page is not leaf
     * rids of keys, if current page is leaf
     */
    vector<uint64_t> rids;

    //pointer to parent page
    BTreePage* parent;

    //current level of the page
    unsigned int level = 0;

public:
  BTreePage();
  pair<bool,uint64_t> find(uint64_t key);
  void setParent(BTreePage*);
  void addKey(uint64_t key);
  void addChild(BTreePage*);
  bool isFull();
  ofstream& flush(ofstream&);
  static void read(int, BTreePage&, bool, off_t);

  ~BTreePage();
};
