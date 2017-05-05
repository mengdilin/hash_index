#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "HashIndex.h"
#include <chrono>
#include <numeric>
using namespace std;

void build_index(string dataFilePath, string indexFileName, float load_capacity) {
    auto t1 = chrono::high_resolution_clock::now();
    HashIndex index(load_capacity);
    index.build_index(dataFilePath, indexFileName);
    auto t2 = chrono::high_resolution_clock::now();
    cout << "index build time (ns): " << chrono::duration_cast<chrono::nanoseconds>(t2-t1).count() << endl;
}

void probe_file(string dataFilePath, string indexFileName, float load_capacity) {
    HashIndex index(load_capacity);
    vector<DataEntry> test_data = index.parse_idx_file(dataFilePath);
    FILE *c_read_index = fopen(indexFileName.c_str(),"rb");
    int fd = fileno(c_read_index);
    auto t1 = chrono::high_resolution_clock::now();
    for (int i = 0; i < test_data.size(); i++) {
    DataEntry test = test_data[i];
    pair<bool,uint64_t> result = index.search(test.key, fd);
    if (not result.first) {
    cout << "not found key: " << test.key << endl;
    } else {
        if (result.second != test.rid) {
            cout << "error: expected: " << test.key << "," <<test.rid << " but got: " << test.key << "," <<result.second <<endl;
            }
        }
    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << "index probe time (ns): " << chrono::duration_cast<chrono::nanoseconds>(t2-t1).count() << endl;
    cout << "average per probe (ns): " << (chrono::duration_cast<chrono::nanoseconds>(t2-t1).count()) / test_data.size() << endl;
}

void probe_key(uint64_t test_key, string indexFileName, float load_capacity) {
    HashIndex index(load_capacity);
    FILE *c_read_index = fopen(indexFileName.c_str(),"rb");
    int fd = fileno(c_read_index);
    pair<bool,uint64_t> t_result = index.search(test_key, fd);
    if (not t_result.first) {
        cout << "not found key: " << test_key << endl;
    } else {
        cout << "found rid: " << t_result.second << endl;
    }
}
int main(int argc, char** argv) {
    if (argc != 5) {
        cerr << "use: ./test <mode> <data_file_path> <index_file_path> <loading_capacity>" << endl;
        exit(1);
    }
    string::size_type sz;
    float load_capacity = stof (argv[4],&sz);
    string indexFileName = argv[3];
    string mode = argv[1];
    if (mode=="-build") {
        string dataFilePath = argv[2];
        cout << "running with data file path: " << dataFilePath << endl;
        cout << "running with index file path: " << indexFileName << endl;
        cout << "running with loading capacity: " << load_capacity << endl;
        build_index(dataFilePath, indexFileName, load_capacity);
    } else if (mode == "-probe_file") {
        string dataFilePath = argv[2];
        cout << "running with data file path: " << dataFilePath << endl;
        cout << "running with index file path: " << indexFileName << endl;
        cout << "running with loading capacity: " << load_capacity << endl;
        probe_file(dataFilePath, indexFileName, load_capacity);
    } else if (mode=="-probe_key") {
        uint64_t test_key;
        istringstream ss(argv[2]);
        if (!(ss >> test_key)) {
            cout << "set key failed" <<endl;
            exit(1);
        }
        cout << "running with key: " << test_key <<endl;
        cout << "running with index file path: " << indexFileName << endl;
        cout << "running with loading capacity: " << load_capacity << endl;
        probe_key(test_key, indexFileName, load_capacity);
    }
    return 0;
}
