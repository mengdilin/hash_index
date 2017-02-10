#include <iostream>
#include <fstream>
#include <vector>
#include "DataEntry.h"

class Page {
  public:
    uint32_t overflow_addr;
    const static int MAX_ENTRIES = 255;
    //const static int MAX_ENTRIES = 2;
    uint32_t counter;
  private:
    DataEntry data_entry_list[MAX_ENTRIES];

  public:
    Page();
    Page(Page&& other);
    void addEntry(DataEntry);
    std::ofstream& flush(std::ofstream&);
    static void read(std::ifstream&, Page&);
    bool isFull();
    bool hasOverflow();
    void setOverflow(uint32_t overflow_addr);
    void sortEntries();
    std::pair<bool,uint64_t> find(uint64_t);

};

