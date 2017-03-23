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

int main(int argc, char** argv) {
  Page page;

  float load_capacity = 1;
  if (argc == 5) {
        string::size_type sz;
        load_capacity = stof (argv[4],&sz);
  }
  cout << "running with capacity: " << load_capacity << endl;
  HashIndex index(load_capacity);

  uint64_t test_key = 1695745814030836708;
  string indexFileName = "indexFile";

  if (argc >= 3) {
    istringstream ss(argv[2]);
    if (!(ss >> test_key)) {
      cout << "set key failed" <<endl;
    }
    cout << "running with " << test_key << " key" <<endl;
  }

  if (argc >= 4) {
    indexFileName = argv[3];
  }

auto t1 = chrono::high_resolution_clock::now();
  index.build_index(argv[1], indexFileName);
auto t2 = chrono::high_resolution_clock::now();
cout << "index build time: " << (t2-t1).count() << endl;
  //initialize the index stream for probing
vector<DataEntry> test_data = index.parse_idx_file(argv[1]);

t1 = chrono::high_resolution_clock::now();
  FILE *c_read_index = fopen(indexFileName.c_str(),"rb");
/*
pair<bool,uint64_t> t_result = index.search(test_key, c_read_index);
    if (not t_result.first) {
    cout << "not found key: " << test_key << endl;
    }
*/

  for (int i = 0; i < test_data.size(); i++) {
    DataEntry test = test_data[i];
    //pair<bool,uint64_t> result = index.search(test.key, readIndex);
    pair<bool,uint64_t> result = index.search(test.key, c_read_index);
    if (not result.first) {
    cout << "not found key: " << test.key << endl;
    } else {
      if (result.second != test.rid) {
        cout << "error: expected: " << test.key << "," <<test.rid << " but got: " << test.key << "," <<result.second <<endl;
      }
    }
  }
t2 = chrono::high_resolution_clock::now();
cout << "index probe time: " << (t2-t1).count() << endl;
    return 0;
}
