#include "BTreeIndex.h"
#include <math.h>
#include <algorithm>
#include <climits>
BTreeIndex::~BTreeIndex() {

}

BTreeIndex::BTreeIndex() {
  cout << "MAX_KEY_PER_PAGE: " << BTreePage::MAX_KEY_PER_PAGE << endl;

  int i = 0;
  while (i < max_level) {
    int num_nodes = pow(BTreePage::fan_out, i);
    keys_per_level.push_back(num_nodes*BTreePage::MAX_KEY_PER_PAGE);
    fanout_per_level.push_back(num_nodes*BTreePage::fan_out);

    i++;
    cout << "level with keys: " << keys_per_level[keys_per_level.size()-1] << endl;
  }
}

void BTreeIndex::debugPrint() {
  vector<BTreePage*> flattened_tree;
  for (int i=0; i < tree.size(); i++) {
    int sum = 0;
    for (int j = 0; j < tree.at(i).size(); j++) {
      vector<DataEntry> keys = tree.at(i).at(j)->keys;
      //cout << keys.size() << '\t';
      flattened_tree.push_back(tree.at(i).at(j));
      for (int k=0; k < keys.size(); k++) {
        cout << keys.at(k).key << '\n';
      }
      sum += keys.size();
    }
    cout << endl;
    cout << "level: " << i <<" with keys: " << sum <<" and with num nodes: " << tree.at(i).size() << endl;

    cout << "max has: " << pow(BTreePage::fan_out, i)*BTreePage::MAX_KEY_PER_PAGE << endl;
  }


}

vector<vector<DataEntry>>  BTreeIndex::getFlattenTree(vector<vector<BTreePage*>> &tree) {
  vector<vector<DataEntry>> flattened_tree;

  for (int i=0; i < tree.size(); i++) {
    vector<DataEntry> level;
    flattened_tree.push_back(level);
    for (int j = 0; j < tree.at(i).size(); j++) {
      for (int k = 0; k < (tree.at(i).at(j)->keys).size(); k++) {
        flattened_tree.at(i).push_back((tree.at(i).at(j)->keys).at(k));
      }
    }
  }
  return flattened_tree;
}

vector<int> BTreeIndex::getFirstPageOfLevels(int level) {
  vector<int> result;
  result.push_back(0);
  result.push_back(1);
  for (int i = 2; i <= level; level++) {
    result.push_back(result.at(i-1) + pow(BTreePage::fan_out, i-1));
  }
  return result;
}

int getNumSiblingsAfterMe(
  int my_level,
  int my_page_num,
  vector<int>& first_page_nums,
  vector<int>& total_page_per_level) {
  return total_page_per_level[my_level] - my_page_num;
}

int getNumSiblingsBeforeMe(
  int my_level,
  int my_page_num,
  vector<int>& first_page_nums,
  vector<int>& total_page_per_level) {
  return my_page_num - first_page_nums[my_level];
}

int getNumChildrenBeforeMe(
  int parent_level,
  int parent_index_to_me,
  int parent_page_num,
  vector<int>& first_page_nums,
  vector<int>& total_page_per_level) {
  int siblings_before_parent = getNumSiblingsBeforeMe(
    parent_level,
    parent_page_num,
    first_page_nums,
    total_page_per_level);
  cout << "sibilings before parent: " << siblings_before_parent << endl;
  return (siblings_before_parent+parent_index_to_me)*BTreePage::fan_out;
}

int getPageNum(
  int parent_level,
  int parent_index_to_me,
  int parent_page_num,
  vector<int>& first_page_nums,
  vector<int>& total_page_per_level) {
  int num_children_before_me = getNumChildrenBeforeMe(
    parent_level,
    parent_index_to_me,
    parent_page_num,
    first_page_nums,
    total_page_per_level);
  int num_parents_after_my_parent = getNumSiblingsAfterMe(
    parent_level,
    parent_page_num,
    first_page_nums,
    total_page_per_level);
  cout << "num children before me: " << num_children_before_me << endl;
  cout << "num parents after my parent: " << num_parents_after_my_parent << endl;
  cout << "parent_index_to_me: " << parent_index_to_me << endl;
  return num_children_before_me+num_parents_after_my_parent+parent_index_to_me; //first page starts at 0
}
/* Binary search routine, will return the range of the probe within a level (as though this is the only
 * level in the entire tree); additional calcs done outside of this routine to get the actual range */
