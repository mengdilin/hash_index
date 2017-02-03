#include <iostream>
#include <fstream>
#include <vector>
#include "DataEntry.h"

class Page {
  public:
    uint32_t overflow_addr;
    const static int MAX_ENTRIES = 255;
    uint32_t counter;
  private:
    std::vector<DataEntry> data_entry_list;

  public:
    Page();
    Page(Page&& other);
    void addEntry(DataEntry);
    std::ofstream& flush(std::ofstream&);
    static Page read(std::ifstream&);
    bool isFull();

};

