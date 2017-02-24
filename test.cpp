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
    BTree btree;
    vector<DataEntry> entry = btree.parse_idx_file(argv[1]);

    return 0;
}
