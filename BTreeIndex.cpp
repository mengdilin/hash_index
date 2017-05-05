#include "BTreeIndex.h"
#include <math.h>
#include <algorithm>
#include <climits>
#include <queue>
#include <cassert>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <math.h>       /* ceil */

/**
 * @file BTreeIndex.cpp
 * @brief implementation of btree index
 */

BTreeIndex::~BTreeIndex() {

}

/**
 * @brief Constructor of BTreeIndex that sets up the initial variables
 * It needs to compute number of total keys per level, given max keys
 * per page.
 */
BTreeIndex::BTreeIndex() {
  cout << "MAX_KEY_PER_PAGE: " << BTreePage::MAX_KEY_PER_PAGE << endl;

  int i = 0;
  while (i < max_level) {
    uint64_t num_nodes = pow(BTreePage::fan_out, i);
    keys_per_level.push_back(num_nodes*BTreePage::MAX_KEY_PER_PAGE);
    fanout_per_level.push_back(num_nodes*BTreePage::fan_out);

    i++;
    cout << "level with keys: " << keys_per_level[keys_per_level.size()-1] << endl;
  }
}

/**
 * @brief helper function when building btree index. Offsets range from 0 to
 * total pages in the btree tree. This function sets each page's offset by doing
 * a level-wise traversal (BFS) of the tree from left to right.
 * root page has an offset of 0. Its leftmost child
 * has an offset of 1. Its rightmost child has an offset of <total pages in the child's level>
 * its leftmost grandchild's offset = its rightmost child's offset + 1 and so on..
 */
void BTreeIndex::setPageOffset() {
  uint64_t pageNum = 0;
  BTreePage* root =(tree.at(0).at(0));
  root->pageNum = pageNum++;
  cout << "root level has: " << tree.at(0).size() << " nodes and first node has: " << root->keys.size() << " keys" <<endl;
  for (int i=1; i < tree.size(); i++) {
    int sum = 0;
    for (int j = 0; j < tree.at(i).size(); j++) {
      BTreePage* curPage =(tree.at(i).at(j));
      curPage->pageNum = pageNum++;
      if (curPage->parent != nullptr) {
        int numChildRefsOfParent = curPage->parent->numChildRefs;
        curPage->parent->rids.push_back(curPage->pageNum);
        curPage->parent->numChildRefs++;
        assert(curPage->parent->numChildRefs == curPage->parent->rids.size());
      } else {
        cout << "parent is null" << endl;
      }
    }
  }
}

/**
 * @brief writes the btree index to a binary file via
 * a level-wise traversal (BFS)
 * Format of the index file:
 * number of pages in the index (8 bytes) followed by
 * root page, leftmost child page of root, ...., rightmost
 * child page of root, leftmost grandchild of root, ...,
 * leftmost leaf page, ..., rightmost leaf page.
 * @param indexFilePath path of the output index file
 */
void BTreeIndex::flush(string indexFilePath) {
  ofstream indexFile;
  indexFile.open(indexFilePath, ios::binary | ios::out);
  uint64_t size = tree.size();
  cout << "tree size: " << size << endl;
  indexFile.write((char *)&size, sizeof(size));
  queue<BTreePage*> myqueue;
  myqueue.push(tree.at(0).at(0));

  while(!myqueue.empty()) {
    BTreePage* page = myqueue.front();
    myqueue.pop();
    page->flush(indexFile);
    for(int i = 0; i < page->children.size(); i++) {
      myqueue.push(page->children.at(i));
    }
  }
}

/**
 * @brief helper function used in probe() to find
 * the index of the corresponding rid of a key in a given btree page.
 * @param begin begin iterator of the key vector
 * @param end end iterator of the key vector
 * @param val key value we are doing the binary find on
 */
template<class Iter, class T>
Iter binary_find(Iter begin, Iter end, T val)
{
    // Finds the lower bound in at most log(last - first) + 1 comparisons
    Iter i = lower_bound(begin, end, val);

    if (i != end && !(val < *i)) {
      return i; // actual value found
    }
    else {
        return i-1; // actual value not found. return smallest value larger than val

    }
}

