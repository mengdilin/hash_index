#include <cstdint>

struct PageHeader {
  uint32_t overflow_addr;
};

struct DataEntry {
  uint64_t key;
  uint64_t rid;
};

struct Page {
   struct PageHeader* page_header;
   uint32_t counter;
   struct DataEntry* list_head;
};

struct IndexFileHeader {
  int num_buckets;
};

struct IndexFile {
  struct IndexFileHeader* header;
  struct Page* primary_bucket_head;
  struct Page* overflow_bucket_head;
};
