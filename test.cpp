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
#include <cmath>
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
    int bin_file = open(bin_file_path.c_str(), O_RDONLY);
    /*
Within each 4K chunk in data.bin
64bit key:
32bit count  = number of DNA segments hashed to this key
32bit length = length of DNA stream containing all compressed DNA segments
reflags array with length ceiling(count/8)
byte array "offsets" with length count
byte stream with length length.
Thus, the next key in 4K chunk is at: cur_offset + sizeof(key) + sizeof(count) + sizeof(length) + ceiling(count/8) + count + length

    */
    uint64_t key;
    off_t initial = 494369494;
    ssize_t size_read = pread(bin_file, (void *)&key, sizeof(key), 494369494) + initial;
    cout << "key: " << key << endl;
    uint32_t count;
    size_read += pread(bin_file, (void *)&count, sizeof(count), size_read);
    cout << "count: " << count << endl;

    uint32_t length;
    size_read += pread(bin_file, (void *)&length, sizeof(length), size_read);
    cout << "length: " << length << endl;

    off_t new_offset = sizeof(key) + sizeof(count) + sizeof(length) + ceil(count/8) + count + length;
    cout << "new_offset: " << new_offset << endl;
    size_read = pread(bin_file, (void *)&key, sizeof(key), new_offset) + new_offset;
    int i = 10;

    while (i-- > 0) {
   cout << "key: " << key << endl;
    uint32_t count;
    size_read += pread(bin_file, (void *)&count, sizeof(count), size_read);
    cout << "count: " << count << endl;

    uint32_t length;
    size_read += pread(bin_file, (void *)&length, sizeof(length), size_read);
    cout << "length: " << length << endl;

    off_t new_offset = sizeof(key) + sizeof(count) + sizeof(length) + ceil(count/8) + count + length;
    cout << "new_offset: " << new_offset << endl;
    size_read = pread(bin_file, (void *)&key, sizeof(key), new_offset) + new_offset;

    }

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
