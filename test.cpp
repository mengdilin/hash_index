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

  HashIndex index(1);

  index.build_index(argv[1]);
  uint64_t test_key = 1708146715154;
  cout << "find key: " << test_key << endl;

  //index.debugRead("indexFile");


  uint64_t offset = index.search(test_key, "indexFile");
  cout << "page offset1: " << offset << endl;
  ifstream is ("indexFile", ifstream::binary);
  cout << "file size: " << is.tellg();
  is.seekg(offset);
  Page curPage = Page::read(is);
  while (curPage.hasOverflow()) {
    cout << "page overflow: " << curPage.overflow_addr << endl;
    offset = 4 + (4 + curPage.overflow_addr-1) * 4096;
    cout << "overflow page offset: " << offset << endl;
    is.seekg(offset);
    curPage = Page::read(is);
  }

    return 0;
}
