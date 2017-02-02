#include "HashIndex.h"
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <vector>
#include <math.h>

using namespace std;
HashIndex::HashIndex(float load_capacity) {
  this->load_capacity = load_capacity;
  this->numer_buckets = 0;
}

HashIndex::~HashIndex() {
}

uint32_t HashIndex::hash(uint64_t key) {
  key ^= key >> 33;
  key *= 0xff51afd7ed558ccdULL;
  key ^= key >> 33;
  key *= 0xc4ceb9fe1a85ec53ULL;
  key ^= key >> 33;
  key = key >> ((64 - (int)log2(this->number_buckets)) -1 );
  return key % this->number_buckets;
  //return key * UINT32_C(2654435761) % this->number_buckets;;
}

uint64_t HashIndex::search(uint64_t key) {
  return 0;
}

void HashIndex::build_index(string path) {
  vector<DataEntry> entries = this->parse_idx_file(path);

  // set number_buckets
  this->number_buckets = ceil(((float)entries.size()/(float)Page::MAX_ENTRIES)/(float)this->load_capacity);
  cout << "num buckets: " << this->number_buckets << endl;
  cout << "entry size: " << entries.size() << endl;

  //initialize the number of primary buckets

  vector<Page*> tmp_primary_buckets(this->number_buckets);
  primary_buckets = move(tmp_primary_buckets);

  vector<int> key_distribution(this->number_buckets);


  for (int i = 0; i < this->number_buckets; i++) {
    cout << i << endl;
    key_distribution.at(i) = 0;

  }
  for (int i = 0; i < this->number_buckets; i++) {
    primary_buckets.at(i) = nullptr;

  }
  for (DataEntry entry : entries) {
    uint32_t hash_key = this->hash(entry.key);
    if (hash_key >= this->number_buckets) {
      cout << "bad hash: " << hash_key << endl;
    } else {
      key_distribution[hash_key]++;
      cout << hash_key << " ";
      if (primary_buckets.at(hash_key) != nullptr) {
        Page* cur_page = primary_buckets[hash_key];
        cur_page->addEntry(entry);
      } else {
        Page* page = new Page();
        page->addEntry(entry);
        primary_buckets.at(hash_key) = page;
      }
    }


  }

  int counter = 0;

  for (int i = 0; i < this->number_buckets; i++) {
    cout << i << " , " << key_distribution.at(i) << endl;
    counter += primary_buckets.at(i)->getCounter();
    cout << i << " , " << primary_buckets.at(i)->getCounter() << endl;
  }
  cout << counter << endl;

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


  return data_entries;
}
