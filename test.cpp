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
    //vector<DataEntry> entries = btree.parse_idx_file(argv[1]);
    //btree.build_tree(entries);
    //btree.BfsDebugPrint();


    string index_path = "indexFile";

    uint64_t key = 3;
    //btree.flush(index_path);
    if (argc == 3) {
        istringstream ss(argv[2]);
        if (!(ss >> key)) {
            cout << "set key failed" << endl;
        }
    }
    FILE *c_read_index = fopen(index_path.c_str(),"rb");
    btree.probe(key,c_read_index);

    //btree.BfsDebugPrint();

    return 0;
}
