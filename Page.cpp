#include "Page.h"

Page::Page() {
  overflow_addr = 0x00;
  counter = 0;
}

void Page::addEntry(DataEntry entry) {
  counter++;
  data_entry_list.push_back(entry);
}

bool Page::isFull() {
  return counter == MAX_ENTRIES;
}

void Page::flush(FILE* fp) {

}

Page Page::read(FILE *fp) {
  Page page;
  return page;
}

uint32_t Page::getCounter() const {
  return this->counter;
}

std::ostream& operator<<(std::ostream &strm, const Page &a) {
  return strm << "counter: " << a.getCounter();
}