int binarysearch(vector<DataEntry>& tree, uint64_t p, int start, int fanout) {
  int i; int l; int r; int res;
  l = start; r = start+fanout-1;
  while (l <= r) {

    i = (l+r)/2;
    if (tree[i].key == p) { // Branch right on equality
      // Check for duplicate INT_MAX keys in front of this one, follow the first pointer
      while (i-1 >= 0 && tree[i-1].key == INT_MAX) {
        i--;
      }
      return i+1;
    } else {
        if (p < tree[i].key) {
        r = i-1;
      } else {
        l = i+1;
      }
    }

  }
  // Element not found
  if (p <= tree[i].key) {
    while (i-1 >= 0 && tree[i-1].key == INT_MAX) {
      i--;
    }
    res = i;
  } else {
    while (i-1 >= 0 && tree[i-1].key == INT_MAX) {
      i--;
    }
    res = i+1;
  }
  return res;
}


/* Each probe goes through this rountine, params are probe, capacity of each level, max number of levels,
 * fanout of each level, this routine returns the correct range of the probe (n+1 ranges given n keys in
 * the tree) */
int binary_search(vector<vector<DataEntry>>& tree, uint64_t p, int maxlvl, vector<int>& fanout) {
  int n; int offset[maxlvl]; int ranges[maxlvl];
  for (n = 0; n < maxlvl; n++) {
    offset[n] = 0;
  }
  for (n = 0; n < maxlvl; n++) {
    int r;
    if (n == 0) { // Root
      r = binarysearch(tree[n], p, 0, fanout[n]-1);
      if (n == maxlvl-1) {
        return r; // For single-level tree, range calculation straightforward
      } else { // Determine offsets for the next level
        offset[0] = r;
        offset[1] = offset[0]*(fanout[1]-1);
      }
    } else {
      /* Use the level's offset (calculated by results of binary search in the parent level)
       * to figure out which node to conduct binary search on */
      int start;
      start = offset[n];
      r = binarysearch(tree[n], p, start, fanout[n]-1);
      ranges[n] = r;
      if (n == maxlvl-1) { // We're done, have to find the range
        // Use the offset of the nodes in preceding levels to figure out how many to skip
        int keysInFront;
          int lev;
        if (r < fanout[n]-1) {
          return r;
        } else {
          keysInFront = 0;
          for (lev = 0; lev < maxlvl; lev++) {
            if (lev == 0){
              keysInFront += offset[0];
            } else {
              keysInFront += ranges[lev];
            }
          }
          return keysInFront;
        }
      } else {
        /* Based on results of this level's binary search, figure out what offset to use
         * for the next level */
        int nodesInFront;
        nodesInFront = start/(fanout[n]-1);
        int pointersInFront;
        pointersInFront = (r - start);
        offset[n+1] = fanout[n]*nodesInFront*(fanout[n+1]-1) + pointersInFront*(fanout[n+1]-1);
      }
    }
  }
  return p;
}


 pair<bool,uint64_t> BTreeIndex::probe(uint64_t key, vector<vector<DataEntry>>& flattened_tree) {
   DataEntry look_for(key, 0);
   int max_level_num = 3-1;
   //find the first pos that is > look_for
   int level = 0;
   cout << "starting to search" << endl;
   int result = binary_search(flattened_tree, key, max_level_num, fanout_per_level);
   cout << "found: " << result << endl;
   //int result = binarysearch(root->keys, key, BTreePage::fan_out);
   /*
   cout << "got result: " << result << endl;

   vector<int> first_page_nums;
   int level_built = 10;
    first_page_nums.push_back(0);
    first_page_nums.push_back(1);

    for (int i = 2; i <= level_built; i++) {
      first_page_nums.push_back(first_page_nums.at(i-1) + pow(BTreePage::fan_out, i-1));
    }
    cout << "finished building first_page_nums " << endl;
    vector<int> total_page_per_level;
    for (int i = 0; i < level_built; i++) {
      total_page_per_level.push_back(pow(BTreePage::fan_out, i));
    }
    cout << "finished building total_page_per_level" << endl;

    int num = getPageNum(
      level,
      result,
      0,
      first_page_nums,
      total_page_per_level);
    cout << "for key: " << key << " we found: " << num << " on level: " << ++level<< endl;
    if (level != max_level_num) {//not leaf level
      BTreePage* parent = flattened_tree.at(num);
    }
    */


    pair <bool,uint64_t> find_result;
    return find_result;
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
