#include <iostream>
#include <vector>
#include "DataEntry.h"

class Page {
  public:
    uint32_t overflow_addr;
    const static int MAX_ENTRIES = 255;

  private:
    uint32_t counter;
    std::vector<DataEntry> data_entry_list;


  public:
    Page();
    void addEntry(DataEntry);
    void flush(FILE* fp);
    static Page read(FILE* fp);
    bool isFull();
};
