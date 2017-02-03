#include "Page.h"
#include <iostream>
#include <fstream>

using namespace std;
Page::Page() {
  overflow_addr = 0x00;
  counter = 0;
}

Page::Page(Page&& other) {
  overflow_addr = other.overflow_addr;
  counter = other.counter;
  data_entry_list = move(other.data_entry_list);
}

void Page::addEntry(DataEntry entry) {
  counter++;
  data_entry_list.push_back(entry);
}

bool Page::isFull() {
  return counter == MAX_ENTRIES;
}

ofstream& Page::flush(ofstream& indexFile) {
  //cout << "overflow: " << overflow_addr << " counter: " << counter << endl;
  indexFile.write((char*) &overflow_addr, sizeof(overflow_addr));
  indexFile.write((char*) &counter, sizeof(counter));
  for (DataEntry entry : data_entry_list) {
    entry.flush(indexFile);
  }
  return indexFile;
}


Page Page::read(std::ifstream& indexFile) {
  Page page;
  int count;

  indexFile.read ((char *)&page.overflow_addr,sizeof(uint32_t));
  indexFile.read((char *)&count, sizeof(uint32_t));

  //cout << "overflow: " << page.overflow_addr << " counter: " << count << endl;

  while (count != 0) {
    DataEntry entry = DataEntry::read(indexFile);
    page.addEntry(entry);
    count--;
  }
  cout << endl;
  return page;
}

std::ostream& operator<<(std::ostream &strm, const Page &a) {
  return strm << "counter: " << a.counter;
}
