#include "BTreeIndex.h"
#include <math.h>
#include <algorithm>
#include <climits>
#include <queue>
#include <cassert>
#include <fstream>

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
    cout << endl;
    cout << "level: " << i <<" with keys: " << sum <<" and with num nodes: " << tree.at(i).size() << endl;

    cout << "max keys: " << pow(BTreePage::fan_out, i)*BTreePage::MAX_KEY_PER_PAGE << " max nodes: " <<  pow(BTreePage::fan_out, i)<< endl;
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
    cout << "page num: " << page->pageNum << " with keys: " << page->keys.size() << endl;
    for(int i = 0; i < page->children.size(); i++) {
      //cout << "child num: " << page->children.at(i)->pageNum << " \t";
      assert(page->children.at(i)->pageNum == page->rids.at(i));
      myqueue.push(page->children.at(i));
    }
    cout << endl;
  }

    cout << "size of root keys: " << tree.at(0).at(0)->keys.size() << endl;
  for (auto& key : tree.at(0).at(0)->keys) {
    cout << key << endl;
  }
}

/*
void BTreeIndex::flush(string indexFilePath) {
  vector<BTreePage*> stream;
  ofstream indexFile;
  indexFile.open(indexFilePath, ios::binary | ios::out);
  uint64_t size = tree.size();
  indexFile.write((char *)&size, sizeof(size));
  for (int i = 0; i < tree.size(); i++) {
    for (int j = 0; j < tree.at(i).size(); j++) {
      //stream.push_back(tree.at(i).at(j));
      tree.at(i).at(j)->flush(indexFile);
    }
  }
}
*/

template<class Iter, class T>
Iter binary_find(Iter begin, Iter end, T val)
{
    // Finds the lower bound in at most log(last - first) + 1 comparisons
    Iter i = lower_bound(begin, end, val);

    if (i != end && !(val < *i))
        return i; // found
    else
        return end; // not found
}

/*
void BTreeIndex::probe(uint64_t key, FILE* indexFile) {
  uint64_t tree_size;
  fread(&tree_size, sizeof(tree_size), 1, indexFile);
  cout << tree_size << endl;
  BTreePage curPage;
  BTreePage::read(indexFile, curPage, false);

  queue<std::pair<uint64_t, int>> myqueue;
  for (auto &rid : curPage.rids) {
    myqueue.push({rid, 0});
  }
  while(!myqueue.empty()) {
    uint64_t rid = myqueue.front().first;
    cout << "page num: " << rid << endl;
    int level = myqueue.front().second;
    myqueue.pop();
    cout << "seek to: " << (rid) * BTreePage::PAGE_SIZE << endl;
    fseek(indexFile, (rid) * BTreePage::PAGE_SIZE + sizeof(uint64_t), SEEK_SET);
    if (level != tree_size - 1)
      BTreePage::read(indexFile, curPage, false);
    else
      BTreePage::read(indexFile, curPage, true);
      cout << "page num: " << rid << endl;


  level ++;
  if (level != tree_size - 1) {
    for (auto &rid : curPage.rids) {

      myqueue.push({rid, level});
    }
    cout << endl;
  }

  }
}

*/

void BTreeIndex::probe(uint64_t key, FILE* indexFile) {
  cout << "key: " << key << endl;
  int level = 1;
  uint64_t tree_size;
  fread(&tree_size, sizeof(tree_size), 1, indexFile);
  cout << "tree size: " << tree_size << endl;
  for (auto &level : tree) {
    cout <<"level nodes: " << level.size() << endl;
  }
  cout << tree.size() << endl;

  BTreePage curPage;
  if (tree_size > 1) {
    BTreePage::read(indexFile, curPage, false);
  } else {
    BTreePage::read(indexFile, curPage, true);
    auto result = curPage.find(key);
    //check result
    return;
  }

  auto result = curPage.find(key);
  while(result.first && level < tree_size) {
    //cout << "child numer: " << result.second << endl;
    //cout << "key: " << key << endl;


    if (level == tree_size-1) {
      cout << "reached max level" << endl;
      cout << "key: " << key << endl;
      cout << "page num: " << result.second << endl;
      fseek(indexFile, result.second * BTreePage::PAGE_SIZE+sizeof(uint64_t), SEEK_SET);
      BTreePage::read(indexFile, curPage, true);
      //curPage = stream.at(result.second);
      cout << "binary find" << endl;
      cout << binary_find(curPage.keys.begin(), curPage.keys.end(), key) - curPage.keys.begin() <<endl;


      for (int i =0; i < curPage.keys.size(); i++) {
        cout << curPage.keys.at(i) << "\t";
      }
      cout << endl;
      for (int i =0; i < curPage.rids.size(); i++) {
        cout << curPage.rids.at(i) << "\t";
      }


      cout << endl;
      break;
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
      cout << "leaf full for entry: " << i << endl;
      //find latest unfill ancestor
      BTreePage *parent = leaf->parent;
      BTreePage* child = leaf;
      cout << "child level before traversal: " << leaf->level;

      while((parent != nullptr) && parent->isFull()) {
        child = parent;
        parent = parent->parent;
      }
      cout << "child level after traversal: " << child->level;

      if (parent == nullptr) {
        //cout << "create new parent" << endl;
        BTreePage* newParent = new BTreePage();
        newParent->level = child->level+1;
        cout << "parent level: " << child->level+1 << endl;

        newParent->addKey(entries.at(i).key);

        addNodeToTree(newParent->level, newParent);

        parent = newParent;

        parent->addChild(child);

        child->setParent(parent);
      } else {
        parent->addKey(entries.at(i).key);
      }
      cout << "create new child" << endl;
      BTreePage* newChild = new BTreePage();
      newChild->level = child->level;
      addNodeToTree(newChild->level, newChild);

      parent->addChild(newChild);
      newChild->setParent(parent);

      while (newChild->level > 0) { //if child is not leaf
        cout << "create grandchild for level: " << newChild->level-1 << endl;
        BTreePage* tmp = new BTreePage();
        tmp->level = newChild->level-1;
        addNodeToTree(tmp->level, tmp);
        newChild->addChild(tmp);
        tmp->setParent(newChild);
        newChild = tmp;
      }
      cout << "add entry: " << i << " to level: " << newChild->level << endl;
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
    //start = end + field_delim.length();
    //end = row.find(field_delim, start);

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
