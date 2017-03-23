#include "BTreePage.h"
#include <cassert>
#include <fstream>
#include <iostream>

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
ofstream& BTreePage::flush(ofstream& indexFile) {
  uint64_t counter = keys.size();
  indexFile.write((char*) &counter, sizeof(counter));

  uint64_t *key_array = &keys[0];
  indexFile.write((char*)key_array, MAX_KEY_PER_PAGE * sizeof(uint64_t));
  uint64_t *rid_array = &rids[0];
  indexFile.write((char*)rid_array, (MAX_KEY_PER_PAGE + 1)*sizeof(uint64_t));
  return indexFile;
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

  // lower_bound implements binary search
  auto result = lower_bound(
    keys.begin(),
    keys.end(),
    key);
  pair <bool,uint64_t> find_result;
  if (result == keys.end()) {
    // did not find key in Page
    find_result = make_pair(true, rids.at(rids.size()-1));
  } else {
    /*
    cout << "found index: " << result-keys.begin() << endl;
    cout << "key for that index: " << keys.at((result-keys.begin())) << endl;
    cout << "rid size: " << rids.size() << endl;
    cout << "found rid: " << rids.at(result-keys.begin()) << endl;

    for (int i = 0; i < keys.size(); i++) {
      cout << keys.at(i) << " \t";
    }
    cout << endl;
    */
    find_result = make_pair(true,  rids.at(result-keys.begin()));
  }

  return find_result;
}


void BTreePage::setParent(BTreePage* p) {
  this->parent = p;
}

void BTreePage::addKey(uint64_t key) {
  assert(keys.size() < MAX_KEY_PER_PAGE);
  keys.push_back(key);
}
bool BTreePage::isFull() {
  return keys.size() == MAX_KEY_PER_PAGE;
}



