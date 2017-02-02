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
  index.build_index(argv[1]);
  /*
  char* input_file = "test.bin";
  if (argc == 2) {
    input_file = argv[1];
  }
  FILE *fp = fopen(input_file, "r");
    if (fp == NULL) {
      printf("error when opening file\n");
  }
  uint64_t key;
  fread(&key, sizeof(uint64_t), 1, fp);

  uint32_t count;
  fread(&count, sizeof(uint32_t), 1, fp);

  uint8_t offset;
  fread(&offset, sizeof(uint8_t), 1, fp);
  cout << "count: " << count << " and key: " << key << " and offset: " << int(offset);

  while (count != 0) {
    if (fread(&offset, sizeof(uint8_t), 1, fp) != 0)
      cout << "new offset: " << int(offset) << '\n';
    else
      break;
    count--;
  }

  fclose(fp);
  */
  /*
  ifstream datafile;
  datafile.open ("test.bin", ios::in | ios::binary);
  //cout << datafile;

  uint64_t key;
  datafile.read(reinterpret_cast<char *>(&key), sizeof(uint64_t));

  uint32_t count;
  datafile.read(reinterpret_cast<char *>(&count), sizeof(uint32_t));
  uint8_t offset;

  datafile.read(reinterpret_cast<char *>(&offset), sizeof(uint8_t));

  cout << "count: " << count << " and key: " << key << " and offset: " << offset;


  datafile.close();
  */
    return 0;
}
