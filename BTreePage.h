#include <iostream>
#include <fstream>
#include <vector>
#include "DataEntry.h"

class Page {
  public:
    const static int MAX_ENTRIES = 255;
    //const static int MAX_ENTRIES = 2;
    std::vector<DataEntry> data_entry_list;
    int counter;
    Page* parent;
    bool has_parent = false;
    int cur_level;


  public:
    Page();
    Page(std::vector<DataEntry>);
    Page* getParent();
    void setParent(Page*);
    void addEntry(DataEntry);
    std::ofstream& flush(std::ofstream&);
    static void read(std::ifstream&, Page&);
    bool isFull();
    void sortEntries();
    std::pair<bool,uint64_t> find(uint64_t);
};

