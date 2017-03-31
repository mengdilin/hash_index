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


    string index_path = "indexFile";

    btree.flush(index_path);
    FILE *c_read_index = fopen(index_path.c_str(),"rb");
    for (int i = 0; i < 1; i++){
      cout << "key: " << i << endl;
      btree.probe(i,c_read_index);
    }
    btree.BfsDebugPrint();
    return 0;
}
