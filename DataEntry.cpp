#include "DataEntry.h"
#include <fstream>
#include <iostream>
#include <cstring>

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

  uint64_t test = 1708146715154;
  if (key == test) {
    cout << "key's offset: " << indexFile.tellp() << endl;
  }
  indexFile.write((char*) &(this->key), sizeof(this->key));
  indexFile.write((char*) &(this->rid), sizeof(this->rid));

  //char a = 'a';
  //indexFile.write((char*) &a, 1);
  //cout << "(" << this->key << " , " << this->rid << ")";
  return indexFile;
}

DataEntry DataEntry::read(ifstream& indexFile) {
  DataEntry dataEntry(0,0);
  //uint64_t* rid = NULL;
  char key;
  indexFile >> key;
  //cout >> key >> endl;
  //unsigned long a;
  //indexFile.read((char *) &a, sizeof(unsigned long));
  //cout << key << endl;
  indexFile.read((char *)&dataEntry.key, sizeof(uint64_t));
    indexFile.read((char *)&dataEntry.rid, sizeof(uint64_t));

  //dataEntry.key = *(key);
  //dataEntry.rid = *(rid);

  //cout << "(" << dataEntry.key << " , " << dataEntry.rid << ")";
  return dataEntry;
}


