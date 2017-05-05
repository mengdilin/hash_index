#include "BTreePage.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <math.h>       /* ceil */

using namespace std;

/**
 * @brief constructor for a BTreePage which initializes
 * current page's parent to null
 */
BTreePage::BTreePage() {
  this->parent = nullptr;
}

/**
 * @brief adds page to the end of children's vector
 * @param c a pointer to the child page
 */
void BTreePage::addChild(BTreePage* c) {
  assert(children.size() < fan_out);
  children.push_back(c);
}

BTreePage::~BTreePage() {

}

/**
 * @brief writes current page to binary index file
 * Page format consists of a uint64_t counter maintaining
 * the number of children in the page, followed by MAX_KEY_PER_PAGE
 * uin64_t keys and MAX_KEY_PER_PAGE + 1 rids
 * @param indexFile ofstream of index file
 */
ofstream& BTreePage::flush(ofstream& indexFile) {
  auto before = indexFile.tellp();
  uint64_t counter = keys.size();
  indexFile.write((char*) &counter, sizeof(counter));
  for (auto& key : keys) {
    indexFile.write((char*) &key, sizeof(uint64_t));
  }
  uint64_t pad = 0;
  while (counter < MAX_KEY_PER_PAGE) {
    indexFile.write((char*) &pad, sizeof(uint64_t));
    counter ++;
  }
  for (auto& rid : rids) {
    indexFile.write((char*) &rid, sizeof(uint64_t));
  }

  int size_of_rids = rids.size();
  while (size_of_rids < MAX_KEY_PER_PAGE + 1) {
    //cout << "here" << endl;
    indexFile.write((char*) &pad, sizeof(uint64_t));
    size_of_rids ++;
  }
  auto after = indexFile.tellp();
  if (after - before != BTreePage::PAGE_SIZE) {
    cout << "bytes written: " << after - before << endl;
    cout << "page num: " << pageNum << endl;
    cout << "total rids written: " << size_of_rids << endl;
    cout << "total keys written: " << counter << endl;
  }
  return indexFile;
}

/**
 * @brief reads data into a BTreePage from an indexFile using a file descriptor
 * @param indexFile is a file descriptor to the binary btree index file
 * @param page is an empty BTreePage which will be populated after read completes
 * @param is_leaf indicates whether the current page to be read should be a leaf page
 * @param offset is the offset of indexFile that indicates the start of the current page
 */
void BTreePage::read(int indexFile, BTreePage& page, bool is_leaf, off_t offset) {
  uint64_t counter;
  off_t size_read = pread(indexFile, (void *)&counter, sizeof(counter), offset) + offset;


  uint64_t tmp_keys[MAX_KEY_PER_PAGE];
  size_read += pread(indexFile, (void *)&tmp_keys, sizeof(uint64_t)*(MAX_KEY_PER_PAGE), size_read);

  page.keys.assign(tmp_keys, tmp_keys+counter);

  uint64_t tmp_rids[MAX_KEY_PER_PAGE+2];

  if (not is_leaf) {
    size_read += pread(indexFile,(void *)&tmp_rids,sizeof(uint64_t)*(MAX_KEY_PER_PAGE+1),size_read);
    page.rids.assign(tmp_rids, tmp_rids+counter+1);
  } else {
    size_read += pread(indexFile,(void *)&tmp_rids,sizeof(uint64_t)*(MAX_KEY_PER_PAGE+1),size_read);
    page.rids.assign(tmp_rids, tmp_rids+counter);
  }
}

/**
 * @brief reads data into a BTreePage from an indexFile using a file handler
 * @param indexFile is a file handler to the binary btree index file
 * @param page is an empty BTreePage which will be populated after read completes
 * @param is_leaf indicates whether the current page to be read should be a leaf page
 */
void BTreePage::read(FILE* indexFile, BTreePage& page, bool is_leaf) {
  uint64_t counter;
  fread(&counter, sizeof(counter), 1, indexFile);

  uint64_t tmp_keys[MAX_KEY_PER_PAGE];
  fread(&tmp_keys, sizeof(uint64_t)*(MAX_KEY_PER_PAGE), 1, indexFile);
  page.keys.assign(tmp_keys, tmp_keys+counter);
  uint64_t tmp_rids[MAX_KEY_PER_PAGE+2];

  if (not is_leaf) {
    fread(&tmp_rids, sizeof(uint64_t)*(MAX_KEY_PER_PAGE+1), 1, indexFile);
    page.rids.assign(tmp_rids, tmp_rids+counter+1);
  } else {
    fread(&tmp_rids, sizeof(uint64_t)*(MAX_KEY_PER_PAGE+1), 1, indexFile);
    page.rids.assign(tmp_rids, tmp_rids+counter);
  }
}

/**
 * @brief given a key, finds the reference/rid to the child page
 * @param key search key
 * @return a pair where the first item is a boolean indicating whether
 * key is found in the current page and the second pair is the rid
 * of the child corresponding to the key
 */
pair<bool,uint64_t> BTreePage::find(uint64_t key) {

  // lower_bound implements binary search
  //first element in the range [first,last)
  //which does not compare less than val.
  auto result = lower_bound(
    keys.begin(),
    keys.end(),
    key);
  pair <bool,uint64_t> find_result;
  if (result == keys.end()) {
    // key is greater than largest key in page
    //return the right child of the last key
    find_result = make_pair(true, rids.at(rids.size()-1));
  } else {
    if (key < *result) {
      //if key < cur_key, return left child of cur key
      find_result = make_pair(true, rids.at(result-keys.begin()));
    } else {
      //if key == cur_key, return right child of cur key
      find_result = make_pair(true,  rids.at(result-keys.begin()+1));

    }
  }

  return find_result;
}

/**
 * @brief function used by BTreeIndex to set the parent of the current page
 * @param p pointer of the parent page
 */
void BTreePage::setParent(BTreePage* p) {
  this->parent = p;
}

/**
 * @brief function used by BTreeIndex to add key to the current page
 * @param key key to be added
 */
void BTreePage::addKey(uint64_t key) {
  assert(keys.size() < MAX_KEY_PER_PAGE);
  keys.push_back(key);
}

/**
 * @brief check if current page is full
 */
bool BTreePage::isFull() {
  return keys.size() == MAX_KEY_PER_PAGE;
}



