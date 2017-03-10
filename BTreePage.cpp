#include "BTreePage.h"
#include <cassert>
using namespace std;

BTreePage::BTreePage() {
  this->parent = nullptr;
}

void BTreePage::addChild(BTreePage* c) {
  assert(children.size() < fan_out);
  children.push_back(c);
}

BTreePage::~BTreePage() {

}

template<class Iter, class T>
Iter binary_find(Iter begin, Iter end, T val)
{
    // Finds the lower bound in at most log(last - first) + 1 comparisons
    Iter i = lower_bound(begin, end, val);

    if (i != end && !(val < *i))
        return i; // found
    else
        return end; // not found
}

pair<bool,uint64_t> BTreePage::find(uint64_t key) {
  DataEntry look_for(key, 0);

  // lower_bound implements binary search
  DataEntry* result = lower_bound(
    keys.begin(),
    keys.end(),
    look_for,
    DataEntry::compare);
  pair <bool,uint64_t> find_result;
  if (result == data_entry_list + counter) {
    // did not find key in Page
    find_result = make_pair(false, 0);
  } else {
    //cout << "prev: " << (result-1)->rid << endl;
    find_result = make_pair(true, result->rid);
  }

  return find_result;
}


void BTreePage::setParent(BTreePage* p) {
  this->parent = p;
}

void BTreePage::addKey(DataEntry key) {
  assert(keys.size() < MAX_KEY_PER_PAGE);
  keys.push_back(key);
}
bool BTreePage::isFull() {
  return keys.size() == MAX_KEY_PER_PAGE;
}



