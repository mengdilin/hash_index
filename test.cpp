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
#include <errno.h>
#include <string>
#include <math.h>       /* ceil */

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

void probe_file(string dataIdxFilePath, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    vector<DataEntry> all_entries = btree.parse_idx_file_get_all(dataIdxFilePath);
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    auto t1 = chrono::high_resolution_clock::now();
    for (auto& entry : all_entries) {
        auto result = btree.probe(entry.key,index_fd, data_bin_fd);
        print_error_if_failed(result, entry.key, entry.rid);
    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << "avg nanosec per probe: " << (t2-t1).count()/all_entries.size() << endl;
}

void probe_key(uint64_t key, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    auto result = btree.probe(key,index_fd, data_bin_fd);
    if (result.first) {
        cout << "got rid: " << result.second << endl;

    } else {
        cout << "not found: " << key << endl;
    }
}

void build_index(string dataIdxFilePath, string indexFilePath) {
    BTreeIndex btree;
    vector<DataEntry> entries = btree.parse_idx_file(dataIdxFilePath);
    auto t1 = chrono::high_resolution_clock::now();
    btree.build_tree(entries);
    btree.flush(indexFilePath);
    auto t2 = chrono::high_resolution_clock::now();
    cout << "time used in building (ns): " << (t2-t1).count() << endl;
}
int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "use: ./test <arg> <mode> <arg>..." << endl;
        exit(1);
    }
    string mode = argv[2];
    if (mode == "-build") {
        if (argc < 4) {
            cerr << "use ./test <data_file_path> -build <index_file_path>" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataIdxFilePath = argv[1];
        build_index(dataIdxFilePath, indexFilePath);
    } else if (mode == "-probe_file") {
        if (argc < 5) {
            cerr << "use ./test <data_file_path> -probe_file <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataIdxFilePath = argv[1];
        string dataBinFilePath = argv[4];
        probe_file(dataIdxFilePath, dataBinFilePath, indexFilePath);
    } else if (mode == "-probe_key") {
        if (argc < 5) {
            cerr << "use ./test <key> -probe_file <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        uint64_t key;
        istringstream ss(argv[1]);
        if (!(ss >> key)) {
            cout << "set key failed" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataBinFilePath = argv[4];
        probe_key(key, dataBinFilePath, indexFilePath);
    } else if (mode=="-all") {
        if (argc < 5) {
            cerr << "use ./test <data_file_path> -probe_file <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataIdxFilePath = argv[1];
        string dataBinFilePath = argv[4];
        build_index(dataIdxFilePath, indexFilePath);
        probe_file(dataIdxFilePath, dataBinFilePath, indexFilePath);
    } else {
        cerr << "mode: " << mode << " not supported" << endl;
    }
    /*
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
    vector<DataEntry> all_entries = btree.parse_idx_file_get_all(argv[1]);


    uint64_t key =40;
    if (argc >= 4) {
        istringstream ss(argv[3]);
        if (!(ss >> key)) {
            cout << "set key failed" << endl;
        }
    }
    string data_bin_path = "/dev/shm/genome/v0/data.bin";
    if (argc >= 5) {
        data_bin_path = argv[4];
        cout << "running with data bin path: " << data_bin_path << endl;
    }
    cout << "probe key: " << key << endl;
    FILE *c_read_index = fopen(index_path.c_str(),"rb");
    FILE *c_bin_file = fopen(data_bin_path.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    auto result = btree.probe(key,index_fd, data_bin_fd);
    cout << "here" << endl;;
    if (result.first) {
        cout << "got value: " << result.second << endl;

    } else {
        cout << "not found: " << key << endl;
    }

    auto t1 = chrono::high_resolution_clock::now();
    for (auto& entry : all_entries) {
        //cout << "key: " << entry.key << endl;
        auto result = btree.probe(entry.key,index_fd, data_bin_fd);
        print_error_if_failed(result, entry.key, entry.rid);
    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << "avg nanosec per probe: " << (t2-t1).count()/entries.size() << endl;
    //btree.BfsDebugPrint();
    */
    return 0;
}

