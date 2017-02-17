//#include "def.h"
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "HashIndex.h"
using namespace std;

int main(int argc, char** argv) {
  Page page;

  HashIndex index(1);

  uint64_t test_key = 1708146715154;
  string indexFileName = "indexFile";

  cout << "argc: " << argc << endl;
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


  index.build_index(argv[1], indexFileName);


  pair<bool,uint64_t> result = index.search(test_key, indexFileName);
  if (result.first) {
    cout << "found rid: " << result.second << endl;
  }

  /*
  vector<DataEntry> test_data = index.parse_idx_file(argv[1]);
  for (int i = 0; i < test_data.size(); i++) {
    DataEntry test = test_data[i];
    cout << "looking for key: " << test.key << endl;
    pair<bool,uint64_t> result = index.search(test.key, indexFileName);
    if (not result.first) {
    cout << "not found key: " << result.first << endl;
    }
  }
  */
  //index.debugRead(indexFileName);



    return 0;
}
