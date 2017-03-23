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
  overflow_merged = false;
  memset((void *)&data_entry_list, 0, sizeof(data_entry_list));
  //vector<DataEntry> tmp_data_entry_list(MAX_ENTRIES);
  //data_entry_list = tmp_data_entry_list;
  //memset((void *)&data_entry_list, 0, sizeof(data_entry_list));
}

Page::Page(vector<DataEntry> entries) {
  assert(entries.size() <= Page::MAX_ENTRIES);
  overflow_addr = 0x00;
  counter = entries.size();
  overflow_merged = false;
  for (int i = 0; i < entries.size(); i++) {
    data_entry_list[i] = entries[i];
  }
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

void Page::setOverflow(uint64_t overflow) {
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
    //cout << "prev: " << (result-1)->rid << endl;
    find_result = make_pair(true, result->rid);
  }

  return find_result;
}

void Page::sortEntries() {
  //Now we call the sort function
  sort(data_entry_list, data_entry_list + counter, DataEntry::compare);
}

ofstream& Page::flush(ofstream& indexFile) {
  indexFile.write((char*) &overflow_addr, sizeof(overflow_addr));
  uint32_t pad = 0;
  //indexFile.write((char*) &pad, sizeof(pad));
  indexFile.write((char*) &counter, sizeof(counter));
  indexFile.write((char*) &pad, sizeof(pad));
  for (DataEntry entry : data_entry_list) {
    entry.flush(indexFile);
  }
  return indexFile;
}

void Page::mergePage(Page& other) {

  assert(MAX_ENTRIES - this->counter >= other.counter);
  for (int i = 0; i < other.counter; i ++) {
    DataEntry other_entry = other.data_entry_list[i];
    addEntry(other_entry);
  }
  sortEntries();
}

void Page::read(std::ifstream& indexFile, Page& page) {

  uint32_t pad;

  indexFile.read ((char *)&page.overflow_addr,sizeof(page.overflow_addr));
  indexFile.read((char *)&page.counter, sizeof(page.counter));
  indexFile.read ((char *)&pad,sizeof(uint32_t));
  indexFile.read((char *)&page.data_entry_list, sizeof(page.data_entry_list));


  /*
  char *buffer = (char *)malloc(PAGE_SIZE);
  indexFile.read(buffer, PAGE_SIZE);
  memcpy(&page.overflow_addr, buffer, sizeof(page.overflow_addr));
  memcpy(&page.counter, buffer+sizeof(page.overflow_addr), sizeof(page.counter));

  memcpy(&pad, buffer+sizeof(page.overflow_addr)+sizeof(page.counter), sizeof(pad));

  memcpy(&page.data_entry_list, buffer+sizeof(page.overflow_addr)+sizeof(page.counter)+sizeof(pad), sizeof(page.data_entry_list));

  cout << "couter: " << page.counter << endl;
  for (int i = 0; i < page.counter; i++) {
    cout << "(" << page.data_entry_list[i].key << " ," << page.data_entry_list[i].rid << " )" << endl;
  }
  */

}

void Page::read(FILE* indexFile, Page& page) {
  char *buffer = (char *)malloc(PAGE_SIZE);
  // read 4K block into memory
  fread(buffer, PAGE_SIZE, 1, indexFile);
  uint32_t pad;
  memcpy(&page.overflow_addr, buffer, sizeof(page.overflow_addr));
  memcpy(&page.counter, buffer+sizeof(page.overflow_addr), sizeof(page.counter));

  memcpy(&pad, buffer+sizeof(page.overflow_addr)+sizeof(page.counter), sizeof(pad));

  memcpy(&page.data_entry_list, buffer+sizeof(page.overflow_addr)+sizeof(page.counter)+sizeof(pad), sizeof(page.data_entry_list));

  /*
  cout << "couter: " << page.counter << endl;
  for (int i = 0; i < page.counter; i++) {
    cout << "(" << page.data_entry_list[i].key << " ," << page.data_entry_list[i].rid << " )" << endl;
  }
  */
  free(buffer);


}

std::ostream& operator<<(std::ostream &strm, const Page &a) {
  return strm << "counter: " << a.counter;
}