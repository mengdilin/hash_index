#include <iostream>
#include <fstream>
#include <vector>
#include "DataEntry.h"
#include <stdlib.h>


class Page {
  public:
    // (total buckets) + overflow page index starting from 0
    uint64_t overflow_addr;
    //const static int MAX_ENTRIES = 255;
    //const static int MAX_ENTRIES = 2;
    static const int PAGE_SIZE = 516;
    static constexpr int MAX_ENTRIES = (Page::PAGE_SIZE-2*sizeof(uint64_t))/(2*sizeof(uint64_t));

    //static const int PAGE_SIZE = 48;
    uint32_t counter;
    DataEntry data_entry_list[MAX_ENTRIES];
    uint32_t hash;
    //used for debug print bucket record counts
    bool overflow_merged;

  public:
    Page();
    Page(std::vector<DataEntry>);
    void addEntry(DataEntry);
    std::ofstream& flush(std::ofstream&);
    static void read(std::ifstream&, Page&);
    static void read(FILE*, Page&);
    bool isFull();
    bool hasOverflow();
    void setOverflow(uint64_t overflow_addr);
    void sortEntries();
    std::pair<bool,uint64_t> find(uint64_t);
    void mergePage(Page& other);
};

