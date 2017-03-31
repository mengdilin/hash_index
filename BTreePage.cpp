#include "BTreePage.h"
#include <cassert>
#include <fstream>
#include <iostream>
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
  //indexFile.write((char*)&pageNum, sizeof(pageNum));

  //uint64_t *key_array = &keys[0];
  for (auto& key : keys) {
    cout << "writing: " << key << endl;
    indexFile.write((char*) &key, sizeof(uint64_t));
  }
  uint64_t pad = 0;
  if (counter < MAX_KEY_PER_PAGE) {
    //cout << "here" << endl;
    indexFile.write((char*) &pad, sizeof(uint64_t));
    counter ++;
  }
  //indexFile.write((char*)key_array, MAX_KEY_PER_PAGE * sizeof(uint64_t));
  uint64_t *rid_array = &rids[0];

  for (auto& rid : rids) {
    indexFile.write((char*) &rid, sizeof(uint64_t));
  }
  int size_of_rids = rids.size();
  if (size_of_rids < MAX_KEY_PER_PAGE + 1) {
    //cout << "here" << endl;
    indexFile.write((char*) &pad, sizeof(uint64_t));
    size_of_rids ++;
  }

  //indexFile.write((char*)rid_array, (MAX_KEY_PER_PAGE + 1)*sizeof(uint64_t));
  return indexFile;
}

void BTreePage::read(FILE* indexFile, BTreePage& page) {
  uint64_t counter;
  fread(&counter, sizeof(counter), 1, indexFile);

  cout << "counter: " << counter << endl;

  uint64_t tmp_keys[MAX_KEY_PER_PAGE];
  fread(&tmp_keys, sizeof(tmp_keys), 1, indexFile);
  for (int i = 0; i < MAX_KEY_PER_PAGE; i ++) {
    cout << tmp_keys[i] << endl;
  }
  page.keys.assign(tmp_keys, tmp_keys+counter);
  uint64_t tmp_rids[MAX_KEY_PER_PAGE+1];
  fread(&tmp_rids, sizeof(tmp_rids), 1, indexFile);
  page.rids.assign(tmp_rids, tmp_rids+counter+1);
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



