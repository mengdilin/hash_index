#include "HashIndex.h"
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <vector>
#include <math.h>

using namespace std;
HashIndex::HashIndex(float load_capacity) {
  number_buckets = 0;
  this->load_capacity = load_capacity;
}

uint32_t HashIndex::hash(uint64_t key) {
  key ^= key >> 33;
  key *= 0xff51afd7ed558ccdULL;
  key ^= key >> 33;
  key *= 0xc4ceb9fe1a85ec53ULL;
  key ^= key >> 33;
  key = key >> ((64 - (int)log2(this->number_buckets)) + 1);
  return key * UINT32_C(2654435761) % this->number_buckets;;
}

uint64_t HashIndex::search(uint64_t key) {
  return 0;
}

void HashIndex::build_index(string path) {
  vector<DataEntry> entries = this->parse_idx_file(path);
  for (DataEntry entry : entries) {
    cout << this->hash(entry.key) << endl;
  }

}

vector<DataEntry> HashIndex::parse_idx_file(string path) {
  vector<DataEntry> data_entries;
  ifstream input(path);
  char const row_delim = '\n';
  string const field_delim = "\t";
  for (string row; getline(input, row, row_delim);) {
    istringstream ss(row);

    //read in key
    auto start = 0U;
    auto end = row.find(field_delim);
    DataEntry entry;
    ss.clear();
    ss.str(row.substr(start, end - start));
    if (!(ss >> entry.key)) {
      cout << "read key failed" << endl;
      cout << row.substr(start, end - start) << endl;
      continue;
    } else {
      //cout << entry.key << " ";
    }

    //ignore count for now
    start = end + field_delim.length();
    end = row.find(field_delim, start);

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
      //cout << entry.rid << endl;
    }
    data_entries.push_back(entry);
  }

  this->number_buckets = ceil(((float)data_entries.size()/(float)Page::MAX_ENTRIES)/(float)this->load_capacity);
  cout << "num buckets: " << this->number_buckets << endl;
  cout << "entry size: " << data_entries.size() << endl;
  return data_entries;
}
