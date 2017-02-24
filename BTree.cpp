#include "BTree.h"
#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
using namespace std;

BTree::BTree() {
  max_level = 0;
}

vector<Page*> BTree::build_level(vector<DataEntry> data_entries) {
  total_entries = data_entries.size();
  Page cur_page;
  vector<Page*> siblings;
  int level = 0;
  for (int i = 0; i < data_entries.size(); i++) {
    if (!cur_page.isFull()) {
      cur_page.addEntry(data_entries.at(i));
    } else {
      siblings.push_back(cur_page);

    }
  }

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

