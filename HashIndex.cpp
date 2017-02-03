#include "HashIndex.h"
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <vector>
#include <math.h>
#include <cmath>
#include <inttypes.h>

using namespace std;
HashIndex::HashIndex(float load_capacity) {
  this->load_capacity = load_capacity;
  this->number_buckets = 0;
}

HashIndex::~HashIndex() {
  for (Page* page : primary_buckets) {
    delete page;
  }
  for (Page* page : overflow_pages) {
    delete page;
  }
}

uint32_t HashIndex::hash(uint64_t key) {
  return hash(key, this->number_buckets);
}

uint32_t HashIndex::hash(uint64_t key, unsigned int num_buckets) {
    bool debugprint = false;
    if (key == 1737642124184) {
      debugprint = true;
    }
    int hashbits = (int)log2(num_buckets) - 1;
    /**
      Knuth multiplicative hashing does not work :(
      hash = (value * NUMBER) >> (64 - HASHBITS)
      key = ((int)(key * HashIndex::KNUTH_NUMBER)) >> (64 - hashbits);
    **/

    /*
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;
    key = key >> (64 - hashbits);
    //cout << "key: " << key;
    */
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;
    key = key >> ((64 - (int)log2(num_buckets)) - 1);
    key = key * UINT32_C(2654435761) % num_buckets;;
    if (debugprint)
      cout << "hash bucket: " << key << endl;

    return key;
}

uint64_t HashIndex::search(uint64_t key, unsigned int num_buckets) {
  uint32_t bucket_num = hash(key, num_buckets);
  cout << "Bucket num: " << bucket_num << endl;
  cout << "size of unsigned int: " << sizeof(unsigned int) << endl;
  uint64_t offset = ((uint64_t)bucket_num * PAGE_SIZE) + sizeof(unsigned int);
  cout << "offset: " << offset << endl;

  cout << "page size: " << PAGE_SIZE << endl;
  cout << "computing: " << bucket_num << "*" << PAGE_SIZE << "+" << sizeof(unsigned int) << endl;

  return offset;
}

uint64_t HashIndex::search(uint64_t key, string indexFilePath) {
  ifstream readIndex (indexFilePath, ifstream::binary);
  unsigned int bucket_num;
  readIndex.read((char *)&bucket_num, sizeof(unsigned int));
  cout << "read bucket num: " << bucket_num <<endl;
  return search(key, bucket_num);
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
    key_distribution.at(i) = 0;

  }

  //initialize primary buckets
  for (int i = 0; i < this->number_buckets; i++) {
    primary_buckets.at(i) = new Page();

  }
  for (DataEntry entry : entries) {
    //cout << "( " << entry.key << " , " << entry.rid << " )" << endl;
    uint32_t hash_key = this->hash(entry.key);
    if (hash_key >= this->number_buckets) {
      cout << "bad hash: " << hash_key << endl;
    } else {
      key_distribution[hash_key]++;
      //cout << hash_key << " ";
      Page* cur_page = primary_buckets[hash_key];
      if (!cur_page->isFull()) {
        cur_page->addEntry(entry);
      } else {
        if (cur_page->hasOverflow()) {
            Page* overflowPage = overflow_pages.at((cur_page->overflow_addr)-1);
            overflowPage->addEntry(entry);
            cout << "hasOverflow at bucket: " << hash_key << " with overflow count: " << overflowPage->counter << " and overflow addr: " << overflowPage->overflow_addr << endl;

        } else {
            Page* overflowPage = new Page();
            overflow_pages.push_back(overflowPage);
            overflowPage->setOverflow(overflow_pages.size());
            overflowPage->addEntry(entry);
            cout << "creating new overflow pages at bucket: " << hash_key << " with overflow count: " << overflowPage->counter << " and overflow addr: " << overflowPage->overflow_addr << endl;
        }

      }

    }

  }

  int counter = 0;
  ofstream indexFile;
  indexFile.open("indexFile", ios::binary | ios::out);
  indexFile.write((char *)&(this->number_buckets), sizeof(unsigned int));
  for (int i = 0; i < this->number_buckets; i++) {
    cout << "bucket: " << i << " with count: " << key_distribution.at(i) << endl;
    Page* page = primary_buckets.at(i);
    counter += page->counter;
    cout << "overflow addr: " << page->overflow_addr << endl;
    if (page->hasOverflow()) {
      cout << "=========has overflow=======" << endl;
      cout << "overflow page with count: " << overflow_pages.at(page->overflow_addr-1)->counter << endl;
    }
    //cout << i << " , " << primary_buckets.at(i)->counter << endl;
    //cout << "page offset: " << indexFile.tellp() << endl;
    primary_buckets.at(i)->flush(indexFile);
  }

/*
  for (Page* page : overflow_pages) {
    cout << "page: " << page->counter <<endl;
  }
*/
  cout << counter << endl;
  indexFile.close();
  cout << "================end=============" << endl;
  //ifstream readIndex ("indexFile", ifstream::binary);

}

void HashIndex::debugRead(string filename) {
  ifstream readIndex(filename, ios::in | ios::binary);

  int bucket_num;
  readIndex.read((char *)&bucket_num, sizeof(unsigned int));
  cout << "num_buckets: " << bucket_num << endl;
  int count = 0;
  while (bucket_num > 0) {
    Page page = Page::read(readIndex);
    count += page.counter;
    bucket_num--;
  }
  cout << "total page pair counter: " << count << endl;

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
      //cout << entry.rid << " |" << endl;
    }
    data_entries.push_back(entry);
  }


  return data_entries;
}
