#include <iostream>

class DataEntry {
  public:
    uint64_t key;
    uint64_t rid;

public:
  DataEntry();
  DataEntry(uint64_t key, uint64_t rid);
  std::ofstream& flush(std::ofstream&);
  static DataEntry read(std::ifstream&);
};
