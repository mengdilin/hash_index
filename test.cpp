#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "BTree.h"
using namespace std;

int main(int argc, char** argv) {
    Page page;
    BTree btree(0.8);
    vector<DataEntry> entry = btree.parse_idx_file(argv[1]);
    vector<Page*> pages = btree.get_leaf_pages(entry);
    for (auto& page : pages) {
      cout << page->counter << endl;
    }
    return 0;
}
