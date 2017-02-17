#include <iostream>
#include <fstream>
#include <vector>
#include "DataEntry.h"

class Page {
  public:
    // (total buckets) + overflow page index starting from 0
    uint64_t overflow_addr;
    const static int MAX_ENTRIES = 255;
    //const static int MAX_ENTRIES = 2;
    uint32_t counter;
    DataEntry data_entry_list[MAX_ENTRIES];

    //used for debug print bucket record counts
    bool overflow_merged;

  public:
    Page();
    Page(std::vector<DataEntry>);
    Page(Page&& other);
    void addEntry(DataEntry);
    std::ofstream& flush(std::ofstream&);
    static void read(std::ifstream&, Page&);
    bool isFull();
    bool hasOverflow();
    void setOverflow(uint64_t overflow_addr);
    void sortEntries();
    std::pair<bool,uint64_t> find(uint64_t);
    void mergePage(Page& other);
};

