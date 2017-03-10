#include "BTreeIndex.h"
#include <math.h>
#include <algorithm>

BTreeIndex::~BTreeIndex() {

}

BTreeIndex::BTreeIndex() {
  cout << "MAX_KEY_PER_PAGE: " << BTreePage::MAX_KEY_PER_PAGE << endl;

  int i = 0;
  while (i < max_level) {
    keys_per_level.push_back(pow(BTreePage::fan_out, i)*BTreePage::MAX_KEY_PER_PAGE);
    i++;
    cout << "level with keys: " << keys_per_level[keys_per_level.size()-1] << endl;
  }
}

void BTreeIndex::debugPrint() {
  for (int i=0; i < tree.size(); i++) {
    int sum = 0;
    for (int j = 0; j < tree.at(i).size(); j++) {
      vector<DataEntry> keys = tree.at(i).at(j)->keys;
      //cout << keys.size() << '\t';

      for (int k=0; k < keys.size(); k++) {
        cout << keys.at(k).key << '\t';
      }
      sum += keys.size();
    }
    cout << endl;
    cout << "level: " << i <<" with keys: " << sum <<" and with num nodes: " << tree.at(i).size() << endl;

    cout << "max has: " << pow(BTreePage::fan_out, i)*BTreePage::MAX_KEY_PER_PAGE << endl;
  }

}

void BTreeIndex::addNodeToTree(int level, BTreePage* node) {
   // cout << "tree size:" << tree.size() << endl;
     //   cout << "insert:" << max_level-level << endl;
       // cout << "Lvl:" << level << endl;


  tree.at(max_level-level).push_back(node);
}

void BTreeIndex::build_tree(vector<DataEntry> entries) {
  unsigned int sum = 0;
  unsigned int num_levels_needed = 0;
  auto low=lower_bound (keys_per_level.begin(), keys_per_level.end(), entries.size());
  cout << "lower bound: " << low-keys_per_level.begin() << endl;
  cout << "entry size: " << entries.size() << endl;
  while (entries.size() > sum) {
    sum = keys_per_level[num_levels_needed++];
    cout << "node for this level: " << keys_per_level[num_levels_needed-1] << endl;
    vector<BTreePage*> cur_level;
    tree.push_back(cur_level);
  }
  max_level = num_levels_needed-1;
  cout << "max level: " << max_level << endl;

  BTreePage* leaf = new BTreePage();
  leaf->level = 0;
  cout << "here" << endl;
  addNodeToTree(leaf->level, leaf);

  for (int i = 0; i < entries.size(); i++) {
    if (!leaf->isFull()) {
      cout << "leaf not full. add entry: " << i << endl;
      leaf->addKey(entries.at(i));
    } else {
      cout << "leaf full for entry: " << i << endl;
      //find latest unfill ancestor
      BTreePage *parent = leaf->parent;
      BTreePage* child = leaf;
      while((parent != nullptr) && parent->isFull()) {
        child = parent;
        parent = parent->parent;
      }
      if (parent == nullptr) {
        cout << "create new parent" << endl;
        BTreePage* newParent = new BTreePage();
        newParent->level = child->level+1;
        newParent->addKey(entries.at(i));
        addNodeToTree(newParent->level, newParent);
        parent = newParent;
        parent->addChild(child);
        child->setParent(parent);
      } else {
        parent->addKey(entries.at(i));
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
      newChild->addKey(entries.at(i)); //add current entry to the leaf
      leaf = newChild;
    }
  }
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
