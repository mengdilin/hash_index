#include "Page.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <assert.h>
#include <algorithm>
#include <unistd.h>
using namespace std;

/**
 * @brief constructor for a Page used for probing
 */
Page::Page() {
  overflow_addr = 0x00; //overflow_addr of 0 means no overflow
  counter = 0;
  overflow_merged = false;
}

/**
 * @brief constructor for a Page used for index building
 */
Page::Page(vector<DataEntry> entries) {
  assert(entries.size() <= Page::MAX_ENTRIES);
  overflow_addr = 0x00;
  counter = entries.size();
  data_entry_list = (DataEntry *)malloc(sizeof(DataEntry)*MAX_ENTRIES);
  overflow_merged = false;
  for (int i = 0; i < entries.size(); i++) {
    data_entry_list[i] = entries[i];
  }
}

/**
 * @brief destructor for a Page
 */
Page::~Page() {
 if (buffer != nullptr) {
  //this Page is meant for probing
    free(buffer);
  } else {
    //this Page is meant for index building
    free(data_entry_list);
  }
}

/**
 * @brief add an entry to the current page
 */
void Page::addEntry(DataEntry entry) {
  assert(counter < MAX_ENTRIES);
  data_entry_list[counter] = entry;
  counter++;
}

/**
 * @brief check whether current page is full
 */
bool Page::isFull() {
  return counter == MAX_ENTRIES;
}

/**
 * @brief check whether current page has overflow
 */
bool Page::hasOverflow() {
  return overflow_addr != 0x00;
}

/**
 * @brief set the overflow of the page
 */
void Page::setOverflow(uint64_t overflow) {
  overflow_addr = overflow;

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
  if (result == data_entry_list + counter || result->key != key) {
    // did not find key in Page
    find_result = make_pair(false, 0);
  } else {
    find_result = make_pair(true, result->rid);
  }

  return find_result;
}

/**
 * @brief Helper function used when merging a page with another page
 * all entries in the same page must be sorted
 */
void Page::sortEntries() {
  sort(data_entry_list, data_entry_list + counter, DataEntry::compare);
}

/**
 * @brief Helper function used during index building stage.
 * flushes a page to index file
 */
ofstream& Page::flush(ofstream& indexFile) {
  indexFile.write((char*) &overflow_addr, sizeof(overflow_addr));
  uint32_t pad = 0;
  indexFile.write((char*) &counter, sizeof(counter));
  indexFile.write((char*) &pad, sizeof(pad));
  for (int i = 0; i < MAX_ENTRIES; i++) {
    data_entry_list[i].flush(indexFile);
  }
  return indexFile;
}

/**
 * @brief Helper function used during index building stage.
 * merging a page with another page.
 */
void Page::mergePage(Page& other) {

  assert(MAX_ENTRIES - this->counter >= other.counter);
  for (int i = 0; i < other.counter; i ++) {
    DataEntry other_entry = other.data_entry_list[i];
    addEntry(other_entry);
  }
  sortEntries();
}

/**
 * @brief Helper function used during probing stage.
 * reading a page into memory using pread
 */
void Page::read(int indexFile, Page& page, uint64_t offset) {
  page.buffer = (uint8_t *)malloc(PAGE_SIZE);
  pread(indexFile, page.buffer, PAGE_SIZE, offset);
  page.overflow_addr = *((uint64_t*)page.buffer);
  page.counter = *((uint64_t*)&page.buffer[8]);
  page.data_entry_list = (DataEntry*)&page.buffer[16];
}


std::ostream& operator<<(std::ostream &strm, const Page &a) {
  return strm << "counter: " << a.counter;
}
