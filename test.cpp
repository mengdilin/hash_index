#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <chrono>
#include <numeric>
#include "BTreeIndex.h"
using namespace std;

void print_error_if_failed(pair<bool, uint64_t>& result, uint64_t& key, uint64_t& rid) {
    if (not result.first) {
        cout << "binary search cannot find key. probe failed for key: " << key << endl;
    } else {
        if (result.second != rid) {
            cout << "proble failed for key: " << key << " expected: " << rid << " but got: " << result.second << endl;
        }
    }
}
int main(int argc, char** argv) {
    BTreePage page;
    BTreeIndex btree;
    vector<DataEntry> entries = btree.parse_idx_file(argv[1]);
    btree.build_tree(entries);
    btree.BfsDebugPrint();


    string index_path = "indexFile";

    btree.flush(index_path);


    uint64_t key = 3;
    if (argc == 3) {
        istringstream ss(argv[2]);
        if (!(ss >> key)) {
            cout << "set key failed" << endl;
        }
    }
    cout << "probe key: " << key << endl;
    FILE *c_read_index = fopen(index_path.c_str(),"rb");
    auto result = btree.probe(key,c_read_index);
    cout << "got value: " << result.second << endl;
    auto t1 = chrono::high_resolution_clock::now();
    for (auto& entry : entries) {

        auto result = btree.probe(entry.key,c_read_index);
        print_error_if_failed(result, entry.key, entry.rid);
    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << "avg nanosec per probe: " << (t2-t1).count()/entries.size() << endl;
    //print_error_if_failed(result, key, )

    //btree.BfsDebugPrint();

    return 0;
}
