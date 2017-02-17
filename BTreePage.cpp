#include "BTreePage.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <assert.h>
#include <algorithm>
using namespace std;
Page::Page() {
  counter = 0;
}

Page::Page(vector<DataEntry> entries) {
  assert(entries.size()-1 <= Page::MAX_ENTRIES);
  counter = entries.size()-1;
  vector<DataEntry> tmp(entries.begin(), entries.end()-1);

  data_entry_list = tmp;
  leftover_key = (entries.end()-1)->key;
  //cout << "first key: " << entries.begin()->key << endl;
  //cout << "key: " << data_entry_list.at(counter-1).key << endl;
  //cout << "leftover: " << leftover_key << endl;

}


void Page::addEntry(DataEntry entry) {
  assert(counter < MAX_ENTRIES);
  data_entry_list[counter] = entry;
  counter++;
}

bool Page::isFull() {
  return counter == MAX_ENTRIES;
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

pair<bool,uint64_t> Page::find(uint64_t key) {
  DataEntry look_for(key, 0);

  // lower_bound implements binary search
  auto result = lower_bound(
    data_entry_list.begin(),
    data_entry_list.end(),
    look_for,
    DataEntry::compare);
  pair <bool,uint64_t> find_result;
  if (result == data_entry_list.end()) {
    // did not find key in Page
    find_result = make_pair(false, 0);
  } else {
    //cout << "prev: " << (result-1)->rid << endl;
    find_result = make_pair(true, result->rid);
  }

  return find_result;
}

void Page::sortEntries() {
  //Now we call the sort function
  sort(data_entry_list.begin(), data_entry_list.end(), DataEntry::compare);
}

ofstream& Page::flush(ofstream& indexFile) {
  indexFile.write((char*) &counter, sizeof(counter));
  uint32_t pad = 0;
  indexFile.write((char*) &pad, sizeof(pad));
  indexFile.write((char*) &pad, sizeof(pad));
  indexFile.write((char*) &pad, sizeof(pad));
  for (DataEntry entry : data_entry_list) {
    entry.flush(indexFile);
  }
  return indexFile;
}

void Page::read(std::ifstream& indexFile, Page& page) {

  uint32_t pad;
  indexFile.read ((char *)&page.counter,sizeof(page.counter));

  indexFile.read ((char *)&pad,sizeof(uint32_t));
  indexFile.read ((char *)&pad,sizeof(uint32_t));
  indexFile.read ((char *)&pad,sizeof(uint32_t));


  for (int i = 0; i < page.counter; i++) {
    DataEntry cur = DataEntry::read(indexFile);
    page.data_entry_list.push_back(cur);

  }

  cout << "couter: " << page.counter << endl;
  for (int i = 0; i < page.counter; i++) {
    cout << "(" << page.data_entry_list[i].key << " ," << page.data_entry_list[i].rid << " )" << endl;
  }

}

std::ostream& operator<<(std::ostream &strm, const Page &a) {
  return strm << "counter: " << a.counter;
}
