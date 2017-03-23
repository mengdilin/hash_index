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
    //btree.BfsDebugPrint();
    for (int i = 0; i < 1000; i++){
      cout << "key: " << i << endl;
      btree.probe(i,btree.get_simulated_stream());
    }

    return 0;
}
