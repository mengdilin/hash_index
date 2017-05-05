* ./test 1000_entries.idx -probe_file indexFile /dev/shm/genome/v0/data.bin
*  ./test 1000_entries.idx -build indexFile
* ./test 0 -probe_key indexFile /dev/shm/genome/v0/data.bin


## To test the correctness of range probing when developing the code:
* Create a test idx file where entries are in ascending order
* Range probe will be run on each entry in the idx file and check against the
expected result in the test idx file as follows:
  * on a greater-than-and-equal-to range probe, a vector of result on current entry
  from test idx file must then match all (key,rid) pairs after the current entry in test idx file
* To run:
  * Ex: ./test_2048 range_test.idx -range_probe_key_gt_test /dev/shm/btree_test/2048_btree_index /dev/shm/genome/v0/data.bin
  * range_test.idx is the testing idx file that contains all the entries I would like to test the range probe against.
* Testing on more than 1000 entries can be very slow because confirming all results from all probes is O(n^3)
