#include <iostream>

/**
 * @file DataEntry.h
 * @brief this header file will contain all required definitions
 * and basic utility functions for DataEntry
 */
class DataEntry {
  public:
    uint64_t key;
    uint64_t rid;

public:
  DataEntry();
  DataEntry(uint64_t key, uint64_t rid);
  std::ofstream& flush(std::ofstream&);
  static DataEntry read(std::ifstream&);
  static bool compare(const DataEntry &, const DataEntry &);
};
