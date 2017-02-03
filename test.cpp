//#include "def.h"
#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include "HashIndex.h"
using namespace std;

int main(int argc, char** argv) {
  DataEntry entry(1, 2);
  //cout << entry.key << " " << entry.rid;
  Page page;
  page.addEntry(entry);

  HashIndex index(0.1);

  //index.build_index(argv[1]);
  uint64_t test_key = 1737642124184;
  cout << "find key: " << test_key << endl;
  uint64_t offset = index.search(test_key, "indexFile");
  //uint64_t offset = index.search(test_key);

  cout << "page offset1: " << offset << endl;
  ifstream is ("indexFile", ifstream::binary);
  is.seekg(offset);
  Page::read(is);
    return 0;
}
