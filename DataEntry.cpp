#include "DataEntry.h"
#include <fstream>
#include <iostream>

using namespace std;
DataEntry::DataEntry(uint64_t key, uint64_t rid) : key(key), rid(rid) {}

ofstream& DataEntry::flush(ofstream& indexFile) {

  indexFile.write((char*) &(this->key), sizeof(this->key));
  indexFile.write((char*) &(this->rid), sizeof(this->rid));

  //char a = 'a';
  //indexFile.write((char*) &a, 1);
  //cout << "(" << this->key << " , " << this->rid << ")";
  return indexFile;
}

DataEntry DataEntry::read(ifstream& indexFile) {
  DataEntry dataEntry(0, 0);
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

  cout << "(" << dataEntry.key << " , " << dataEntry.rid << ")";
  return dataEntry;
}


