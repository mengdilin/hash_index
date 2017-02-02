#include "DataEntry.h"

DataEntry::DataEntry(uint64_t key, uint64_t rid) : key(key), rid(rid) {}
DataEntry::DataEntry() {
  key = -1;
  rid = -1;
}
