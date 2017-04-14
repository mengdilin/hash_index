#include "Page.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <assert.h>
#include <algorithm>
#include <unistd.h>
using namespace std;
Page::Page() {
  overflow_addr = 0x00;
  counter = 0;
  overflow_merged = false;
}

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
Page::~Page() {

 if (buffer != nullptr) {
    free(buffer);
  } else {
    free(data_entry_list);
  }
}
void Page::addEntry(DataEntry entry) {
  assert(counter < MAX_ENTRIES);
  data_entry_list[counter] = entry;
  counter++;
}

bool Page::isFull() {
  return counter == MAX_ENTRIES;
}

bool Page::hasOverflow() {
  return overflow_addr != 0x00;
}

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
  if (result == data_entry_list + counter) {
    // did not find key in Page
    find_result = make_pair(false, 0);
  } else {
    find_result = make_pair(true, result->rid);
  }

  return find_result;
}

void Page::sortEntries() {
  sort(data_entry_list, data_entry_list + counter, DataEntry::compare);
}

ofstream& Page::flush(ofstream& indexFile) {
  indexFile.write((char*) &overflow_addr, sizeof(overflow_addr));
  uint32_t pad = 0;
  //indexFile.write((char*) &pad, sizeof(pad));
  indexFile.write((char*) &counter, sizeof(counter));
  indexFile.write((char*) &pad, sizeof(pad));
  for (int i = 0; i < MAX_ENTRIES; i++) {
    data_entry_list[i].flush(indexFile);
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

}

void Page::read(int indexFile, Page& page, uint64_t offset) {
  page.buffer = (uint8_t *)malloc(PAGE_SIZE);
  pread(indexFile, page.buffer, PAGE_SIZE, offset);
  page.overflow_addr = *((uint64_t*)page.buffer);
  page.counter = *((uint64_t*)&page.buffer[8]);
  page.data_entry_list = (DataEntry*)&page.buffer[16];

}
void Page::read(FILE* indexFile, Page& page) {
  page.buffer = (uint8_t *)malloc(PAGE_SIZE);

  /*
  char *buffer = (char *)malloc(PAGE_SIZE);
  // read 4K block into memory
  fread(buffer, PAGE_SIZE, 1, indexFile);
  uint32_t pad;
  memcpy(&page.overflow_addr, buffer, sizeof(page.overflow_addr));
  memcpy(&page.counter, buffer+sizeof(page.overflow_addr), sizeof(page.counter));

  memcpy(&pad, buffer+sizeof(page.overflow_addr)+sizeof(page.counter), sizeof(pad));

  memcpy(&page.data_entry_list, buffer+sizeof(page.overflow_addr)+sizeof(page.counter)+sizeof(pad), sizeof(page.data_entry_list));
  free(buffer);

  */

  /*
  uint32_t pad;
  fread(&page.overflow_addr, sizeof(page.overflow_addr), 1, indexFile);
  fread(&page.counter, sizeof(page.counter), 1, indexFile);
  fread(&pad, sizeof(pad), 1, indexFile);
  fread(&page.data_entry_list, sizeof(page.data_entry_list), 1, indexFile);
  */
  fread(page.buffer, PAGE_SIZE, 1, indexFile);
  page.overflow_addr = *((uint64_t*)page.buffer);
  page.counter = *((uint64_t*)&page.buffer[8]);
  page.data_entry_list = (DataEntry*)&page.buffer[16];
}


std::ostream& operator<<(std::ostream &strm, const Page &a) {
  return strm << "counter: " << a.counter;
}
