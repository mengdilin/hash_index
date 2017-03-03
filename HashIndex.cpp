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
#include <algorithm>
#include <chrono>
#include <numeric>

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

  for (Page* page : overflow_pages) {
    delete page;
  }

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
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;
    //key = key >> ((64 - (int)log2(num_buckets)) - 1);
    //key = key * UINT32_C(2654435761) % num_buckets;;
    key = key % num_buckets;

    if (debugprint)
      cout << "hash bucket: " << key << endl;

    return key;
}

/*
 * takes a key and number of total primary buckets and returns the offset
 * of the page in the index file corresponding to the key
 */
uint64_t HashIndex::search(uint64_t key, unsigned int num_buckets) {
  //get the primary bucket of the key
  uint32_t bucket_num = hash(key, num_buckets);

  //compute the offset: index header size + bucket * page size
  uint64_t offset = ((uint64_t)bucket_num * PAGE_SIZE) + sizeof(unsigned int);

  return offset;
}

/**
 * searches through the index file and returns rid of a given key
 * structure of index file: all records with the same hash
 * are sorted in ascending order. Therefore, before performing
 * binary search on a page, check that the greatest key in the page
 * is >= the key we are looking for. Else, we go to the overflow page
 */
pair<bool,uint64_t> HashIndex::search(uint64_t key, ifstream& is) {
  unsigned int bucket_num;
  is.read((char *)&bucket_num, sizeof(unsigned int));
    //test performance on a pre-allocated vector
  auto t1 = chrono::high_resolution_clock::now();
  uint64_t primary_bucket_offset = search(key, bucket_num);
  auto t2 = chrono::high_resolution_clock::now();
  auto sum = (t2-t1).count();
  cout << "compute offset (ns): " << (t2-t1).count() << endl;
  t1 = chrono::high_resolution_clock::now();
  is.seekg(0);
  is.seekg(primary_bucket_offset);
  t2 = chrono::high_resolution_clock::now();
  cout << "seek to offset (ns): " << (t2-t1).count() << endl;

  t1 = chrono::high_resolution_clock::now();
  Page curPage;
  t2 = chrono::high_resolution_clock::now();
  sum += (t2-t1).count();
  cout << "page init (ns): " << (t2-t1).count() << endl;
  uint64_t offset;

  t1 = chrono::high_resolution_clock::now();
  Page::read(is, curPage);
  t2 = chrono::high_resolution_clock::now();
  sum += (t2-t1).count();
  cout << "page read (ns): " << (t2-t1).count() << endl;

  t1 = chrono::high_resolution_clock::now();
  //check if key > largest/last key on current page
  while (key > curPage.data_entry_list[curPage.counter-1].key) {
    //go to overflow
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
  t2 = chrono::high_resolution_clock::now();
  cout << "searching loop (ns): " << (t2-t1).count() << endl;
  sum += (t2-t1).count();
  t1 = chrono::high_resolution_clock::now();
  //binary search to find the result
  pair<bool,uint64_t> result = curPage.find(key);
  t2 = chrono::high_resolution_clock::now();
  sum += (t2-t1).count();
  cout << "binary search (ns): " << (t2-t1).count() << endl;
  cout << "total sum: " << sum << endl;
  is.seekg(0);
  if (result.first) {
    return result;
  }
  cout << "not found" << endl;
  // not found

  return result;

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

/*
 * merges overflow pages with certain primary buckets, if primary buckets
 * have enough room.
 * overflow pages are sorted in increasing order;
 * primary buckets sorted in decreasing order;
 */
int HashIndex::merge(vector<Page*>& merge_primary_buckets, vector<Page*>& overflow_pages) {

  int i = 0;
  int j = 0;
  int fulled_overflows = 0;
  while (i < overflow_pages.size() && overflow_pages.at(i)->counter == Page::MAX_ENTRIES) {
    //don't bother merging overflow pages that are full
    //this avoids an added complexity of logic where
    //the merge has to keep track of the overflow of the
    //overflow and sets the pointers for overflow of the
    //overflow correctly. For now, this sounds like a
    //reasonable thing to do because no primary buckets
    //should have 0 entry. Otherwise, choose a better
    //hash function
    i++;
  }
  fulled_overflows = i;
  while (i < overflow_pages.size() and j < merge_primary_buckets.size()) {
    Page* overflow = overflow_pages[i];
    Page* merge_candidate = merge_primary_buckets[j];
    while (Page::MAX_ENTRIES - merge_candidate->counter < overflow->counter) {
      /*
      * current overflow page has max load among the rest of the overflow pages
      * current merge candidate has max capacity among the rest of the merge candidates
      * if current merge candidate can't fit current overflow page, move on to
      * the next overflow page
      */
      i++;
      if (i >= overflow_pages.size()) {
        break;
      }
      overflow = overflow_pages.at(i);
      //merge_candidate = merge_primary_buckets[j];
    }

    if (i >= overflow_pages.size()) {
      // cannot find anymore overflow page to merge
      // Stop merging
      break;
    }

    merge_candidate->mergePage(*(overflow));

    auto parent_result = map_for_prev_page.find(overflow);
    if (parent_result != map_for_prev_page.end()) {
      Page* parent = parent_result->second;
      parent->setOverflow(merge_candidate->hash);

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

  int overflow_count = 0;
  for (int i = 0; i < overflow_pages.size(); i++) {
      Page* cur_page = overflow_pages.at(i);
      auto iterator = map_for_prev_page.find(cur_page);
      if (iterator == map_for_prev_page.end()) {
        continue;
      } else {
        Page* parent = iterator->second;
        parent->setOverflow((uint64_t)(this->number_buckets+overflow_count));
        overflow_count++;
      }
  }

  /*
   * set the overflow address for the overflow pages that are not merged


  //set the overflow address of full overflows
  for (int k = 0; k < fulled_overflows; k++) {
    Page* overflow = overflow_pages.at(k);
    auto parent_result = map_for_prev_page.find(overflow);
    if (parent_result != map_for_prev_page.end()) {
      Page* parent = parent_result->second;
      //overflow addr ranges from [number_buckets, fulled_overflows)
      parent->setOverflow((uint64_t)(this->number_buckets+k));
    } else {
      cout << "1overflow page is not present in map_for_prev_page!?" << endl;
    }
  }

  //set the overflow address of not full overflow pages
  //i is the beginning of the first not merged and not full overflow page
  for (int k = i; k < overflow_pages.size(); k++) {
    Page* overflow = overflow_pages.at(k);
    auto parent_result = map_for_prev_page.find(overflow);
    if (parent_result != map_for_prev_page.end()) {
      Page* parent = parent_result->second;
      //overflow addr ranges from [number_buckets+full_overflows, number_buckets+full_overflows+(total overflow) - i]
      parent->setOverflow((uint64_t)(this->number_buckets+k-i+fulled_overflows));
    } else {
      cout << "1overflow page is not present in map_for_prev_page!?" << endl;
    }
  }
  */
  return i;
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

  //initialize sorted primary bucket where all records with the same key are
  //stored in the same index
  for (int i = 0; i < this->number_buckets; i++) {
    vector<DataEntry> bucket;
    sorted_primary_buckets.at(i) = bucket;
  }
  for (DataEntry entry : entries) {
    uint32_t hash_key = this->hash(entry.key);
    if (hash_key >= this->number_buckets) {
      cout << "bad hash: " << hash_key << endl;
    } else {
      key_distribution[hash_key]++;
      sorted_primary_buckets.at(hash_key).push_back(entry);
    }
  }

  //sort all records with the same hash key
  for (auto& bucket : sorted_primary_buckets) {
      sort(bucket.begin(), bucket.end(), DataEntry::compare);
  }

  //partition records in sorted_primary_buckets into primary and overflow page
  for (int i = 0; i < sorted_primary_buckets.size(); i++) {
    vector<DataEntry> bucket = sorted_primary_buckets.at(i);
    if (bucket.size() <= Page::MAX_ENTRIES) {
      //no overflow page at current bucket
      Page *page = new Page(bucket);
      page->hash = i;
      primary_buckets.at(i) = page;

      continue;
    } else {
      //cout << "overflow" <<endl;
      //has overflow pages
      vector<DataEntry> entries(bucket.begin(), bucket.begin()+Page::MAX_ENTRIES);
      Page *page = new Page(entries);
      page->hash = i;
      primary_buckets.at(i) = page;

    }
    //populate overflow
    Page* parent = primary_buckets.at(i);
    size_t copied_bucket_size = Page::MAX_ENTRIES;

    int overflow_pg = 0;
    while (copied_bucket_size < bucket.size()) {
      size_t leng_to_copy = Page::MAX_ENTRIES;
      if (bucket.size() - copied_bucket_size < Page::MAX_ENTRIES) {
        leng_to_copy = (bucket.size() - copied_bucket_size);
      }
      //copy the corresponding records for the overflow page
      vector<DataEntry> entries(bucket.begin() + copied_bucket_size, bucket.begin()+copied_bucket_size+leng_to_copy);
      Page *overflowPage = new Page(entries);

      //set up maps to keep track of parent to overflow page relationship
      overflow_map[parent] = overflowPage;
      map_for_prev_page[overflowPage] = parent;
      overflow_pages.push_back(overflowPage);
      parent = overflowPage;
      copied_bucket_size += leng_to_copy;
      overflow_pg ++;
    }
  }


  /*
   * prepare for merge: sort primary buckets and overflow pages
   * primary buckets are sorted in descending order
   * overflow pages are sorted in ascending order
   */
  vector<Page*> merge_primary_buckets = primary_buckets;
  sort(merge_primary_buckets.begin(), merge_primary_buckets.end(),
    [](const Page* a, const Page* b) -> bool {
      return a->counter < b->counter;
    });

  sort(overflow_pages.begin(), overflow_pages.end(),
    [](const Page* a, const Page* b) -> bool {
      return a->counter > b->counter;
    });

  cout << "-------------statistic before merge-----------------" << endl;
  cout << "number of primary buckets: " << merge_primary_buckets.size() << endl;
  cout << "number of overflow pages in map: " << overflow_map.size() << endl;
  cout << "number of overflow pages: " << overflow_pages.size() << endl;
  cout << "---------------end statistic-------------------" << endl;
  //merge step
  int overflow_merge_start = merge(merge_primary_buckets, overflow_pages);
  cout << "after number of overflow pages in map: " << overflow_map.size() << endl;

  cout << "done" <<endl;

  cout << "---------------begin key distribution---------------" << endl;
  for (int i = 0; i < key_distribution.size(); i++) {
    cout << "bucket: " << key_distribution.at(i) << endl;
  }
  cout << "---------------end key distribution---------------" << endl;
  int overflow_count = 0;


  uint32_t counter = 0;
  ofstream indexFile;
  indexFile.open(indexFilePath, ios::binary | ios::out);
  indexFile.write((char *)&(this->number_buckets), sizeof(unsigned int));

  //debug print
  for (int i = 0; i < this->number_buckets; i++) {
    Page* page = primary_buckets.at(i);
    //cout << "bucket: " << i << " with count: " << key_distribution.at(i) << endl;

    counter += page->counter;
    cout << "primary bucket count: " << (uint32_t)page->counter;
    //auto result = overflow_map.find(page);


    while (page->hasOverflow()) {
      auto iterator = overflow_map.find(page);
      if (iterator == overflow_map.end()) {
        //cout << page->overflow_addr << endl;
        //cout << primary_buckets.size() << endl;
        //overflow has been merged with primary buckets
        page = primary_buckets.at((uint64_t)page->overflow_addr);
        cout << " merged -> " << (uint32_t)page->counter;
        break;
      } else {
        page = iterator->second;
        cout << " -> " << (uint32_t)page->counter;
      }


      //counter += page->counter;

    }

    cout << endl;

    primary_buckets.at(i)->flush(indexFile);
  }

  cout << "---------overflow-----------"<<endl;
  cout << "overflow after merge: " << overflow_map.size() << endl;


  /*
  while (i < overflow_pages.size() && overflow_pages.at(i)->counter == Page::MAX_ENTRIES) {
   // counter += overflow_pages.at(i)->counter;
    overflow_pages.at(i)->flush(indexFile);
    counter += overflow_pages.at(i)->counter;
    i++;
  }

  cout << "full overflow: " << i << endl;
  for (int i = overflow_merge_start; i < overflow_pages.size(); i++) {
    overflow_pages.at(i)->flush(indexFile);
    counter += overflow_pages.at(i)->counter;
  }
  cout << counter << endl;
  */
    for (int i = 0; i < overflow_pages.size(); i++) {
      Page* cur_page = overflow_pages.at(i);
      auto iterator = map_for_prev_page.find(cur_page);
      if (iterator == map_for_prev_page.end()) {
        continue;
      } else {
        counter += cur_page->counter;
        cur_page->flush(indexFile);
      }
  }
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
