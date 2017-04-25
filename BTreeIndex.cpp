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
BTreeIndex::~BTreeIndex() {

}

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
        //curPage->parent->rids.at(numChildRefsOfParent) = curPage->pageNum;
        curPage->parent->rids.push_back(curPage->pageNum);
        curPage->parent->numChildRefs++;
        assert(curPage->parent->numChildRefs == curPage->parent->rids.size());
      } else {
        cout << "parent is null" << endl;
      }
    }
    //cout << endl;
    //cout << "level: " << i <<" with keys: " << sum <<" and with num nodes: " << tree.at(i).size() << endl;

    //cout << "max keys: " << pow(BTreePage::fan_out, i)*BTreePage::MAX_KEY_PER_PAGE << " max nodes: " <<  pow(BTreePage::fan_out, i)<< endl;
  }
}

void BTreeIndex::debugPrint() {
  debugPrint(tree.at(0).at(0));
}


vector<BTreePage*> BTreeIndex::get_simulated_stream() {
  vector<BTreePage*> stream;
  for (int i = 0; i < tree.size(); i++) {
    for (int j = 0; j < tree.at(i).size(); j++) {
      stream.push_back(tree.at(i).at(j));
    }
  }
  return stream;
}



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
    //cout << "page num: " << page->pageNum << " with keys: " << page->keys.size() << endl;
    for(int i = 0; i < page->children.size(); i++) {
      //cout << "child num: " << page->children.at(i)->pageNum << " \t";
      assert(page->children.at(i)->pageNum == page->rids.at(i));
      myqueue.push(page->children.at(i));
    }
    //cout << endl;
  }
}


template<class Iter, class T>
Iter binary_find(Iter begin, Iter end, T val)
{
    // Finds the lower bound in at most log(last - first) + 1 comparisons
    Iter i = lower_bound(begin, end, val);

    if (i != end && !(val < *i)) {
      //cout << "binary_find found value" << endl;
      return i; // actual value found
    }
    else {
            //cout << "binary_find not found value" << endl;
        return i-1; // actual value not found. return smallest value larger than val

    }
}

vector<pair<uint64_t, uint64_t>> BTreeIndex::range_probe_gt(uint64_t key, int indexFile, int dataBinFile, off_t bin_file_end) {
    auto result = probe(key,indexFile, dataBinFile);
    off_t rid = (off_t)result.second;
    return range_probe_bin(dataBinFile, rid, bin_file_end);
  }

vector<pair<uint64_t, uint64_t>> BTreeIndex::range_probe_lt(uint64_t key, int indexFile, int dataBinFile) {
    auto result = probe(key,indexFile, dataBinFile);
    off_t rid = (off_t)result.second;
    auto pairs = range_probe_bin(dataBinFile, (off_t)0, rid);
    pairs.push_back(make_pair(key, rid));
    return pairs;
  }

vector<pair<uint64_t, uint64_t>> BTreeIndex::range_probe_bin(int indexFile, off_t offset, off_t end_offset) {
  //cout << "begin offset: " << offset << " and end offset: " << end_offset << endl;
  vector<pair<uint64_t, uint64_t>> result;
  uint8_t buffer[4096];
  pread(indexFile, (void *)&buffer, 4096, offset);
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

    /*
    cout << "local key: " << local_key << endl;
    cout << "size+offset: " << (size_read+offset) << endl;
    cout << "size: " << size_read << endl;
    cout << "total size+offset: " << (total_size_read+offset) << endl;
    cout << "end_offset: " << end_offset << endl;
    cout << "inserted offset: " << (total_size_read + 4096 - size_read+offset) << endl;
    */
    if (total_size_read+offset >= end_offset) {
      //last 4096 buffer has reached its end
      break;
    }
    if (size_read > 4096 - header_leng) {
      size_read = 0;
      pread(indexFile, (void *)&buffer, 4096, offset+total_size_read);
    }

    local_key = *((uint64_t *)&buffer[size_read]);

    result.push_back(make_pair(local_key, total_size_read + offset));
    size_read += sizeof(local_key);
    total_size_read += sizeof(local_key);
  }
  return result;
}