/**
 * @brief range probe if we are searching for all keys x such that: x >= key
 * @param key: value of key for the inquality x >= key
 * @param indexFile: file descriptor of the binary index file
 * @param dataBinFile: file descriptor of the binary data file
 * @param bin_file_end: size of dataBinFile (i.e. last byte of dataBinFile)
 */
vector<pair<uint64_t, uint64_t>> BTreeIndex::range_probe_gt(uint64_t key, int indexFile, int dataBinFile, off_t bin_file_end) {
    auto result = probe(key,indexFile, dataBinFile);
    off_t rid = (off_t)result.second;
    if (rid >= bin_file_end) {
      vector<pair<uint64_t, uint64_t>> empty;
      return empty;
    }
    return range_probe_bin(dataBinFile, rid, bin_file_end);
  }

/**
 * @brief range probe if we are searching for all keys x such that: key >= x
 * @param key: value of key for the inquality key >= x
 * @param indexFile: file descriptor of the binary index file
 * @param dataBinFile: file descriptor of the binary data file
 */
vector<pair<uint64_t, uint64_t>> BTreeIndex::range_probe_lt(uint64_t key, int indexFile, int dataBinFile) {
    auto result = probe(key,indexFile, dataBinFile);
    off_t rid = (off_t)result.second;
    auto pairs = range_probe_bin(dataBinFile, (off_t)0, rid);
    pairs.push_back(make_pair(key, rid));
    return pairs;
  }

  /**
 * @brief range probe if we are searching for all keys x such that: start_key <= x <= end_key
 * @param start_key: value of key for the inquality start_key <= x <= end_key
 * @param end_key: value of key for the inquality start_key <= x <= end_key
 * @param indexFile: file descriptor of the binary index file
 * @param dataBinFile: file descriptor of the binary data file
 * @param bin_file_end: size of dataBinFile (i.e. last byte of dataBinFile)
 */
 vector<pair<uint64_t, uint64_t>> BTreeIndex::range_probe_endpts(uint64_t start_key, uint64_t end_key, int indexFile, int dataBinFile, off_t bin_file_end) {
    auto start_result = probe(start_key, indexFile, dataBinFile);
    auto end_result = probe(end_key,indexFile, dataBinFile);
    off_t start_rid = (off_t)start_result.second;
    off_t end_rid = (off_t)end_result.second;
    //if start_rid > end_rid or if start_rid is at the end of the bin file
    if (start_rid > end_rid || start_rid >= bin_file_end) {
      //check end_rid?
      vector<pair<uint64_t, uint64_t>> empty;
      return empty;
    }

    if (end_rid > bin_file_end) {
      end_rid = bin_file_end;
    }
    vector<pair<uint64_t, uint64_t>> result = range_probe_bin(dataBinFile, start_rid, end_rid);
    if (end_result.first && start_key != end_key) {
      //if end_key is found in indexFile, include it in the range probe result
      result.push_back({end_key, end_rid});
    }
    return result;
 }

/**
 * @brief helper function for range probe.
 * Given a start offset and an end offset of a binary data file, return
 * all keys within the offsets
 * @param indexFile: file descriptor of the binary data file
 * @param offset: start offset of the binary data file
 * @param end_offset: end offset of the binary data file
 */
