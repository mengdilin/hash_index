#include "HashIndex.h"
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <vector>
#include <math.h>
#include <cmath>
#include <inttypes.h>
#include <functional>

using namespace std;
/*
 * constructor that takes in load_capacity as a variable
 */
HashIndex::HashIndex(float load_capacity) {
  this->load_capacity = load_capacity;
  this->number_buckets = 0;
}

/*
 * destructor that frees all pages
 */
HashIndex::~HashIndex() {
  for (Page* page : primary_buckets) {
    delete page;
  }
  /*
  for (Page* page : overflow_pages) {
    delete page;
  }
  */
}

uint32_t HashIndex::hash(uint64_t key) {
  return hash(key, this->number_buckets);
}

/*
 * hash function that given a key and number of primary buckets, returns the hash
 * has some debug print in it.
 */
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

/*
 * takes a key and number of total primary buckets and returns the offset
 * of the page in the index file corresponding to the key
 */
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

/**
 * searches through the index file and returns rid of a given key
 */
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
  uint64_t offset;
  Page::read(is, curPage);
  while (key > curPage.data_entry_list[curPage.counter-1].key) {
    if (curPage.hasOverflow()) {
      //reset pointer
      is.seekg(0);
      offset = sizeof(unsigned int) + ((uint64_t)curPage.overflow_addr) * PAGE_SIZE;
      is.seekg(offset);
      Page::read(is, curPage);
    } else {
      break;
    }
  }

  pair<bool,uint64_t> result = curPage.find(key);

  if (result.first) {
    return result;
  }
  cout << "not found" << endl;
  // not found
  return result;

}

/*
 * returns the overflow page of the current page
 */
Page* HashIndex::get_overflow_page(Page* cur_page) {
  return overflow_map[cur_page];
  //return this->overflow_pages.at((cur_page->overflow_addr-this->number_buckets));
}

/*
 * checks if current page has overflow
 * cannot use Page::hasOverflow() because this function
 * is used during the index building stage where hasOverflow()
 * has not been set
 */
bool HashIndex::page_has_overflow(Page* page) {

  unordered_map<Page*, Page*>::iterator it;
  it = overflow_map.find(page);
  if (it == overflow_map.end()) {
    return false;
  } else {
    return true;
  }
}

void HashIndex::merge(vector<Page*> merge_primary_buckets, vector<Page*> overflow_pages) {
  //cout << "overflow size: " << overflow_pages.size() << endl;
  sort(merge_primary_buckets.begin(), merge_primary_buckets.end(),
    [](const Page* a, const Page* b) -> bool {
      return a->counter < b->counter;
    });

  sort(overflow_pages.begin(), overflow_pages.end(),
    [](const Page* a, const Page* b) -> bool {
      return a->counter > b->counter;
    });

  int i = 0;
  int j = 0;
  while (i < overflow_pages.size() and j < merge_primary_buckets.size()) {
    Page* overflow = overflow_pages[i];
    Page* merge_candidate = merge_primary_buckets[j];
    while (Page::MAX_ENTRIES - merge_candidate->counter < overflow->counter) {
      j++;
      if (j >= merge_primary_buckets.size()) {
        break;
      }
      merge_candidate = merge_primary_buckets[j];
    }
    if (j >= merge_primary_buckets.size()) {
      // all overflow pages after this page
      // need capacity greater than the primary bucket
      // with the largest empty space.
      // Stop merging
      break;
    }
    merge_candidate->mergePage(*(overflow));
    auto parent_result = map_for_prev_page.find(overflow);
    if (parent_result != map_for_prev_page.end()) {
      Page* parent = parent_result->second;
      auto overflow_result = overflow_map.find(parent);
      if (overflow_result != overflow_map.end()) {
        overflow_map.erase(overflow_result);
      } else {
        cout << "overflow page is not present in overflow_map!?" << endl;
      }
      map_for_prev_page.erase(parent_result);
    } else {
      cout << "overflow page is not present in map_for_prev_page!?" << endl;
    }
    i++;
    j++;
  }

  for (int k = i; k < overflow_pages.size(); k++) {
    Page* overflow = overflow_pages.at(k);
    auto parent_result = map_for_prev_page.find(overflow);
    if (parent_result != map_for_prev_page.end()) {
      Page* parent = parent_result->second;
      parent->setOverflow((uint64_t)(k-i));
      //cout << "k-i: " << k-i << endl;
      //cout << "overflow: " << parent->overflow_addr << endl;
    } else {
      cout << "1overflow page is not present in map_for_prev_page!?" << endl;
    }
  }

}

