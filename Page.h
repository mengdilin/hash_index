#include <iostream>
#include <fstream>
#include <vector>
#include "DataEntry.h"
#include <stdlib.h>


class Page {
  public:
    // (total buckets) + overflow page index starting from 0
    uint64_t overflow_addr;

    static const int PAGE_SIZE = 64;
    static constexpr int MAX_ENTRIES = (Page::PAGE_SIZE-2*sizeof(uint64_t))/(2*sizeof(uint64_t));

    uint32_t counter; //number of data entries in this page
    uint32_t hash; //address of this page. Used when setting overflow addresses

    /* data_entry_list is only used during index building stage*/
    DataEntry* data_entry_list = nullptr;

    /* buffer is only used during probing stage*/
    uint8_t* buffer = nullptr;

    //used for debug print bucket record counts
    bool overflow_merged;


  public:
    Page();
    Page(std::vector<DataEntry>);
    ~Page();
    void addEntry(DataEntry);
    std::ofstream& flush(std::ofstream&);
    static void read(std::ifstream&, Page&);
    static void read(FILE*, Page&);
    static void read(int, Page&, uint64_t);

    bool isFull();
    bool hasOverflow();
    void setOverflow(uint64_t overflow_addr);
    void sortEntries();
    std::pair<bool,uint64_t> find(uint64_t);
    void mergePage(Page& other);
};