vector<pair<uint64_t, uint64_t>> BTreeIndex::range_probe_bin(int indexFile, off_t offset, off_t end_offset) {
  //cout << "begin offset: " << offset << " and end offset: " << end_offset << endl;
  vector<pair<uint64_t, uint64_t>> result;
  uint8_t buffer[4096];
  ssize_t bytes_read = pread(indexFile, (void *)&buffer, 4096, offset);
  uint64_t local_key = *((uint64_t *)&buffer[0]);
   pread(indexFile, (void *)&local_key, sizeof(local_key), offset);
  uint32_t count;
  uint32_t length;
  off_t size_read = sizeof(local_key);
  off_t old_size_read = size_read;
  off_t total_size_read = sizeof(local_key);
  result.push_back(make_pair(local_key, offset));


  ssize_t header_leng = sizeof(uint64_t) + 2*sizeof(uint32_t);
  while (total_size_read+offset < end_offset) {
    old_size_read = size_read;

    count = *((uint32_t *)&buffer[size_read]);

    size_read += sizeof(count);
    total_size_read += sizeof(count);

    length = *((uint32_t *)&buffer[size_read]);

    size_read += sizeof(length);
    size_read += ceil((float)count/8.0) + count + length ;

    total_size_read += sizeof(length);
    total_size_read += ceil((float)count/8.0) + count + length ;

    if (total_size_read+offset >= end_offset) {
      //last 4096 buffer has reached its end
      break;
    }
    if (size_read > bytes_read - header_leng) {
      size_read = 0;
      bytes_read = pread(indexFile, (void *)&buffer, 4096, offset+total_size_read);
    }

    local_key = *((uint64_t *)&buffer[size_read]);

    result.push_back(make_pair(local_key, total_size_read + offset));
    size_read += sizeof(local_key);
    total_size_read += sizeof(local_key);
  }
  return result;
}

/**
 * @brief helper function for btree quality probe on a single key. It searches
 * the binary data file, looking for key and if key is found, returns an
 * offset to the binary data file. If key is not found, return an offset of
 * the smallest key larger than the search key.
 * @param key: the key we are probing for
 * @param indexFile: file descriptor of the binary data file
 * @param offset: start offset of the binary data file
 */
pair<bool, uint64_t> BTreeIndex::probe_bin(uint64_t key, int indexFile, off_t offset) {
  uint8_t buffer[4096];
  ssize_t bytes_read = pread(indexFile, (void *)&buffer, 4096, offset);
  off_t new_offset;
  off_t old_offset = offset;
  uint64_t local_key = *((uint64_t *)&buffer[0]);
  if (local_key == key) {
    return make_pair(true, offset);
  }
  uint32_t count = *((uint32_t *)&buffer[sizeof(local_key)]);
  uint32_t length = *((uint32_t *)&buffer[sizeof(local_key)+sizeof(count)]);
  new_offset = sizeof(local_key) + sizeof(count) + sizeof(length) + ceil((float)count/8.0) + count + length ;
  off_t size_read = new_offset;
  off_t old_size_read = 0;

  new_offset += offset;
  ssize_t header_leng = sizeof(uint64_t) + 2*sizeof(uint32_t);

  if (size_read > bytes_read - header_leng) {
      pread(indexFile, (void *)&local_key, sizeof(local_key), new_offset);

  } else {
      local_key = *((uint64_t *)&buffer[size_read]);
  }

    size_read += sizeof(local_key);
  while (local_key < key and size_read <= bytes_read - header_leng) {


    old_offset = new_offset;
    old_size_read = size_read;
    count = *((uint32_t *)&buffer[size_read]);

    size_read += sizeof(count);
    length = *((uint32_t *)&buffer[size_read]);

    size_read += sizeof(length);
    new_offset = sizeof(local_key) + sizeof(count) + sizeof(length) + ceil((float)count/8.0) + count + length ;
    size_read += ceil((float)count/8.0) + count + length ;
    new_offset += old_offset;


    if (size_read > 4096 - header_leng) {
      pread(indexFile, (void *)&local_key, sizeof(local_key), new_offset);

    } else {
      local_key = *((uint64_t *)&buffer[size_read]);
    }
    size_read += sizeof(local_key);
  }

  if (local_key == key) {
    return make_pair(true, size_read+offset-sizeof(local_key));
  } else {
    //if search key not found, return the smallest key larger than search key.
    //if no key is larger than search key in data.bin, probe will return
    //an offset to the end of the data.bin file
    return make_pair(false, size_read+offset-sizeof(local_key));
  }

}

/**
 * @brief btree quality probe on a single key using file descriptor. First, it traverses the btree
 * index, then it calls probe_bin to probe the binary data file, looking for
 * the offset of the key.
 * @param key: the key we are probing for
 * @param indexFile: file descriptor of the btree index file
 * @param binFile: file descriptor of the binary data file
 */