/*
 * builds the index
 */
void HashIndex::build_index(string path, string indexFilePath) {
  vector<DataEntry> entries = this->parse_idx_file(path);

  // set number_buckets
  this->number_buckets = ceil(((float)entries.size()/(float)Page::MAX_ENTRIES)/(float)this->load_capacity);


  cout << "num buckets: " << this->number_buckets << endl;
  cout << "entry size: " << entries.size() << endl;

  //initialize the number of primary buckets

  vector<Page*> tmp_primary_buckets(this->number_buckets);
  primary_buckets = move(tmp_primary_buckets);

  vector<int> key_distribution(this->number_buckets);
  vector<vector<DataEntry> > sorted_primary_buckets(this->number_buckets);


  for (int i = 0; i < this->number_buckets; i++) {
    key_distribution.at(i) = 0;

  }

  //initialize primary buckets
  for (int i = 0; i < this->number_buckets; i++) {
    //primary_buckets.at(i) = new Page();
    vector<DataEntry> bucket;
    sorted_primary_buckets.at(i) = bucket;
  }
  for (DataEntry entry : entries) {
    //cout << "( " << entry.key << " , " << entry.rid << " )" << endl;
    uint32_t hash_key = this->hash(entry.key);
    if (hash_key >= this->number_buckets) {
      cout << "bad hash: " << hash_key << endl;
    } else {
      key_distribution[hash_key]++;
      sorted_primary_buckets.at(hash_key).push_back(entry);
      //add_entry_to_bucket(hash_key, entry);

    }
  }
  for (auto& bucket : sorted_primary_buckets) {
      sort(bucket.begin(), bucket.end(), DataEntry::compare);
  }
  for (int i = 0; i < sorted_primary_buckets.size(); i++) {
    vector<DataEntry> bucket = sorted_primary_buckets.at(i);
    if (bucket.size() <= Page::MAX_ENTRIES) {
      primary_buckets.at(i) = new Page(bucket);
      continue;
    } else {
      vector<DataEntry> entries(bucket.begin(), bucket.begin()+Page::MAX_ENTRIES);
      primary_buckets.at(i) = new Page(entries);
    }
    // has overflow
    Page* parent = primary_buckets.at(i);
    size_t copied_bucket_size = Page::MAX_ENTRIES;

    int overflow_pg = 0;
    while (copied_bucket_size < bucket.size()) {
      size_t leng_to_copy = Page::MAX_ENTRIES;
      //cout << "copied_bucket_size: " << copied_bucket_size << " bucket size: " << bucket.size() << endl;
      if (bucket.size() - copied_bucket_size < Page::MAX_ENTRIES) {
        leng_to_copy = (bucket.size() - copied_bucket_size);
      }

      vector<DataEntry> entries(bucket.begin() + copied_bucket_size, bucket.begin()+copied_bucket_size+leng_to_copy);
      Page* overflowPage = new Page(entries);
      overflow_map[parent] = overflowPage;
      map_for_prev_page[overflowPage] = parent;
      overflow_pages.push_back(overflowPage);
      parent = overflowPage;
      copied_bucket_size += leng_to_copy;
      overflow_pg ++;
    }
  }




  merge(primary_buckets, overflow_pages);
  cout << "done" <<endl;


  int counter = 0;
  ofstream indexFile;
  indexFile.open(indexFilePath, ios::binary | ios::out);
  indexFile.write((char *)&(this->number_buckets), sizeof(unsigned int));

  int of_count = 0;
  for (int i = 0; i < this->number_buckets; i++) {
    Page* page = primary_buckets.at(i);
    //cout << "bucket: " << i << " with count: " << key_distribution.at(i) << endl;

    counter += page->counter;
    cout << "primary bucket count: " << page->counter;
    //cout << "overflow: " << page->overflow_addr << endl;
    auto result = overflow_map.find(page);
    if (result != overflow_map.end()) {
      of_count++;
    }

    while (page->hasOverflow()) {
      cout << " -> " << get_overflow_page(page)->counter;
      page = get_overflow_page(page);
      counter += page->counter;

    }
    cout << endl;

    primary_buckets.at(i)->flush(indexFile);
  }

  cout << "overflow count: " <<of_count <<endl;
  for (int i = 0; i < overflow_pages.size(); i++) {
    overflow_pages.at(i)->flush(indexFile);
  }
  cout << counter << endl;
  indexFile.close();
  cout << "================end=============" << endl;

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

/*
 * parser that expects a file with 3 tab-delimited columns
 * with the following format: key\tcount\trid where the
 * middle value count is ignored
 */
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
