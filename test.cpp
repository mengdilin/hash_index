#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "BTreeIndex.h"
using namespace std;

int main(int argc, char** argv) {
    BTreePage page;
    BTreeIndex btree;
    vector<DataEntry> entries = btree.parse_idx_file(argv[1]);
    btree.build_tree(entries);
    btree.debugPrint();
    vector<BTreePage*> flattened_tree = btree.getFlattenTree(btree.tree);
    uint64_t test;
    std::istringstream ss(argv[2]);
    if (!(ss >> test))
     std::cout << "failed" << std::endl;
    btree.probe(test, flattened_tree);
    return 0;
}
