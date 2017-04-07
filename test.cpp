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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
    string bin_file_path = "4096_data.bin";
    if (argc >= 2) {
        bin_file_path = argv[1];
        cout << "running with index path: " << bin_file_path << endl;
    }
    int bin_file = open(bin_file_path, O_RDONLY);
    uint64_t key;
    pread(is, (void *)&key, sizeof(key), 0);

}
/*
int main(int argc, char** argv) {
    BTreePage page;
    BTreeIndex btree;
    vector<DataEntry> entries = btree.parse_idx_file(argv[1]);
    btree.build_tree(entries);
    //btree.BfsDebugPrint();


    string index_path = "indexFile";
    if (argc >= 3) {
       index_path = argv[2];
       cout << "running with index path: " << index_path << endl;
    }

    btree.flush(index_path);


    uint64_t key =40;
    if (argc >= 4) {
        istringstream ss(argv[3]);
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

    //btree.BfsDebugPrint();

    return 0;
}
*/
