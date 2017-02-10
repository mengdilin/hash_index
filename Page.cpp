#include "Page.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <assert.h>
#include <algorithm>
using namespace std;
Page::Page() {
  overflow_addr = 0x00;
  counter = 0;
  memset((void *)&data_entry_list, 0, sizeof(data_entry_list));
  //vector<DataEntry> tmp_data_entry_list(MAX_ENTRIES);
  //data_entry_list = tmp_data_entry_list;
  //memset((void *)&data_entry_list, 0, sizeof(data_entry_list));
}


Page::Page(Page&& other) {
  overflow_addr = other.overflow_addr;
  counter = other.counter;

  for (int i = 0; i < other.counter; i++) {
    data_entry_list[i] = other.data_entry_list[i];
  }

  other.overflow_addr = 0;
  other.counter = 0;
}

void Page::addEntry(DataEntry entry) {
  assert(counter < MAX_ENTRIES);
  data_entry_list[counter] = entry;
  counter++;
  //data_entry_list.push_back(entry);
}

bool Page::isFull() {
  return counter == MAX_ENTRIES;
}

bool Page::hasOverflow() {
  return overflow_addr != 0x00;
}

void Page::setOverflow(uint32_t overflow) {
  overflow_addr = overflow;
  //cout << "set overflow addr: " << overflow_addr << endl;

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
  DataEntry* result = lower_bound(
    data_entry_list,
    data_entry_list + counter,
    look_for,
    DataEntry::compare);
  pair <bool,uint64_t> find_result;
  if (result == data_entry_list + counter) {
    // did not find key in Page
    find_result = make_pair(false, 0);
  } else {
    find_result = make_pair(true, result->rid);
  }

  return find_result;
}

void Page::sortEntries() {
  //Now we call the sort function
  sort(data_entry_list, data_entry_list + counter, DataEntry::compare);
}

ofstream& Page::flush(ofstream& indexFile) {
  sortEntries();
  indexFile.write((char*) &overflow_addr, sizeof(overflow_addr));
  uint32_t pad = 0;
  indexFile.write((char*) &pad, sizeof(pad));
  indexFile.write((char*) &counter, sizeof(counter));
  indexFile.write((char*) &pad, sizeof(pad));
  for (DataEntry entry : data_entry_list) {
    entry.flush(indexFile);
  }
  return indexFile;
}


void Page::read(std::ifstream& indexFile, Page& page) {

  uint32_t pad;
  indexFile.read ((char *)&page.overflow_addr,sizeof(uint32_t));

  indexFile.read ((char *)&pad,sizeof(uint32_t));

  //indexFile.seekg(sizeof(uint32_t), indexFile.tellg());
  indexFile.read((char *)&page.counter, sizeof(uint32_t));
  //indexFile.seekg(sizeof(uint32_t), indexFile.tellg());
  indexFile.read ((char *)&pad,sizeof(uint32_t));


  indexFile.read((char *)&page.data_entry_list, sizeof(page.data_entry_list));
  /*
  cout << "couter: " << page.counter << endl;
  for (int i = 0; i < page.counter; i++) {
    cout << "(" << page.data_entry_list[i].key << " ," << page.data_entry_list[i].rid << " )" << endl;
  }
  */
}

std::ostream& operator<<(std::ostream &strm, const Page &a) {
  return strm << "counter: " << a.counter;
}
