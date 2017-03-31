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
  auto before = indexFile.tellp();
  uint64_t counter = keys.size();
  indexFile.write((char*) &counter, sizeof(counter));
  //indexFile.write((char*)&pageNum, sizeof(pageNum));
  //cout << "keys before: " << counter << endl;
  //uint64_t *key_array = &keys[0];
  for (auto& key : keys) {
    //cout << "writing: " << key << endl;
    indexFile.write((char*) &key, sizeof(uint64_t));
  }
  uint64_t pad = 0;
  while (counter < MAX_KEY_PER_PAGE) {
    //cout << "here" << endl;
    indexFile.write((char*) &pad, sizeof(uint64_t));
    counter ++;
  }
  //cout << "total keys written: " << counter << endl;
  //indexFile.write((char*)key_array, MAX_KEY_PER_PAGE * sizeof(uint64_t));

  for (auto& rid : rids) {
    indexFile.write((char*) &rid, sizeof(uint64_t));
  }

  int size_of_rids = rids.size();

  //cout << "page num: " << pageNum << endl;
  //cout << "num child refs: " << numChildRefs << endl;
  //cout << "rids before: " << size_of_rids << endl;
  while (size_of_rids < MAX_KEY_PER_PAGE + 1) {
    //cout << "here" << endl;
    indexFile.write((char*) &pad, sizeof(uint64_t));
    size_of_rids ++;
  }
  /*
  cout << "counter: " << counter << endl;
  cout << "size of rid: " << size_of_rids << endl;
  cout << "max key per page: " << MAX_KEY_PER_PAGE << endl;
  cout << "size of keys: " << counter << endl;
  */
  auto after = indexFile.tellp();
  //cout << "bytes written: " << after - before << endl;
  if (after - before != BTreePage::PAGE_SIZE) {
    cout << "bytes written: " << after - before << endl;
    cout << "page num: " << pageNum << endl;
    cout << "total rids written: " << size_of_rids << endl;
    cout << "total keys written: " << counter << endl;
  }
  return indexFile;
}

void BTreePage::read(FILE* indexFile, BTreePage& page, bool is_leaf) {
  uint64_t counter;
  fread(&counter, sizeof(counter), 1, indexFile);
  //fread(&counter, sizeof(counter), 1, indexFile);

  cout << "counter: " << counter << endl;

  uint64_t tmp_keys[MAX_KEY_PER_PAGE];
  fread(&tmp_keys, sizeof(uint64_t)*(MAX_KEY_PER_PAGE), 1, indexFile);

  /*
  cout << "read keys: ";
  for (int i = 0; i < MAX_KEY_PER_PAGE; i++) {
    cout << tmp_keys[i] << ",";
  }
  cout << endl;
  */

  page.keys.assign(tmp_keys, tmp_keys+counter);

  /*
  cout << "keys: ";
  for (auto& key : page.keys) {
    cout << key << " ,";
  }
  cout << endl;
  */

  uint64_t tmp_rids[MAX_KEY_PER_PAGE+2];

  if (not is_leaf) {
    //cout << "rids read: " << MAX_KEY_PER_PAGE + 1 << endl;
    fread(&tmp_rids, sizeof(uint64_t)*(MAX_KEY_PER_PAGE+1), 1, indexFile);
    page.rids.assign(tmp_rids, tmp_rids+counter+1);
  } else {
    //cout << "leaf rids read: " << MAX_KEY_PER_PAGE +1 << endl;

    fread(&tmp_rids, sizeof(uint64_t)*(MAX_KEY_PER_PAGE+1), 1, indexFile);
    page.rids.assign(tmp_rids, tmp_rids+counter);
  }
  /*
  cout << "read rids: ";
  for (int i = 0; i < MAX_KEY_PER_PAGE+1; i++) {
    cout << tmp_rids[i] << ",";
  }
  cout << endl;
    cout << "rids: ";
  for (auto& rid : page.rids) {
    cout << rid << " ,";
  }
  cout << endl;
    cout << "read key size: " << page.keys.size() << endl;

    cout << "read rid size: " << page.rids.size() << endl;
  */

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



