#include "BTree.h"
#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
using namespace std;

BTree::BTree(float load_capacity) {
  this->load_capacity = load_capacity;
  max_level = 0;
}

/**
 * offset of page = (2^level + pos in level)
 */
vector<Page*> BTree::build_level(vector<Page*> children_level) {
  vector<Page*> cur_level;
  for (int i=0; i < children_level.size(); i++) {
    Page* child = children_level.at(i);
    uint64_t add_key = child->leftover_key;
  }


  return cur_level;
}

void BTree::set_max_level(uint64_t total_records) {
  this->total_records = total_records;
  this->max_level = ceil(log((total_records-1)/Page::MAX_ENTRIES)/log(Page::MAX_ENTRIES))+1;
}

vector<Page*> BTree::get_leaf_pages(vector<DataEntry> entries) {
  set_max_level(entries.size());
  vector<Page*> level;
  size_t copied_entries_size = 0;
  while (copied_entries_size < entries.size()) {
    size_t leng_to_copy = Page::MAX_ENTRIES;
    if (entries.size() - copied_entries_size < Page::MAX_ENTRIES) {
      leng_to_copy = (entries.size() - copied_entries_size);
    }
    vector<DataEntry> page_entries(entries.begin() + copied_entries_size, entries.begin()+copied_entries_size+leng_to_copy);
    Page *page = new Page(page_entries);
    level.push_back(page);
    copied_entries_size += leng_to_copy;
  }
  return level;
}


/*
 * parser that expects a file with 3 tab-delimited columns
 * with the following format: key\tcount\trid where the
 * middle value count is ignored
 */
vector<DataEntry> BTree::parse_idx_file(string path) {
  vector<DataEntry> data_entries;
  ifstream input(path);
  char const row_delim = '\n';
  string const field_delim = "\t";
  for (string row; getline(input, row, row_delim);) {
    istringstream ss(row);

    //read in key
    auto start = 0U;
    auto end = row.find(field_delim);
    DataEntry entry (0, 0);
    ss.clear();
    ss.str(row.substr(start, end - start));
    if (!(ss >> entry.key)) {
      cout << "read key failed" << endl;
      cout << row.substr(start, end - start) << endl;
      continue;
    } else {
      //cout << "| " << entry.key << " ";
    }

    /*
    //ignore count for now
    start = end + field_delim.length();
    end = row.find(field_delim, start);
    */
    //read in rid
    start = end + field_delim.length();
    end = row.find(field_delim, start);
    ss.clear();
    ss.str(row.substr(start, end - start));
    if (!(ss >> entry.rid)) {
      cout << "read rid failed" << endl;
      cout << row.substr(start, end - start) << endl;
      continue;
    } else {
      //cout << entry.rid << " |" << endl;
    }
    data_entries.push_back(entry);
  }


  return data_entries;
}

