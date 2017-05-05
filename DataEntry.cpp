#include "DataEntry.h"
#include <fstream>
#include <iostream>
#include <cstring>

/**
 * @file DataEntry.cpp
 * @brief this file implements DataEntry object which is
 * solely used to represent a row in a idx file when parsing
 * an idx file
 */

using namespace std;

DataEntry::DataEntry() {
  memset(&key, 0, sizeof(key));
    memset(&rid, 0, sizeof(rid));

}

bool DataEntry::compare(const DataEntry &a, const DataEntry &b) {
    return a.key < b.key;
}

DataEntry::DataEntry(uint64_t key, uint64_t rid) : key(key), rid(rid) {}

ofstream& DataEntry::flush(ofstream& indexFile) {
  indexFile.write((char*) &(this->key), sizeof(this->key));
  indexFile.write((char*) &(this->rid), sizeof(this->rid));

  return indexFile;
}

DataEntry DataEntry::read(ifstream& indexFile) {
  DataEntry dataEntry(0,0);
  char key;
  indexFile >> key;
  indexFile.read((char *)&dataEntry.key, sizeof(uint64_t));
    indexFile.read((char *)&dataEntry.rid, sizeof(uint64_t));
  return dataEntry;
}


