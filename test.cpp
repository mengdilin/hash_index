#include <fstream>
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <chrono>
#include <numeric>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <math.h>       /* ceil */
#include <vector>       // std::vector
#include <algorithm>

#include "BTreeIndex.h"
using namespace std;

void print_error_if_failed(pair<bool, uint64_t>& result, uint64_t& key, uint64_t& rid) {
    if (result.first and result.second != rid) {
        cout << "proble failed for key: " << key << " expected: " << rid << " but got: " << result.second << endl;
    }
}

void probe_file(string dataIdxFilePath, string dataBinFilePath, string indexFilePath) {
    uint64_t successful = 0;
    uint64_t unsuccessful = 0;
    BTreeIndex btree;
    vector<DataEntry> all_entries = btree.parse_key_file(dataIdxFilePath);
    cout << "num entries read: " << all_entries.size() << endl;
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    auto t1 = chrono::high_resolution_clock::now();
    for (auto& entry : all_entries) {
        auto result = btree.probe(entry.key,index_fd, data_bin_fd);
        if (result.first) {
            successful++;
        } else {
            unsuccessful++;
        }
    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << "avg microsec per probe: " << chrono::duration_cast<chrono::microseconds>(t2-t1).count()/all_entries.size() << endl;
    cout << "total entry size: " << all_entries.size() << endl;
    cout << "successful probe: " << successful << endl;
    cout << "unsuccessful probe: " << unsuccessful << endl;
}

void probe_file_test(string dataIdxFilePath, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    vector<DataEntry> all_entries = btree.parse_idx_file_get_all(dataIdxFilePath);
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    auto t1 = chrono::high_resolution_clock::now();
    for (auto& entry : all_entries) {
        auto result = btree.probe(entry.key,index_fd, data_bin_fd);
        print_error_if_failed(result, entry.key, entry.rid);
    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << "avg microsec per probe: " << chrono::duration_cast<chrono::microseconds>(t2-t1).count()/all_entries.size() << endl;
}
inline off_t fileSize(int fd) {
   struct stat s;
   if (fstat(fd, &s) == -1) {
      int saveErrno = errno;
      fprintf(stderr, "fstat(%d) returned errno=%d.", fd, saveErrno);
      return(0);
   }
   return(s.st_size);
}
void range_probe_key_get_test(string dataIdxFilePath, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    vector<DataEntry> all_entries = btree.parse_idx_file_get_all(dataIdxFilePath);
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    off_t bin_file_size = fileSize(data_bin_fd); //indicate end of file

    int i = 0;
    for (int i = 0; i < all_entries.size(); i++) {
        auto& entry = all_entries.at(i);
        auto result = btree.range_probe_gt(entry.key, index_fd, data_bin_fd, bin_file_size);
        if (result.size() != all_entries.size()-i) {
            cerr << "expected range result size: " << (all_entries.size()-i) << " but got: " << result.size() << endl;
        }
        /*
        for(auto pair_r : result) {
            auto test_result = btree.probe(pair_r.first,index_fd, data_bin_fd);
            print_error_if_failed(test_result, pair_r.first, pair_r.second);
        }
        */
        for (int j = i; j < all_entries.size(); j++) {
            if (all_entries.at(j).key != result.at(j-i).first or all_entries.at(j).rid != result.at(j-i).second) {
                cout << "expected key: " << all_entries.at(j).key << " rid: " << all_entries.at(j).rid << endl;
                cout << "found key: " << result.at(j-i).first << " rid: " << result.at(j-i).second << endl;
            }

        }

    }
}

void range_probe_key_endpts_test(string dataIdxFilePath, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    cout << "inside range_probe_key_endpts_test" <<endl;
    vector<DataEntry> all_entries = btree.parse_idx_file_get_all(dataIdxFilePath);
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    off_t bin_file_size = fileSize(data_bin_fd); //indicate end of file

    int i = 0;
    int increments = 5;
    for (int i = 0; i < all_entries.size()/increments; i=i+increments) {
        auto& start_entry = all_entries.at(i);
        auto& end_entry = all_entries.at(i+increments-1);
        auto result = btree.range_probe_endpts(start_entry.key, end_entry.key, index_fd, data_bin_fd, bin_file_size);
        if (result.size() != increments-1) {
            cerr << "expected range result size: " << increments-1 << " but got: " << result.size() << endl;
        }
        for (int j = i; j < increments+i; j++) {
            cout << result.at(j-i).first << "," << result.at(j-i).second << endl;
            if (all_entries.at(j).key != result.at(j-i).first or all_entries.at(j).rid != result.at(j-i).second) {
                cout << "expected key: " << all_entries.at(j).key << " rid: " << all_entries.at(j).rid << endl;
                cout << "found key: " << result.at(j-i).first << " rid: " << result.at(j-i).second << endl;
            }
        }
    }
}


void range_probe_key_gt(uint64_t key, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    off_t bin_file_size = fileSize(data_bin_fd); //indicate end of file
    auto result = btree.range_probe_gt(key, index_fd, data_bin_fd, bin_file_size);
    cout << "found keys: " << result.size() << endl;
}

void generate_range_sample_for_test(string dataIdxFilePath, string probeIdxFilePath) {
    BTreeIndex btree;
    vector<DataEntry> all_entries = btree.parse_key_file(dataIdxFilePath);
    vector<DataEntry> start_probe_entries = btree.parse_key_file(probeIdxFilePath);

    int num_probes = start_probe_entries.size();
    for (int i = 0; i < num_probes; i++) {
        auto result = lower_bound (all_entries.begin(), all_entries.end(), start_probe_entries.at(i), DataEntry::compare);
        if (result !=  all_entries.end()) {
            if (all_entries.end() - result > 1000) {
                cout << start_probe_entries.at(i).key << '\t' << (result + 1000)->key << '\t' << 1000 << endl;
            } else {
                cout << start_probe_entries.at(i).key << '\t' << (all_entries.end()-1)->key << '\t' << all_entries.end() - result << endl;
            }

        }
    }
}

void range_probe_key_endpts(string probeIdxFilePath, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    vector<pair<DataEntry, int>> all_entries = btree.parse_sample_range_probe_idx(probeIdxFilePath);

    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    off_t bin_file_size = fileSize(data_bin_fd); //indicate end of file
    cout << "entry size: " << all_entries.size() << endl;

    auto t1 = chrono::high_resolution_clock::now();

    for (int i = 0; i < all_entries.size(); i++) {
        auto result = btree.range_probe_endpts((all_entries.at(i).first).key, (all_entries.at(i).first).rid, index_fd, data_bin_fd, bin_file_size);
        if (result.size() != all_entries.at(i).second+1) {
            cout << "got size: " << result.size() << " but expected: " << all_entries.at(i).second << endl;
        }

    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << "avg microsec per probe: " << chrono::duration_cast<chrono::microseconds>(t2-t1).count()/all_entries.size() << endl;

}

void range_probe_file_key_endpts(uint64_t start_key, uint64_t end_key, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    off_t bin_file_size = fileSize(data_bin_fd); //indicate end of file
    auto result = btree.range_probe_endpts(start_key, end_key, index_fd, data_bin_fd, bin_file_size);
    cout << "found keys: " << result.size() << endl;
}

void range_probe_key_lt(uint64_t key, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    off_t bin_file_size = fileSize(data_bin_fd); //indicate end of file
    auto result = btree.range_probe_lt(key, index_fd, data_bin_fd);
    cout << "found keys: " << result.size() << endl;
}

void probe_key(uint64_t key, string dataBinFilePath, string indexFilePath) {
    BTreeIndex btree;
    FILE *c_read_index = fopen(indexFilePath.c_str(),"rb");
    FILE *c_bin_file = fopen(dataBinFilePath.c_str(), "rb");
    int index_fd = fileno(c_read_index);
    int data_bin_fd = fileno(c_bin_file);
    auto result = btree.probe(key,index_fd, data_bin_fd);
    if (result.first) {
        cout << "got rid: " << result.second << endl;

    } else {
        cout << "not found: " << key << endl;
    }
}

void build_index(string dataIdxFilePath, string indexFilePath) {
    BTreeIndex btree;
    vector<DataEntry> entries = btree.parse_idx_file(dataIdxFilePath);
    auto t1 = chrono::high_resolution_clock::now();
    btree.build_tree(entries);
    btree.flush(indexFilePath);
    auto t2 = chrono::high_resolution_clock::now();
    cout << "time used in building (microsec): " << chrono::duration_cast<chrono::microseconds>(t2-t1).count() << endl;
}
int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "use: ./test <arg> <mode> <arg>..." << endl;
        exit(1);
    }
    string mode = argv[1];
    if (mode == "-build") {
        if (argc < 4) {
            cerr << "use ./test  -build <data_file_path> <index_file_path>" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataIdxFilePath = argv[2];
        build_index(dataIdxFilePath, indexFilePath);
    } else if (mode == "-probe_file") {
        if (argc < 5) {
            cerr << "use ./test -probe_file <data_file_path> <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataIdxFilePath = argv[2];
        string dataBinFilePath = argv[4];
        probe_file(dataIdxFilePath, dataBinFilePath, indexFilePath);
    } else if (mode == "-probe_file_test") {
        if (argc < 5) {
            cerr << "use ./test -probe_file <data_file_path> <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataIdxFilePath = argv[2];
        string dataBinFilePath = argv[4];
        probe_file_test(dataIdxFilePath, dataBinFilePath, indexFilePath);
    } else if (mode == "-probe_key") {
        if (argc < 5) {
            cerr << "use ./test -probe_key <key> <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        uint64_t key;
        istringstream ss(argv[2]);
        if (!(ss >> key)) {
            cout << "set key failed" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataBinFilePath = argv[4];
        probe_key(key, dataBinFilePath, indexFilePath);
    } else if (mode=="-all") {
        if (argc < 5) {
            cerr << "use ./test -probe_file <data_file_path> <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataIdxFilePath = argv[2];
        string dataBinFilePath = argv[4];
        build_index(dataIdxFilePath, indexFilePath);
        probe_file(dataIdxFilePath, dataBinFilePath, indexFilePath);
    } else if (mode == "-range" ) {
        uint64_t key;
        istringstream ss(argv[2]);
        if (!(ss >> key)) {
            cout << "set key failed" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataBinFilePath = argv[4];
        string dataIdxFilePath = argv[5];
        build_index(dataIdxFilePath, indexFilePath);
        range_probe_key_gt(key, dataBinFilePath, indexFilePath);

    } else if (mode == "-range_probe_key_gt") {
        if (argc < 5) {
            cerr << "use ./test -range_probe_key_gt <key> <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        uint64_t key;
        istringstream ss(argv[2]);
        if (!(ss >> key)) {
            cout << "set key failed" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataBinFilePath = argv[4];
        range_probe_key_gt(key, dataBinFilePath, indexFilePath);
    } else if (mode == "-range_probe_key_endpts") {
        if (argc < 4) {
            cerr << "use ./test -range_probe_key_endpts <dataFilePath> <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        string dataIdxFilePath = argv[2];
        string indexFilePath = argv[3];
        string dataBinFilePath = argv[4];
        range_probe_key_endpts(dataIdxFilePath, dataBinFilePath, indexFilePath);
    } else if (mode == "-range_probe_key_lt") {
        if (argc < 5) {
            cerr << "use ./test -range_probe_key_lt <key> <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        uint64_t key;
        istringstream ss(argv[2]);
        if (!(ss >> key)) {
            cout << "set key failed" << endl;
            exit(1);
        }
        string indexFilePath = argv[3];
        string dataBinFilePath = argv[4];
        range_probe_key_lt(key, dataBinFilePath, indexFilePath);
    } else if (mode == "-range_probe_key_gt_test") {
        if (argc < 5) {
            cerr << "use ./test -range_probe_key_gt_test <data_file_path> <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        string dataIdxFilePath = argv[2];
        string indexFilePath = argv[3];
        string dataBinFilePath = argv[4];
        range_probe_key_get_test(dataIdxFilePath, dataBinFilePath, indexFilePath);
    } else if (mode == "-range_probe_key_endpts_test") {
        if (argc < 5) {
            cerr << "use ./test -range_probe_key_gt_test <data_file_path> <index_file_path> <binary_data_file_path>" << endl;
            exit(1);
        }
        string dataIdxFilePath = argv[2];
        string indexFilePath = argv[3];
        string dataBinFilePath = argv[4];
        range_probe_key_endpts_test(dataIdxFilePath, dataBinFilePath, indexFilePath);
    } else if (mode == "-generate_range_sample_for_test") {
        if (argc < 3) {
            cerr << "use ./test -generate_range_sample_for_test <sorted_idx_data_file_path> <random_idx_file_path>" << endl;
            exit(1);
        }
        string dataIdxFilePath = argv[2];
        string probeFilePath = argv[3];
        generate_range_sample_for_test(dataIdxFilePath, probeFilePath);
    }
    else {
        cerr << "mode: " << mode << " not supported" << endl;

    }
    return 0;
}