pair<bool, uint64_t> BTreeIndex::probe(uint64_t key, int indexFile, int binFile) {
  //cout << "key: " << key << endl;
  int level = 1;
  uint64_t tree_size;
  off_t size_read = pread(indexFile, (void *)&tree_size, sizeof(tree_size), 0);

  BTreePage curPage;
  if (tree_size > 1) {
    BTreePage::read(indexFile, curPage, false, size_read);
  } else {
    BTreePage::read(indexFile, curPage, true, size_read);
    int found_index = binary_find(curPage.keys.begin(), curPage.keys.end(), key) - curPage.keys.begin();
    if (found_index == -1) {
      return make_pair(false, 0);
    }
    uint64_t offset = curPage.rids.at(found_index);
    return probe_bin(key, binFile, offset);
  }

  auto result = curPage.find(key);
  while(level < tree_size) {
    if (level == tree_size-1) {
      BTreePage::read(indexFile, curPage, true, (off_t)result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t));
      int found_index = binary_find(curPage.keys.begin(), curPage.keys.end(), key) - curPage.keys.begin();
    if (found_index == - 1) {
      return make_pair(false, 0);
    }
      uint64_t offset = curPage.rids.at(found_index);
      return probe_bin(key, binFile, offset);

    }
    level++;
    BTreePage::read(indexFile, curPage, false, (off_t)result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t));
    result = curPage.find(key);
  }
  return make_pair(false, 0);
}

/**
 * @brief This method is not being used in the current
 * implementation of probe. btree quality probe on a single key using file handler.
 * First, it traverses the btree
 * index, then it calls probe_bin to probe the binary data file, looking for
 * the offset of the key.
 * @param key: the key we are probing for
 * @param indexFile: file handler of the btree index file
 */
pair<bool, uint64_t> BTreeIndex::probe(uint64_t key, FILE* indexFile) {
  int level = 1;
  uint64_t tree_size;
  fread(&tree_size, sizeof(tree_size), 1, indexFile);
  BTreePage curPage;
  if (tree_size > 1) {
    BTreePage::read(indexFile, curPage, false);
  } else {
    BTreePage::read(indexFile, curPage, true);


    int found_index = binary_find(curPage.keys.begin(), curPage.keys.end(), key) - curPage.keys.begin();
    if (found_index == curPage.keys.size()) {
      fseek(indexFile, 0, SEEK_SET);
      return make_pair(false, 0);
    } else {
      fseek(indexFile, 0, SEEK_SET);
      return make_pair(true, curPage.rids.at(found_index));
    }

  }

  auto result = curPage.find(key);
  while(level < tree_size) {
    if (level == tree_size-1) {
      fseek(indexFile, result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t), SEEK_SET);
      BTreePage::read(indexFile, curPage, true);
      int found_index = binary_find(curPage.keys.begin(), curPage.keys.end(), key) - curPage.keys.begin();

      fseek(indexFile, 0, SEEK_SET);
      return make_pair(true, curPage.rids.at(found_index));
    }
    level++;
    fseek(indexFile, result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t), SEEK_SET);
    BTreePage::read(indexFile, curPage, false);
    result = curPage.find(key);
  }
  return make_pair(false, 0);
}

/**
 * @brief Prints each page in the tree
 */
void BTreeIndex::BfsDebugPrint() {
  queue<BTreePage*> myqueue;
  myqueue.push(tree.at(0).at(0));
  while(!myqueue.empty()) {
    BTreePage* page = myqueue.front();
    myqueue.pop();
    cout << "page num: " << page->pageNum << " with keys: " << page->keys.size() << " and children num: " << page->rids.size()<< endl;
    cout << "key: ";
    for (auto& key : page->keys) {
      cout << key << ",";
    }
    cout << endl;
    cout << "rid: ";
    for (auto& key : page->rids) {
      cout << key << ",";
    }
    cout << endl;
    for(int i = 0; i < page->children.size(); i++) {
      //cout << "child num: " << page->children.at(i)->pageNum << " \t";
      assert(page->children.at(i)->pageNum == page->rids.at(i));
      myqueue.push(page->children.at(i));
    }
    cout << endl;
  }
}