pair<bool, uint64_t> BTreeIndex::probe_bin(uint64_t key, int indexFile, off_t offset) {
  uint8_t buffer[4096];
  pread(indexFile, (void *)&buffer, 4096, offset);
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

  if (size_read > 4096 - header_leng) {
      pread(indexFile, (void *)&local_key, sizeof(local_key), new_offset);

  } else {
      local_key = *((uint64_t *)&buffer[size_read]);
  }

    size_read += sizeof(local_key);
  while (local_key < key and size_read <= 4096 - header_leng) {


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
  cout << "local key: " << local_key << endl;
  cout << "found: " << (size_read+offset-sizeof(local_key)) << endl;

  if (local_key == key) {
    return make_pair(true, size_read+offset-sizeof(local_key));
  } else {
    //return the smallest key larger than search key, if search key not found
    return make_pair(false, size_read+offset-sizeof(local_key));
  }

}

vector<pair<uint64_t, uint64_t>> BTreeIndex::range_probe(uint64_t key, int indexFile, int binFile, off_t bin_file_end) {
  int level = 1;
  uint64_t tree_size;
  off_t size_read = pread(indexFile, (void *)&tree_size, sizeof(tree_size), 0);
  BTreePage curPage;
  if (tree_size > 1) {
    BTreePage::read(indexFile, curPage, false, size_read);
  } else {
    BTreePage::read(indexFile, curPage, true, size_read);
    int found_index = binary_find(curPage.keys.begin(), curPage.keys.end(), key) - curPage.keys.begin();
    uint64_t offset = curPage.rids.at(found_index);
    return range_probe_bin(binFile, offset, bin_file_end);
  }

  auto result = curPage.find(key);
  while(level < tree_size) {
    if (level == tree_size-1) {
      BTreePage::read(indexFile, curPage, true, (off_t)result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t));
      int found_index = binary_find(curPage.keys.begin(), curPage.keys.end(), key) - curPage.keys.begin();
      uint64_t offset = curPage.rids.at(found_index);
      return range_probe_bin(binFile, offset, bin_file_end);
    }
    level++;
    BTreePage::read(indexFile, curPage, false, (off_t)result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t));
    result = curPage.find(key);
  }
  vector<pair<uint64_t, uint64_t>> final_result;
  return final_result;
}


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
    if (found_index == - 1) {
      for (auto key : curPage.keys) {
        cout << key << "\t";
      }
      cout << endl;
    }
    //cout << "found_index: " << found_index << endl;
    uint64_t offset = curPage.rids.at(found_index);
    if (key == 844468738445) {
      cout << "offset for key: " << key << " is " << offset << endl;
   }
    return probe_bin(key, binFile, offset);
  }

  auto result = curPage.find(key);
  while(level < tree_size) {
    if (level == tree_size-1) {
      BTreePage::read(indexFile, curPage, true, (off_t)result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t));
      int found_index = binary_find(curPage.keys.begin(), curPage.keys.end(), key) - curPage.keys.begin();
    if (found_index == - 1) {
      for (auto key : curPage.keys) {
        cout << key << "\t";
      }
      cout << endl;
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


pair<bool, uint64_t> BTreeIndex::probe(uint64_t key, FILE* indexFile) {
  //cout << "key: " << key << endl;
  int level = 1;
  uint64_t tree_size;
  fread(&tree_size, sizeof(tree_size), 1, indexFile);
  /*
  cout << "tree size: " << tree_size << endl;
  for (auto &level : tree) {
    cout <<"level nodes: " << level.size() << endl;
  }
  */
  //cout << tree.size() << endl;

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
    //cout << "child numer: " << result.second << endl;
    //cout << "key: " << key << endl;


    if (level == tree_size-1) {
      /*
      cout << "reached max level" << endl;
      cout << "key: " << key << endl;
      cout << "page num: " << result.second << endl;
      */
      fseek(indexFile, result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t), SEEK_SET);
      BTreePage::read(indexFile, curPage, true);
      //curPage = stream.at(result.second);
      //cout << "binary find" << endl;
      int found_index = binary_find(curPage.keys.begin(), curPage.keys.end(), key) - curPage.keys.begin();

      fseek(indexFile, 0, SEEK_SET);
      return make_pair(true, curPage.rids.at(found_index));

      //cout << "found index: " << found_index <<endl;

      /*
      for (int i =0; i < curPage.keys.size(); i++) {
        cout << curPage.keys.at(i) << "\t";
        if (curPage.keys.at(i) == key) {
          cout << "new found index: " << i << endl;
          found_index = i;
        }
      }

      cout << endl;
      for (int i =0; i < curPage.rids.size(); i++) {
        cout << curPage.rids.at(i) << "\t";
      }


      cout << endl;
      cout << "found value: " << curPage.rids.at(found_index) << endl;
      break;
      */

    }
    level++;
    fseek(indexFile, result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t), SEEK_SET);
    BTreePage::read(indexFile, curPage, false);
    //cout << "total pages: " << stream.size() << endl;

    //curPage = stream.at(result.second);
    result = curPage.find(key);
    //cout << "level: " << level << endl;
    //cout << "tree level: " << tree.size() << endl;
  }
  return make_pair(false, 0);
}

