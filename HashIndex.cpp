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
    if (key == 1708146715154) {
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

pair<bool,uint64_t> HashIndex::search(uint64_t key, string indexFilePath) {
  ifstream readIndex (indexFilePath, ifstream::binary);
  unsigned int bucket_num;
  readIndex.read((char *)&bucket_num, sizeof(unsigned int));
  cout << "read bucket num: " << bucket_num <<endl;
  uint64_t primary_bucket_offset = search(key, bucket_num);
  ifstream is(indexFilePath, ifstream::binary);
  is.seekg(primary_bucket_offset);
  //Page curPage = Page::read(is);
  Page curPage;
  Page::read(is, curPage);
  pair<bool,uint64_t> result = curPage.find(key);

  if (result.first) {
    return result;
  }
  uint64_t offset;
  while (curPage.hasOverflow()) {
    //reset pointer
    is.seekg(0);
    //cout << "page overflow: " << curPage.overflow_addr << endl;
    offset = sizeof(unsigned int) + ((uint64_t)curPage.overflow_addr) * PAGE_SIZE;
    //cout << "overflow page offset: " << offset << endl;
    is.seekg(offset);
    //curPage = Page::read(is);
    Page curPage;
    Page::read(is, curPage);
    result = curPage.find(key);
    if (result.first) {
      return result;
    }
  }
  cout << "not found" << endl;
  // not found
  return result;

}

Page* HashIndex::get_overflow_page(Page* cur_page) {
  return overflow_map[cur_page];
  //return this->overflow_pages.at((cur_page->overflow_addr-this->number_buckets));
}

void HashIndex::add_entry_to_bucket(uint32_t hash_key, DataEntry entry) {
  Page* cur_page = primary_buckets[hash_key];


  if (!cur_page->isFull()) {
    if (entry.key == 1708146715154) {
      cout << "added key to primary bucket with key: " << hash_key << endl;
    }
    cur_page->addEntry(entry);
  } else {
    if (entry.key == 1708146715154) {
        cout << "added key to overflow bucket with key: " << hash_key << endl;

    }
    while(page_has_overflow(cur_page) && get_overflow_page(cur_page)->isFull()) {
      // while current page at bucket is full, go to its overflow page
      cur_page = get_overflow_page(cur_page);
    }

    if (page_has_overflow(cur_page)) {
        // go to the last page in the current bucket chain
        Page* overflowPage = get_overflow_page(cur_page);
        overflowPage->addEntry(entry);
        //cout << "hasOverflow at bucket: " << hash_key << " with overflow count: " << overflowPage->counter << " and overflow addr: " << cur_page->overflow_addr << endl;

    } else {
        // all pages in the bucket are full. Create a new page and add it to the bucket chain.
        Page* overflowPage = new Page();
        overflow_map[cur_page] = overflowPage;
        overflow_pages.push_back(overflowPage);
        map_for_prev_page[overflowPage] = cur_page;

        //cur_page->setOverflow(((uint64_t)this->number_buckets) + overflow_pages.size()-1);
        overflowPage->addEntry(entry);

        //cout << "creating new overflow pages at bucket: " << hash_key << " with overflow count: " << overflowPage->counter << " and overflow addr: " << cur_page->overflow_addr << endl;
    }

  }
}

bool HashIndex::page_has_overflow(Page* page) {

  unordered_map<Page*, Page*>::iterator it;
  it = overflow_map.find(page);
  if (it == overflow_map.end()) {
    return false;
  } else {
    return true;
  }
}
void HashIndex::merge_overflows() {

  auto i = begin(overflow_pages);
  while (i != end(overflow_pages)) {
    Page* page = (*(i));
    int capacity_needed = page->counter;
    cout << "capacity: " << capacity_needed << endl;
    bool did_merge = merge(capacity_needed, page);
    if (did_merge) {
      i = overflow_pages.erase(i);
    } else {
      ++i;
    }
  }
}

bool HashIndex::merge(int capacity_needed, Page* page) {
  auto i = begin(primary_buckets);

  while (i != end(primary_buckets)) {
    Page* merge_candidate = *(i);
    if (Page::MAX_ENTRIES - merge_candidate->counter >= capacity_needed) {
      cout << "capacity needed: " << capacity_needed << endl;
      cout << "before merge count: " << merge_candidate->counter <<endl;
      merge_candidate->mergePage(*(page));
      cout << "after merge count: " << merge_candidate->counter <<endl;

      int cur_index = i - primary_buckets.begin();

      unordered_map<Page*, Page*>::iterator it;
      it = map_for_prev_page.find(page);
      if (it == map_for_prev_page.end()) {
        cout << "merge: could not find certain overflow page in map :(" << endl;
      }
      //overflow page's parent now points to merge_candidate;
      Page* parent = it->second;
      parent->setOverflow(cur_index);
      parent->overflow_merged = true;
      cout << "set new overflow to: " << cur_index << endl;

      return true;
    }
    ++i;
  }
  return false;;
}

void HashIndex::set_pages_overflow_addr() {
  unordered_map<Page*, Page*>::iterator it;
  auto i = begin(overflow_pages);

  while (i != end(overflow_pages)) {
    Page* page = *(i);
    it = map_for_prev_page.find(page);
    if (it == map_for_prev_page.end()) {
      cout << "could not find certain overflow page in map :(" << endl;
      continue;
    }
    int cur_index = i - overflow_pages.begin();
     //cur_page->setOverflow(((uint64_t)this->number_buckets) + overflow_pages.size()-1);
    Page* parent = it->second;
    parent->setOverflow(((uint64_t)this->number_buckets)+cur_index);
    //cout << "set new overflow to: " << ((uint64_t)this->number_buckets)+cur_index << endl;
    ++i;
  }

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
      add_entry_to_bucket(hash_key, entry);

    }

  }

  merge_overflows();
  set_pages_overflow_addr();

  int counter = 0;
  ofstream indexFile;
  indexFile.open("indexFile", ios::binary | ios::out);
  indexFile.write((char *)&(this->number_buckets), sizeof(unsigned int));

  for (int i = 0; i < this->number_buckets; i++) {
    Page* page = primary_buckets.at(i);
    //cout << "bucket: " << i << " with count: " << key_distribution.at(i) << endl;

    counter += page->counter;
    cout << "primary bucket count: " << page->counter;


    while (page->hasOverflow() && !page->overflow_merged) {
      cout << " -> " << get_overflow_page(page)->counter;
      page = get_overflow_page(page);
      counter += page->counter;

    }
    cout << endl;

    primary_buckets.at(i)->flush(indexFile);
  }
  for (int i = 0; i < overflow_pages.size(); i++) {
    overflow_pages.at(i)->flush(indexFile);
  }

  for (Page* page : overflow_pages) {
    cout << "page: " << page->counter <<endl;
  }

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
    //Page page = Page::read(readIndex);
    Page page;
    Page::read(readIndex, page);
    count += page.counter;
    bucket_num--;
  }
  cout << "=======overflow===========" << endl;
  while (!readIndex.eof()) {
    //Page page = Page::read(readIndex);
    Page page;
    Page::read(readIndex, page);
    count += page.counter;
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
