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
  Page page;

  HashIndex index(1);
  /*
  for (int i = 0; i < 10; i ++) {
    DataEntry a(i, i+1);
    page.addEntry(a);
  }
  uint64_t rid;
  bool found = page.find(1, rid);
  cout << found << " " << rid << endl;
  found = page.find(11, rid);
  cout << found << " " << rid << endl;
  */

  index.build_index(argv[1]);
  uint64_t test_key = 1717836253630;
  pair<bool,uint64_t> result = index.search(test_key, "indexFile");
  if (result.first) {
    cout << "found rid: " << result.second << endl;
  }

  /*
  cout << "find key: " << test_key << endl;
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
  */


    return 0;
}