/*
void BTreeIndex::probe(uint64_t key, vector<BTreePage*> stream) {
  cout << "key: " << key << endl;
  int level = 1;
  BTreePage* curPage = stream.at(0);
  auto result = curPage->find(key);
  while(result.first) {
    if (level == tree.size()-1) {
      cout << "reached max level" << endl;
      curPage = stream.at(result.second);
      for (int i =0; i < curPage->keys.size(); i++) {
        cout << curPage->keys.at(i) << "\t";
      }
      cout << endl;
      break;
    }
    level++;
    //cout << "total pages: " << stream.size() << endl;
    curPage = stream.at(result.second);
    result = curPage->find(key);
    //cout << "level: " << level << endl;
    //cout << "tree level: " << tree.size() << endl;
  }
}
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



void BTreeIndex::test_page_read(FILE* indexFile) {
  uint64_t tree_size;
  fread(&tree_size, sizeof(tree_size), 1, indexFile);
  BTreePage page;
  BTreePage::read(indexFile, page, false);
  cout << "size of keys: " << page.keys.size() << endl;
  for (auto& key : page.keys) {
    cout << key << endl;
  }
}

void BTreeIndex::debugPrint(BTreePage* page) {
  if (page == nullptr) {
    cout << " encountered null " << endl;
    return;
  }
  cout << "page num: " << page->pageNum << " with keys: " << page->keys.size() << endl;
  for(int i = 0; i < page->children.size(); i++) {
    cout << "child num: " << page->children.at(i)->pageNum << " \t";
  }
  cout << endl;
  for(int i = 0; i < page->children.size(); i++) {
    debugPrint(page->children.at(i));
  }
}

void BTreeIndex::addNodeToTree(int level, BTreePage* node) {
  /*
    cout << "tree size:" << tree.size() << endl;
       cout << "insert:" << max_level-level << endl;
        cout << "Lvl:" << level << endl;

  */
  tree.at(max_level-level).push_back(node);
}

void BTreeIndex::build_tree(vector<DataEntry> entries) {
  uint64_t sum = 0;
  unsigned int num_levels_needed = 0;
  auto low=lower_bound (keys_per_level.begin(), keys_per_level.end(), entries.size());
  //cout << "lower bound: " << low-keys_per_level.begin() << endl;
  //cout << "entry size: " << entries.size() << endl;
  while (entries.size() > sum) {
    sum = keys_per_level[num_levels_needed++];
    //cout << "node for this level: " << keys_per_level[num_levels_needed-1] << endl;
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
      //cout << "leaf not full. add entry: " << i << endl;

      leaf->addKey(entries.at(i).key);
      leaf->rids.push_back(entries.at(i).rid);
    } else {
      //cout << "leaf full for entry: " << i << endl;
      //find latest unfill ancestor
      BTreePage *parent = leaf->parent;
      BTreePage* child = leaf;
      //cout << "child level before traversal: " << leaf->level;

      while((parent != nullptr) && parent->isFull()) {
        child = parent;
        parent = parent->parent;
      }
      //cout << "child level after traversal: " << child->level;

      if (parent == nullptr) {
        //cout << "create new parent" << endl;
        BTreePage* newParent = new BTreePage();
        newParent->level = child->level+1;
        //cout << "parent level: " << child->level+1 << endl;

        newParent->addKey(entries.at(i).key);

        addNodeToTree(newParent->level, newParent);

        parent = newParent;

        parent->addChild(child);

        child->setParent(parent);
      } else {
        parent->addKey(entries.at(i).key);
      }
      //cout << "create new child" << endl;
      BTreePage* newChild = new BTreePage();
      newChild->level = child->level;
      addNodeToTree(newChild->level, newChild);

      parent->addChild(newChild);
      newChild->setParent(parent);

      while (newChild->level > 0) { //if child is not leaf
        //cout << "create grandchild for level: " << newChild->level-1 << endl;
        BTreePage* tmp = new BTreePage();
        tmp->level = newChild->level-1;
        addNodeToTree(tmp->level, tmp);
        newChild->addChild(tmp);
        tmp->setParent(newChild);
        newChild = tmp;
      }

      //cout << "add entry: " << i << " to level: " << newChild->level << endl;
      newChild->addKey(entries.at(i).key); //add current entry to the leaf
      newChild->rids.push_back(entries.at(i).rid);
      leaf = newChild;
    }
  }
  setPageOffset();
}

/*
 * parser that expects a file with 3 tab-delimited columns
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


  return data_entries;
}

/*
 * parser that expects a file with 3 tab-delimited columns
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
