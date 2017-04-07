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
 /*
int main(int argc, char** argv) {
    string bin_file_path = "4096_data.bin";
    if (argc >= 2) {
        bin_file_path = argv[1];
        cout << "running with index path: " << bin_file_path << endl;
    }
  FILE *c_read_index = fopen(bin_file_path.c_str(),"rb");
  int bin_file = fileno(c_read_index);
  BTreeIndex btree;
  uint64_t key =0;
    if (argc >= 3) {
        istringstream ss(argv[2]);
        if (!(ss >> key)) {
            cout << "set key failed" << endl;
        }
    }

off_t offset =0;
    if (argc >= 4) {
        istringstream ss(argv[3]);
        if (!(ss >> offset)) {
            cout << "set offset failed" << endl;
        }
    }
  auto result = btree.probe_bin(key, bin_file, offset);
  if (result.first) {
    cout << "found offset: " << result.second << endl;
  } else {
    cout << "not found offset" << endl;
  }
    //int bin_file = open(bin_file_path.c_str(), O_RDONLY);

    uint64_t key;
    off_t initial = 0;
    ssize_t size_read = pread(bin_file, (void *)&key, sizeof(key), initial) + initial;
    cout << "key: " << key << endl;
    uint32_t count;
    size_read += pread(bin_file, (void *)&count, sizeof(count), size_read);
    cout << "count: " << count << endl;

    uint32_t length;
    size_read += pread(bin_file, (void *)&length, sizeof(length), size_read);
    cout << "length: " << length << endl;

    off_t new_offset = sizeof(key) + sizeof(count) + sizeof(length) + ceil((float)count/8.0) + count + length ;
    cout << "new_offset: " << new_offset << endl;
    size_read = pread(bin_file, (void *)&key, sizeof(key), new_offset) + new_offset;
    int i = 10;
    off_t old_offset = new_offset;
    while (i-- > 0) {
   cout << "key: " << key << endl;
    uint32_t count;
    size_read += pread(bin_file, (void *)&count, sizeof(count), size_read);
    cout << "count: " << count << endl;

    uint32_t length;
    size_read += pread(bin_file, (void *)&length, sizeof(length), size_read);
    cout << "length: " << length << endl;

    off_t new_offset = sizeof(key) + sizeof(count) + sizeof(length) + ceil((float)count/8.0) + count + length;
    cout << "new relative offset: " << new_offset << endl;
    cout << "new actual offset: " << new_offset+old_offset << endl;
    cout << "ceil(count: " << ceil((float)count/8.0) << endl;
    new_offset += old_offset;
    old_offset = new_offset;
    size_read = pread(bin_file, (void *)&key, sizeof(key), new_offset) + new_offset;

    }

  return 0;

}
 */

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
    string data_bin_path = "/dev/shm/genome/v0/data.bin";
    if (argc >= 5) {
        istringstream ss(argv[4]);
        cout << "running with data bin path: " << data_bin_path << endl;
    }
    cout << "probe key: " << key << endl;
    FILE *c_read_index = fopen(index_path.c_str(),"rb");
    FILE *c_bin_file = fopen(data_bin_path.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    auto result = btree.probe(key,index_fd, data_bin_fd);
    if (result.first) {
        cout << "got value: " << result.second << endl;

    } else {
        cout << "not found: " << key << endl;
    }
    auto t1 = chrono::high_resolution_clock::now();
    for (auto& entry : entries) {

        auto result = btree.probe(entry.key,index_fd, data_bin_fd);
        print_error_if_failed(result, entry.key, entry.rid);
        /*
        if (result.first) {
            cout << "found key: " << entry.key << " with rid: " << result.second << endl;
        } else {
            cout << "not found key: " << entry.key << endl;
        }
        */
    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << "avg nanosec per probe: " << (t2-t1).count()/entries.size() << endl;

    //btree.BfsDebugPrint();

    return 0;
}