/**
 * @brief helper function for building a btree index. It adds a node to a level
 *
 */
void BTreeIndex::addNodeToTree(int level, BTreePage* node) {
  tree.at(max_level-level).push_back(node);
}

/**
 * @brief builds a btree index using the bulkloading algorithm as discussed in
 * the textbook
 * @param entries: all (key,rid) entries
 */
void BTreeIndex::build_tree(vector<DataEntry> entries) {
  uint64_t sum = 0;
  unsigned int num_levels_needed = 0;
  auto low=lower_bound (keys_per_level.begin(), keys_per_level.end(), entries.size());
  while (entries.size() > sum) {
    sum = keys_per_level[num_levels_needed++];
    vector<BTreePage*> cur_level;
    tree.push_back(cur_level);
  }
  max_level = num_levels_needed-1;
  cout << "max level: " << max_level << endl;

  BTreePage* leaf = new BTreePage();
  leaf->level = 0;
  addNodeToTree(leaf->level, leaf);

  for (int i = 0; i < entries.size(); i++) {

    if (!leaf->isFull()) {

      leaf->addKey(entries.at(i).key);
      leaf->rids.push_back(entries.at(i).rid);
    } else {
      //find latest unfill ancestor
      BTreePage *parent = leaf->parent;
      BTreePage* child = leaf;
      while((parent != nullptr) && parent->isFull()) {
        child = parent;
        parent = parent->parent;
      }

      if (parent == nullptr) {
        //create new ancestor and add current entry to ancestor
        //add latest full ancestor as its child
        BTreePage* newParent = new BTreePage();
        newParent->level = child->level+1;
        newParent->addKey(entries.at(i).key);

        addNodeToTree(newParent->level, newParent);

        parent = newParent;

        parent->addChild(child);

        child->setParent(parent);
      } else {
        //add current key to latest unfilled parent
        parent->addKey(entries.at(i).key);
      }
      //create new child
      BTreePage* newChild = new BTreePage();
      newChild->level = child->level;
      addNodeToTree(newChild->level, newChild);
      //add child to parent
      parent->addChild(newChild);
      newChild->setParent(parent);

      while (newChild->level > 0) { //if child is not leaf
        //create descendants until we hit the leaf level
        BTreePage* tmp = new BTreePage();
        tmp->level = newChild->level-1;
        addNodeToTree(tmp->level, tmp);
        newChild->addChild(tmp);
        tmp->setParent(newChild);
        newChild = tmp;
      }

      newChild->addKey(entries.at(i).key); //add current entry to the leaf
      newChild->rids.push_back(entries.at(i).rid);
      leaf = newChild;
    }
  }
  setPageOffset();
}

/**
 * @brief parser that expects a file with 3 tab-delimited columns
 * with the following format: key\tcount\trid where the
 * middle value count is ignored
 */
vector<DataEntry> BTreeIndex::parse_idx_file(string path) {
  int leaf_chunk_size = 4096; //leaf level rids should be 4k chunk apart
  vector<DataEntry> data_entries;
  ifstream input(path);
  char const row_delim = '\n';
  string const field_delim = "\t";
  uint64_t rid = 0;
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
    if (entry.rid - rid >= leaf_chunk_size || entry.rid == 0 ) { //rid == 0 means first rid

      data_entries.push_back(entry);
      rid = entry.rid;
    }

  }
  cout << "data size: " << data_entries.size() << endl;


  return data_entries;
}

/**
 * @brief parser that expects a file with 3 tab-delimited columns
 * with the following format: key\tcount\trid where the
 * middle value count is ignored
 */
vector<DataEntry> BTreeIndex::parse_idx_file_get_all(string path) {
  int leaf_chunk_size = 4096; //leaf level rids should be 4k chunk apart
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
