#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <ammintrin.h>
#include <immintrin.h>
#include <x86intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <limits.h> /* for max int */
#include <time.h>
#include <stdarg.h> /* to accept variable arguments */
#include <stdint.h>
#include <assert.h>
#include <math.h> /* for ceiling function */

int indexOf(int* array, int element, int len);
void hard_coded_probe(
  int* probe,
  __m128i lvl_0_A,
  __m128i lvl_0_B,
  int** tree) ;
int probe_fanout_seventeen(int prev_offset, int* cur_level, __m128i k1);
int probe_fanout_nine(int prev_offset, int* cur_level, __m128i k1);
int probe_fanout_five(int prev_offset, int* cur_level, __m128i k1);
int (*probe_funct[17]) (int prev_offset, int* cur_level, __m128i k1);
void regular_probe(int size, int levels, int numProbes, int* probes, int* fanout, int** tree);

// Provided code to generate and sort random numbers
typedef struct {
    size_t index;
    uint32_t num[625];
} rand32_t;

rand32_t *rand32_init(uint32_t x)
{
    rand32_t *s = malloc(sizeof(rand32_t));
    uint32_t *n = s->num;
    size_t i = 1;
    n[0] = x;
    do {
        x = 0x6c078965 * (x ^ (x >> 30));
        n[i] = x;
    } while (++i != 624);
    s->index = i;
    return s;
}

uint32_t rand32_next(rand32_t *s)
{
    uint32_t x, *n = s->num;
    size_t i = s->index;
    if (i == 624) {
        i = 0;
        do {
            x = (n[i] & 0x80000000) + (n[i + 1] & 0x7fffffff);
            n[i] = (n[i + 397] ^ (x >> 1)) ^ (0x9908b0df & -(x & 1));
        } while (++i != 227);
        n[624] = n[0];
        do {
            x = (n[i] & 0x80000000) + (n[i + 1] & 0x7fffffff);
            n[i] = (n[i - 227] ^ (x >> 1)) ^ (0x9908b0df & -(x & 1));
        } while (++i != 624);
        i = 0;
    }
    x = n[i];
    x ^= (x >> 11);
    x ^= (x <<  7) & 0x9d2c5680;
    x ^= (x << 15) & 0xefc60000;
    x ^= (x >> 18);
    s->index = i + 1;
    return x;
}

int int32_cmp(const void *x, const void *y)
{
    int32_t a = * (const int*) x;
    int32_t b = * (const int*) y;
    return a < b ? -1 : a > b ? 1 : 0;
}

int32_t *generate(size_t n, rand32_t *gen)
{
    size_t i;
    int32_t *a = malloc(n << 2);
    for (i = 0 ; i != n ; ++i)
        a[i] = rand32_next(gen);
    return a;
}

int32_t *generate_sorted_unique(size_t n, rand32_t *gen)
{
    size_t i = 0;
    size_t m = n / 0.7;
    uint8_t z = 0;
    uint32_t *a = malloc(n << 2);
    uint32_t *b = calloc(m, 4);
    while (i != n) {
        uint32_t k = rand32_next(gen);
        if (k != 0) {
            size_t h = (uint32_t) (k * 0x9e3779b1);
            h = (h * (uint64_t) m) >> 32;
            while (b[h] != k) {
                if (b[h] == 0) {
                    b[h] = a[i++] = k;
                    break;
                }
                if (++h == m) h = 0;
            }
        } else if (z == 0) {
            a[i++] = 0;
            z = 1;
        }
    }
    free(b);
    qsort(a, n, 4, int32_cmp);
    return (int32_t*) a;
}

void ratio_per_bit(const int32_t *a, size_t n)
{
    size_t i, j, *c = calloc(32, sizeof(size_t));
    for (i = 0 ; i != n ; ++i) {
        //int32_t x = a[i];
        for (j = 0 ; j != 32 ; ++j)
            c[j] += (a[i] >> j) & 1;
    }
    for (j = 0 ; j != 32 ; ++j)
        fprintf(stderr, "%2ld: %.2f%%\n", j + 1, c[j] * 100.0 / n);
    free(c);
}

// Print error and exit
static void die(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

typedef struct {
  int range; // Holds the relative range of a level after binary search
} Result;


int main(int argc, char *argv[]) {
  /*
  int root_copy[8] = {0, 500, 600, 700, 800, 900, 1000,
                      INT_MAX};
                      //1100};
  int level_1_copy[9*4] = {-100, -90, -80, -70,
                  10, 100, 200, 300,
                  510, 520, 530, 540,
                  610, 620, 630, 640,
                  710, 720, 730, 740,
                  810, 820, 830, 840,
                  910, 920, 930, 940,
                  1010, 1020, 1030, 1040,
                  INT_MAX};
                  //1110, 1120, 1130, 1140};

                  //number of parent nodes * parent's fanout * my fanout
  int level_2_copy[(9*5)*8] = {-200, -190, -180, -170, -160, -150, -140, -130, //[0:8]
                        -99, -98, -97, -96, -95, -94, -93, -92, //[9:17]
                        -89, -88, -87, -86, -85, -84, -83, -82, //[18:26]
                        -79, -78, -77, -76, -75, -74, -73, -72, //[27:35]
                        -69, -68, -67, -66, -65, -63, -62, -61, //[36:44]

                        1, 2, 3, 4, 5, 6, 7, 8,                 //[45:53]
                        20, 30, 40, 50, 60, 70, 80, 90,         //[54:62]
                        120, 130, 140, 150, 160, 170, 180, 190, //[63:71]
                        220, 230, 240, 250, 260, 270, 280, 290, //[72:80]
                        320, 330, 340, 350, 360, 370, 380, 390, //[81:89]

                        501, 502, 503, 504, 505, 506, 507, 508, //[90:98]
                        511, 512, 513, 514, 515, 516, 517, 518, //[99:107]
                        521, 522, 523, 524, 525, 526, 527, 528, //[108:116]
                        531, 532, 533, 534, 535, 536, 537, 538, //[117:125]
                        541, 542, 543, 544, 545, 546, 547, 548, //[126:134]

                        601, 602, 603, 604, 605, 606, 607, 608, //[135:143]
                        611, 612, 613, 614, 615, 616, 617, 618, //[144:152]
                        621, 622, 623, 624, 625, 626, 627, 628, //[153:161]
                        631, 632, 633, 634, 635, 636, 637, 638, //[162:170]
                        641, 642, 643, 644, 645, 646, 647, 648, //[171:179]

                        701, 702, 703, 704, 705, 706, 707, 708, //[180:188]
                        711, 712, 713, 714, 715, 716, 717, 718, //[189:197]
                        721, 722, 723, 724, 725, 726, 727, 728, //[198:206]
                        731, 732, 733, 734, 735, 736, 737, 738, //[207:215]
                        741, 742, 743, 744, 745, 746, 747, 748, //[216:224]

                        801, 802, 803, 804, 805, 806, 807, 808, //[225:233]
                        811, 812, 813, 814, 815, 816, 817, 818, //[234:242]
                        821, 822, 823, 824, 825, 826, 827, 828, //[243:251]
                        831, 832, 833, 834, 835, 836, 837, 838, //[252:260]
                        841, 842, 843, 844, 845, 846, 847, 848, //[261:269]

                        901, 902, 903, 904, 905, 906, 907, 908, //[270:278]
                        911, 912, 913, 914, 915, 916, 917, 918, //[279:287]
                        921, 922, 923, 924, 925, 926, 927, 928, //[288:296]
                        931, 932, 933, 934, 935, 936, 937, 938, //[297:305]
                        941, 942, 943, 944, 945, 946, 947, 948, //[306:314]

                        1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, //[315:323]
                        1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, //[324:332]
                        1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, //[333:341]
                        1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, //[342:350]
                        1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, //[351:359]

                        //INT_MAX, INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,
                        //INT_MAX, INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,
                        //INT_MAX, INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,
                        //INT_MAX, INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,
                        INT_MAX, INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX
                      };


                        //1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, //[360:368]
                        //1111, 1112, 1113, 1114, 1115, 1116, 1117, 1118, //[369:377]
                        //1121, 1122, 1123, 1124, 1125, 1126, 1127, 1128, //[378:386] //43nd node
                        //1131, 1132, 1133, 1134, 1135, 1136, 1137, 1138, //[387:395] //44th node
                        //1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148 //[396:404] //45th node
                        //};


  // data must be aligned by 16 bytes for _mm_load_si128 to work
  int *root, *level_1, *level_2, *key_val, *probe;
  posix_memalign((void **) &root, 16, sizeof(int)*8);
  posix_memalign((void **) &level_1, 16, sizeof(int)*9*4);
  posix_memalign((void **) &level_2, 16, sizeof(int)*(9*4+1)*8);
  posix_memalign((void **) &key_val, 16, sizeof(int));
  posix_memalign((void **) &probe, 16, sizeof(int)*8);
  printf("addr lvl_1 %p\n", level_1);
  printf("addr lvl_2 %p\n", level_2);
  printf("pos of 1148 %d\n", level_2_copy[359]);
  int i;
  for (i=0; i<sizeof(level_1_copy)/sizeof(int); i++) {
    level_1[i] = level_1_copy[i];
    level_2[i] = level_2_copy[i];
  }
  for (i=sizeof(level_1_copy)/sizeof(int); i<sizeof(level_2_copy)/sizeof(int); i++) {
    level_2[i] = level_2_copy[i];
  }
  for (i=0; i<sizeof(root_copy)/sizeof(int); i++) {
    root[i] = root_copy[i];
    printf("root %d\n", root[i]);
  }
  probe[0] = 0;
  probe[1] = 72;
  probe[2] = 701;
  probe[3] = 801;
  probe[4] = 901;
  probe[5] = 1001;
  probe[6] = 1048;
  probe[7] = INT_MAX;
  *(key_val) = 1001;
  */

    // parameter setting and checking
  if (argc < 4) {
    die("Invoke: build [numKeys] [numProbes] [fanout per level]");
  }
  int K = atoi(argv[1]); int P = atoi(argv[2]);
  int levels = argc - 3;
  if (levels < 1) {
    die("Tree should have at least one level.");
  }
  if (K < 1) {
    die("Should provide at least one key.");
  }
  if (P < 1) {
    die("Should provide at least one probe.");
  }
  int fanout[levels];
  int Fsize = sizeof(fanout)/sizeof(fanout[0]);
  fprintf(stderr, "K: %d, P: %d, Levels: %d\n", K, P, levels);
  int i;
  for(i = 0; i < argc-3; i++) {
    fanout[i] = atoi(argv[i+3]);
    if (fanout[i] != 5 && fanout[i] != 9 && fanout[i] != 17) {
      die("Only fanouts of 5, 9, and 17 are supported.");
    }
    fprintf(stderr, "Fanout at level %d: %d\n", i+1, fanout[i]);
  }

  int maxKeys = fanout[0]-1;
  int totalFanout = fanout[0];
  int *capacity;
  if ((capacity = malloc(levels*sizeof(int))) == NULL) {
    die("malloc of capacity failed.");
  }
  // Calculate the max amount of keys that this tree can hold
  capacity[0] = fanout[0]-1;
  fprintf(stderr, "Max keys at level %d: %d\n", 1, maxKeys);
  for (i = 1; i < Fsize; i++) {
    int maxKeyAtLvl = totalFanout*(fanout[i]-1);
    fprintf(stderr, "Max keys at level %d: %d\n", i+1, maxKeyAtLvl);
    capacity[i] = maxKeyAtLvl;
    maxKeys += maxKeyAtLvl;
    totalFanout *= fanout[i];
  }
  // Exit if too many keys for the specified level fanouts
  fprintf(stderr, "Max keys: %d, Specified Keys: %d\n\n", maxKeys, K);
  if (K > maxKeys) {
    free(capacity);
    die("Too many build keys for the given fanouts.");
  }

  rand32_t *gen = rand32_init(time(NULL));

  // Save the randomly generated probes into an array
  size_t j, numProbes = argc > 1 ? atoll(argv[2]) : P;
  int32_t *probe;
  int size = numProbes;
  if (numProbes % 4 != 0) {
   size = numProbes + (4- (numProbes % 4));
  }
  posix_memalign((void **) &probe, 16, sizeof(int)*size);
  int32_t *probes = generate_sorted_unique(numProbes, gen);

    for (j = 0 ; j != numProbes; ++j) {
      probe[j] = probes[j];
    fprintf(stderr, "Probe %lu: %d\n", j+1, probes[j]);
    }
  fprintf(stderr, "\n");
    ratio_per_bit(probes, numProbes);
  fprintf(stderr, "\n");

  // Save the randomly generated keys into an array
    size_t k, numKeys = argc > 1 ? atoll(argv[1]) : maxKeys;
  int32_t *keys = generate_sorted_unique(numKeys, gen);
      for (k = 0 ; k != numKeys; ++k) {
          fprintf(stderr, "Key %lu: %d\n", k+1, keys[k]);
    }
  fprintf(stderr, "\n");
    ratio_per_bit(keys, numKeys);
  fprintf(stderr, "\n");

  free(gen);

  /* Array to store how much of each level will be filled in each level of the tree
     and by extension how much to allocate for each level */
  int *fillState;
  if ((fillState = malloc(levels*sizeof(int))) == NULL) {
    free(capacity);
    free(probes);
    free(keys);
    die("calloc of fillState failed");
  }

  // This is needed to determine whether to fill the current non-leaf node or one of its ancestors
  int *nodeFull;
  if ((nodeFull = malloc(levels*sizeof(int))) == NULL) {
    free(capacity);
    free(probes);
    free(keys);
    free(fillState);
    die("calloc of nodeFull failed");
  }

  int n;
  for (n = 0; n < levels; n++) {
    fillState[n] = 0;
    nodeFull[n] = 0;
  }

  // Pre-determine how much to allocate for each level
  int m;
  int curlvl = levels-1;
  for (m = 0; m < numKeys; m++) {
    if (curlvl == levels-1) { // leaf level
      if(++fillState[curlvl]%(fanout[curlvl]-1) == 0) { // move to the level above, current leaf node full
        curlvl--;
      }
    }  else {
      int nodeFilled = nodeFull[curlvl]%(fanout[curlvl]-1);
      while(nodeFull[curlvl] > 0 && nodeFilled == 0 && curlvl > 0) { // move up to level above, current non-leaf node full
        nodeFull[curlvl] = 0;
        curlvl--;
        nodeFilled = nodeFull[curlvl]%(fanout[curlvl]-1);
      }
      fillState[curlvl]++;
      nodeFull[curlvl]++;
      curlvl = levels-1; // go back to filling leaves
    }
  }

  // Exit the program with error if the root node is empty (too few keys)
  if (fillState[0] < 1) {
    free(capacity);
    free(probes);
    free(keys);
    free(fillState);
    free(nodeFull);
    die("Too few keys, root is empty.");
  }

  int maxes[levels]; // Need to see how much of each level is padded with INT_MAXs
  int q; int s;
  for (q = 0; q < levels; q++) {
    maxes[q] = 0;
  }

  // Add INT_MAXs to tree nodes that are not filled
  for(q = 0; q < levels; q++) {
    if (q == 0 && fillState[q] < capacity[q]) { // Fill out remainder of the root node
      for (s = fillState[q]; s < capacity[q]; s++) {
        fillState[q]++;
        maxes[q]++;
      }
    } else if (fillState[q] < capacity[q]) {
      // Padding the node is necessary; figure out how many INT_MAXs we need to fill hte node
      if (fillState[q]%(fanout[q]-1) != 0) {
        int extra;
        int start;
        int next;
        next = fillState[q]/(fanout[q]-1) + 1;
        extra = next*(fanout[q]-1) - fillState[q];
        start = fillState[q];
        for (s = start; s < start+extra; s++) {
          ++fillState[q];
          maxes[q]++;
        }
      }
    }
  }

  // Check to see if there are enough nodes in the middle levels; if not then add extra node with only INT_MAXs
  int ls;
  for (ls = levels-2; ls > 0; ls--) {
    int prevLayer; int thisLayer; int shouldHave;
    prevLayer = fillState[ls+1]/(fanout[ls+1]-1);
    thisLayer = fillState[ls]/(fanout[ls]-1);
      shouldHave = ceil((double)prevLayer/(fanout[ls]));
    if (thisLayer < shouldHave) {
      fillState[ls] += fanout[ls]-1;
      maxes[ls] += fanout[ls]-1;
    }
  }

  // Stores the old fillState (before possible modification in the next for loop)
  int oldState[levels];
  for (ls = 0; ls < levels; ls++) {
    oldState[ls] = fillState[ls];
  }

  /* Check to see if there are enough child nodes given the current capacity of this
   * level, not counting the pointers to the right of INT_MAX keys; if this is not
     done then there may be out of range errors for sparse trees */
  for (ls = 1; ls < levels; ls++) {
    int shouldHave; int actual;
    if (ls-1 == 0) {
      shouldHave = 1+(oldState[ls-1]-maxes[ls-1]);
    } else {
      shouldHave = fanout[ls-1]*ceil((double)(oldState[ls-1]-maxes[ls-1])/(fanout[ls-1]-1));
    }
    actual = ceil((double)oldState[ls]/(fanout[ls]-1));
    if (actual < shouldHave) {
      int add;
      add = (shouldHave-actual)*(fanout[ls]-1);
      fillState[ls] += add;
      maxes[ls] += add;
    }
  }

  // Now allocate the actual tree with the correct capacities, as determined above
  int **tree;
    if ((tree = malloc(levels*sizeof(int*))) == NULL) {
    free(capacity);
    free(probes);
    free(keys);
    free(fillState);
    free(nodeFull);
    die("malloc of tree failed");
  }

  int l;
  int prevFanout = 1;
  for (l = 0; l < levels; l++) {
    posix_memalign((void *)&tree[l], 16, fillState[l]*sizeof(int*));
    if (tree[l] == NULL) {
      free(capacity);
      free(probes);
      free(keys);
      int o;
      for (o = 0; o < l; o++) {
        free(tree[o]);
      }
      free(tree);
      free(fillState);
      free(nodeFull);
      die("malloc of tree levels failed");
    }
    prevFanout *= fanout[l]; // in preparation for next level
  }

  // Reset the nodeFull array for tree insertion routine
  for (n = 0; n < levels; n++) {
    nodeFull[n] = 0;
  }

  // Set fillState for each level as that level's capacity, reset fillStates for tree insertion routine
  for (n = 0; n < levels; n++) {
    capacity[n] = fillState[n];
    fillState[n] = 0;
  }

  // Tree insertion routine
  curlvl = levels-1;
  for (m = 0; m < numKeys; m++) {
    if (curlvl == levels-1) { // leaf level
      tree[curlvl][fillState[curlvl]] = keys[m];
      if(++fillState[curlvl]%(fanout[curlvl]-1) == 0) { // move to the block above b/c just filled a leaf
        curlvl--;
      }
    } else { // each time we hit a non-leaf level, we only add one key then go back down to leaf
      int nodeFilled = nodeFull[curlvl]%(fanout[curlvl]-1);
      while(nodeFull[curlvl] > 0 && nodeFilled == 0 && curlvl > 0) { // move up a level b/c just filled a node at this lvl
        nodeFull[curlvl] = 0;
        curlvl--;
        nodeFilled = nodeFull[curlvl]%(fanout[curlvl]-1);
      }
      tree[curlvl][fillState[curlvl]] = keys[m];
      fillState[curlvl]++;
      nodeFull[curlvl]++;
      curlvl = levels-1; // go back to filling leaves
    } // after we add a key to a level above the leaf level, the next key(s) go towards filling another leaf node
  }

  // Print the tree
  int r;
  for(q = 0; q < levels; q++) {
    fprintf(stderr, "Level: %d, Capacity: %d\n", q, capacity[q]);
    for(r = 0; r < fillState[q]; r++) {
      fprintf(stderr, "%d ", tree[q][r]);
    }
    if (q == 0 && fillState[q] < capacity[q]) {
      int st;
      st = fillState[q];
      for (s = st; s < capacity[q]; s++) {
        tree[q][s] = INT_MAX;
        fillState[q]++;
        fprintf(stderr, "%d ", tree[q][s]);
      }
    } else if (fillState[q] < capacity[q]) {
      int start;
      start = fillState[q];
      for (s = start; s < capacity[q]; s++) {
          tree[q][s] = INT_MAX;
          fillState[q]++;
          fprintf(stderr, "%d ", tree[q][s]);
      }
    }
    fprintf(stderr, "\n\n");
  }

  regular_probe(size, levels, numProbes, probe, fanout, tree);

  /* if we need to use hard-coded version, uncomment this
  int* root = tree[0];
  __m128i lvl_0_A = _mm_load_si128((__m128i *)&root[0]);
  __m128i lvl_0_B = _mm_load_si128((__m128i *)&root[4]);
  for (i=0; i<size/4; i++) {
    //9-5-9 tree only
    hard_coded_probe(&probe[i*4], lvl_0_A, lvl_0_B, tree);
  }
  */





  free(capacity);
  free(probes);
  free(keys);
  // Free the tree
  for (l = 0; l < levels; l++) {
    free(tree[l]);
  }
  free(tree);
  free(fillState);
  free(nodeFull);
  free(probe);
  return 0;
}

void regular_probe(int size, int levels, int numProbes, int* probe, int* fanout, int** tree) {
  int i, j, count;
    for (i=0; i<numProbes; i++) {
    printf("%d\n", probe[i]);
  }

  probe_funct[4] = probe_fanout_five;
  probe_funct[8] = probe_fanout_nine;
  probe_funct[16] = probe_fanout_seventeen;

  int ranges[4];
  //register
  __m128i keys_val[4];
  fprintf(stderr, "result: ");
  for (i=0; i<size/4; i++) {
      //why u no have memset?
    for (count=0; count<4; count++) {
        ranges[count] = 0;
    }
    //memset(ranges, 0, sizeof(ranges));
    __m128i key = _mm_load_si128((__m128i*) &probe[i*4]);
    keys_val[0] = _mm_shuffle_epi32(key, _MM_SHUFFLE(0, 0, 0 ,0));
    keys_val[1] = _mm_shuffle_epi32(key, _MM_SHUFFLE(1, 1, 1 ,1));
    keys_val[2] = _mm_shuffle_epi32(key, _MM_SHUFFLE(2, 2, 2 ,2));
    keys_val[3] = _mm_shuffle_epi32(key, _MM_SHUFFLE(3, 3, 3 ,3));

    for (j=0; j<levels; j++) {
      for (count=0; count<4; count++) {
        ranges[count] = probe_funct[fanout[j]-1](ranges[count], tree[j], keys_val[count]);
      }
    }
    for (j=0; j<4; j++) {
      //appended some ints to end of probes such that the length of probes array is 16x
      //we need to disregard the results corresponding to those ints at the end
      if (i == size/4-1 &&(numProbes%4 != 0) && j>=(numProbes % 4)) {
          break;
      }
      fprintf(stderr, "%d ", ranges[j]);
    }
  }
  fprintf(stderr, "\n");
}

void hard_coded_probe(
  int* probe,
  __m128i lvl_0_A,
  __m128i lvl_0_B,
  int** tree) {
  int* level_1 = tree[1];
  int* level_2 = tree[2];

  int i;
  for (i=0; i<4; i++) {
    printf("probe %d\n", probe[i]);
  }

    __m128i k = _mm_load_si128((__m128i*) &probe[0]);
  register __m128i k1 = _mm_shuffle_epi32(k, _MM_SHUFFLE(0, 0,0 ,0));
  register __m128i k2 = _mm_shuffle_epi32(k, _MM_SHUFFLE(1, 1,1 ,1));
  register __m128i k3 = _mm_shuffle_epi32(k, _MM_SHUFFLE(2, 2,2 ,2));
  register __m128i k4 = _mm_shuffle_epi32(k, _MM_SHUFFLE(3, 3,3 ,3));

  //=================offset for root level: fanout=9===========================
  __m128i cmp_0_A = _mm_cmplt_epi32(lvl_0_A, k1);
   __m128i cmp_0_B = _mm_cmplt_epi32(lvl_0_B, k1);
   __m128i cmp_0 = _mm_packs_epi32(cmp_0_A, cmp_0_B);
  cmp_0 = _mm_packs_epi16(cmp_0, _mm_setzero_si128());
  int msk_0 = _mm_movemask_ps((__m128)cmp_0_B);
  //printf("get mask B %04x\n", msk_0);
  msk_0 = _mm_movemask_ps((__m128)cmp_0_A);
  //printf("get mask A %04x\n", msk_0);
  msk_0 = _mm_movemask_epi8(cmp_0);
  //printf("get mask %04x\n", msk_0);
  int r_0_k1 = ((ffs(msk_0^0x1FFFF)-1)+9)%9;
  //printf("r_0_k1: %d\n", r_0_k1);

  cmp_0_A = _mm_cmplt_epi32(lvl_0_A, k2);
  cmp_0_B = _mm_cmplt_epi32(lvl_0_B, k2);
  cmp_0 = _mm_packs_epi32(cmp_0_A, cmp_0_B);
  cmp_0 = _mm_packs_epi16(cmp_0, _mm_setzero_si128());
  msk_0 = _mm_movemask_ps((__m128)cmp_0_B);
  //printf("get mask B %04x\n", msk_0);
  msk_0 = _mm_movemask_ps((__m128)cmp_0_A);
  //printf("get mask A %04x\n", msk_0);
  msk_0 = _mm_movemask_epi8(cmp_0);
  //printf("get mask %04x\n", msk_0);
  int r_0_k2 = ((ffs(msk_0^0x1FFFF)-1)+9)%9;
  //printf("r_0_k2: %d\n", r_0_k2);

  cmp_0_A = _mm_cmplt_epi32(lvl_0_A, k3);
  cmp_0_B = _mm_cmplt_epi32(lvl_0_B, k3);
  cmp_0 = _mm_packs_epi32(cmp_0_A, cmp_0_B);
  cmp_0 = _mm_packs_epi16(cmp_0, _mm_setzero_si128());
  msk_0 = _mm_movemask_ps((__m128)cmp_0_B);
  //printf("get mask B %04x\n", msk_0);
  msk_0 = _mm_movemask_ps((__m128)cmp_0_A);
  //printf("get mask A %04x\n", msk_0);
  msk_0 = _mm_movemask_epi8(cmp_0);
  //printf("get mask %04x\n", msk_0);
  int r_0_k3 = ((ffs(msk_0^0x1FFFF)-1)+9)%9;
  //printf("r_0_k3: %d\n", r_0_k3);

  cmp_0_A = _mm_cmplt_epi32(lvl_0_A, k4);
  cmp_0_B = _mm_cmplt_epi32(lvl_0_B, k4);
  cmp_0 = _mm_packs_epi32(cmp_0_A, cmp_0_B);
  cmp_0 = _mm_packs_epi16(cmp_0, _mm_setzero_si128());
  msk_0 = _mm_movemask_ps((__m128)cmp_0_B);
  //printf("get mask B %04x\n", msk_0);
  msk_0 = _mm_movemask_ps((__m128)cmp_0_A);
  //printf("get mask A %04x\n", msk_0);
  msk_0 = _mm_movemask_epi8(cmp_0);
  //printf("get mask %04x\n", msk_0);
  int r_0_k4 = ((ffs(msk_0^0x1FFFF)-1)+9)%9;
  //printf("r_0_k4: %d\n", r_0_k4);

  //===============offset for first level: fanout=5============================
  __m128i lvl_1 = _mm_load_si128((__m128i *)&level_1[r_0_k1 << 2]);
  //printf("get start of lvl1 %d\n", *((unsigned int *)&lvl_1));
  __m128i cmp_1 = _mm_cmplt_epi32(lvl_1, k1);
  //printf("get cmp_1 %04x\n", *((unsigned int *)&cmp_1));
  int msk_1 = _mm_movemask_ps((__m128)cmp_1);
  //printf("mask %04x\n", msk_1);
  //printf("mask ffs %d\n", ffs(msk_1));
  int r_1_k1 = ffs(msk_1 ^ 0x1FF);
  r_1_k1 = (ffs(msk_1 ^ 0x1FF)-1+5)%5;
  //printf("r_1 %d\n", r_1_k1);
  r_1_k1 += (r_0_k1)*5; // cur pos in node + (x-th child in front)* (cur lvl fanout).
  //printf("after add r_1 %d\n", r_1_k1);

  lvl_1 = _mm_load_si128((__m128i *)&level_1[r_0_k2 << 2]);
  //printf("get start of lvl1 %d\n", *((unsigned int *)&lvl_1));
  cmp_1 = _mm_cmplt_epi32(lvl_1, k2);
  //printf("get cmp_1 %04x\n", *((unsigned int *)&cmp_1));
  msk_1 = _mm_movemask_ps((__m128)cmp_1);
  //printf("mask %04x\n", msk_1);
  //printf("mask ffs %d\n", ffs(msk_1));
  int r_1_k2 = ffs(msk_1 ^ 0x1FF);
  r_1_k2 = (ffs(msk_1 ^ 0x1FF)-1+5)%5;
  //printf("r_1_k2 %d\n", r_1_k2);
  r_1_k2 += (r_0_k2)*5; // cur pos in node + (x-th child in front)* (cur lvl fanout).
  //printf("after add r_1_k2 %d\n", r_1_k2);

  lvl_1 = _mm_load_si128((__m128i *)&level_1[r_0_k3 << 2]);
  //printf("get start of lvl1 %d\n", *((unsigned int *)&lvl_1));
  cmp_1 = _mm_cmplt_epi32(lvl_1, k3);
  //printf("get cmp_1 %04x\n", *((unsigned int *)&cmp_1));
  msk_1 = _mm_movemask_ps((__m128)cmp_1);
  //printf("mask %04x\n", msk_1);
  //printf("mask ffs %d\n", ffs(msk_1));
  int r_1_k3 = ffs(msk_1 ^ 0x1FF);
  r_1_k3 = (ffs(msk_1 ^ 0x1FF)-1+5)%5;
  //printf("r_1_k3 %d\n", r_1_k3);
  r_1_k3 += (r_0_k3)*5; // cur pos in node + (x-th child in front)* (cur lvl fanout).
  //printf("after add r_1_k3 %d\n", r_1_k3);

  lvl_1 = _mm_load_si128((__m128i *)&level_1[r_0_k4 << 2]);
  //printf("get start of lvl1 %d\n", *((unsigned int *)&lvl_1));
  cmp_1 = _mm_cmplt_epi32(lvl_1, k4);
  //printf("get cmp_1 %04x\n", *((unsigned int *)&cmp_1));
  msk_1 = _mm_movemask_ps((__m128)cmp_1);
  //printf("mask %04x\n", msk_1);
  //printf("mask ffs %d\n", ffs(msk_1));
  int r_1_k4 = ffs(msk_1 ^ 0x1FF);
  r_1_k4 = (ffs(msk_1 ^ 0x1FF)-1+5)%5;
  //printf("r_1_k4 %d\n", r_1_k4);
  r_1_k4 += (r_0_k4)*5; // cur pos in node + (x-th child in front)* (cur lvl fanout).
  //printf("after add r_1_k3 %d\n", r_1_k4);

  //===============offset for second level: fanout=9============================
  __m128i lvl_2_A = _mm_load_si128((__m128i *)&level_2[r_1_k1 << 3]);
  __m128i lvl_2_B = _mm_load_si128((__m128i *)&level_2[(r_1_k1 << 3)+4]);
    __m128i cmp_2_A = _mm_cmplt_epi32(lvl_2_A, k1);
  __m128i cmp_2_B = _mm_cmplt_epi32(lvl_2_B, k1);
  __m128i cmp_2 = _mm_packs_epi32(cmp_2_A, cmp_2_B);
  cmp_2 = _mm_packs_epi16(cmp_2, _mm_setzero_si128());
  int msk_2 = _mm_movemask_epi8(cmp_2);
  //printf("mask %04x\n", msk_2);
  int r_2_k1 = (ffs(msk_2^0x1FFFF)-1+9)%9;
  //printf("r_2 %d\n", r_2_k1);
  r_2_k1 += (r_1_k1)*9; //cur pos in node + (x-th child in front) * (cur lvl fanout)
  //printf("r_2 after add %d\n", r_2_k1);

  lvl_2_A = _mm_load_si128((__m128i *)&level_2[r_1_k2 << 3]);
  lvl_2_B = _mm_load_si128((__m128i *)&level_2[(r_1_k2 << 3)+4]);
  cmp_2_A = _mm_cmplt_epi32(lvl_2_A, k2);
  cmp_2_B = _mm_cmplt_epi32(lvl_2_B, k2);
  cmp_2 = _mm_packs_epi32(cmp_2_A, cmp_2_B);
  cmp_2 = _mm_packs_epi16(cmp_2, _mm_setzero_si128());
  msk_2 = _mm_movemask_epi8(cmp_2);
  //printf("mask %04x\n", msk_2);
  int r_2_k2 = (ffs(msk_2^0x1FFFF)-1+9)%9;
  //printf("r_2_k2 %d\n", r_2_k2);
  r_2_k2 += (r_1_k2)*9; //cur pos in node + (x-th child in front) * (cur lvl fanout)
  //printf("r_2_k2 after add %d\n", r_2_k2);

  lvl_2_A = _mm_load_si128((__m128i *)&level_2[r_1_k3 << 3]);
  lvl_2_B = _mm_load_si128((__m128i *)&level_2[(r_1_k3 << 3)+4]);
  cmp_2_A = _mm_cmplt_epi32(lvl_2_A, k3);
  cmp_2_B = _mm_cmplt_epi32(lvl_2_B, k3);
  cmp_2 = _mm_packs_epi32(cmp_2_A, cmp_2_B);
  cmp_2 = _mm_packs_epi16(cmp_2, _mm_setzero_si128());
  msk_2 = _mm_movemask_epi8(cmp_2);
  //printf("mask %04x\n", msk_2);
  int r_2_k3 = (ffs(msk_2^0x1FFFF)-1+9)%9;
  //printf("r_2_k2 %d\n", r_2_k3);
  r_2_k3 += (r_1_k3)*9; //cur pos in node + (x-th child in front) * (cur lvl fanout)
  //printf("r_2_k2 after add %d\n", r_2_k3);

  lvl_2_A = _mm_load_si128((__m128i *)&level_2[r_1_k4 << 3]);
  lvl_2_B = _mm_load_si128((__m128i *)&level_2[(r_1_k4 << 3)+4]);
  cmp_2_A = _mm_cmplt_epi32(lvl_2_A, k4);
  cmp_2_B = _mm_cmplt_epi32(lvl_2_B, k4);
  cmp_2 = _mm_packs_epi32(cmp_2_A, cmp_2_B);
  cmp_2 = _mm_packs_epi16(cmp_2, _mm_setzero_si128());
  msk_2 = _mm_movemask_epi8(cmp_2);
  //printf("mask %04x\n", msk_2);
  int r_2_k4 = (ffs(msk_2^0x1FFFF)-1+9)%9;
  //printf("r_2_k2 %d\n", r_2_k4);
  r_2_k4 += (r_1_k4)*9; //cur pos in node + (x-th child in front) * (cur lvl fanout)
  //printf("r_2_k2 after add %d\n", r_2_k4);

  printf("%d %d %d %d\n", r_2_k1, r_2_k2, r_2_k3, r_2_k4);
}

int indexOf(int* array, int element, int len) {
  int i;
  for (i=0; i<len; i++) {
    if (array[i] == element) {
      return i;
    }
  }
  return -1;
}

int probe_fanout_five(int prev_offset, int* cur_level, __m128i k1) {
    //===============offset for first level: fanout=5============================
  //prev_offset << (log(cur_fanout-1)) == starting element in the cur_level
  __m128i lvl_1 = _mm_load_si128((__m128i *)&cur_level[prev_offset << 2]);
  __m128i cmp_1 = _mm_cmplt_epi32(lvl_1, k1);
  //grab msb (leftmost) bit of 32bit. 4 32bits in cmp_1.
  int msk_1 = _mm_movemask_ps((__m128)cmp_1);
  int r_1_k1 = ffs(msk_1 ^ 0x1FF);
  r_1_k1 = (ffs(msk_1 ^ 0x1FF)-1+5)%5;
  r_1_k1 += (prev_offset)*5; // cur pos in node + (x-th child in front)* (cur lvl fanout).
  return r_1_k1;
}

int probe_fanout_nine(int prev_offset, int* cur_level, __m128i k1) {
  //prev_offset << log(cur_fanout-1) == starting element in the cur_level
  __m128i lvl_2_A = _mm_load_si128((__m128i *)&cur_level[prev_offset << 3]);
  __m128i lvl_2_B = _mm_load_si128((__m128i *)&cur_level[(prev_offset << 3)+4]);
    __m128i cmp_2_A = _mm_cmplt_epi32(lvl_2_A, k1);
  __m128i cmp_2_B = _mm_cmplt_epi32(lvl_2_B, k1);
  //packs 32bits into 16bits: 32bit -1/0 becomes 16bit -1/0
  __m128i cmp_2 = _mm_packs_epi32(cmp_2_A, cmp_2_B);
  //packs 16bits into 8bits: 16bit -1/0 becomes 8bit -1/0
  //fills up LSB with first arg. Then second arg
  cmp_2 = _mm_packs_epi16(cmp_2, _mm_setzero_si128());
  //grab msb (leftmost) bit of 8 bit. 16 8bits in cmp_1.
  //Only the last 8 bits are useful
  int msk_2 = _mm_movemask_epi8(cmp_2);
  int r_2_k1 = (ffs(msk_2^0x1FFFF)-1+9)%9;
  r_2_k1 += (prev_offset)*9; //cur pos in node + (x-th child in front) * (cur lvl fanout)
  return r_2_k1;
}

int probe_fanout_seventeen(int prev_offset, int* cur_level, __m128i k1) {
  //prev_offset << log(cur_fanout-1) == starting element in the cur_level
  __m128i lvl_2_A = _mm_load_si128((__m128i *)&cur_level[prev_offset << 4]);
  __m128i lvl_2_B = _mm_load_si128((__m128i *)&cur_level[(prev_offset << 4)+4]);
    __m128i cmp_2_A = _mm_cmplt_epi32(lvl_2_A, k1);
  __m128i cmp_2_B = _mm_cmplt_epi32(lvl_2_B, k1);
  __m128i cmp_2 = _mm_packs_epi32(cmp_2_A, cmp_2_B);

  lvl_2_A = _mm_load_si128((__m128i *)&cur_level[(prev_offset << 4)+8]);
  lvl_2_B = _mm_load_si128((__m128i *)&cur_level[(prev_offset << 4)+12]);
  cmp_2_A = _mm_cmplt_epi32(lvl_2_A, k1);
  cmp_2_B = _mm_cmplt_epi32(lvl_2_B, k1);
  __m128i cmp_2_1 = _mm_packs_epi32(cmp_2_A, cmp_2_B);

  cmp_2 = _mm_packs_epi16(cmp_2, cmp_2_1);
  //grab msb (leftmost) bit of 8 bit. 16 8bits in cmp_1.
  //Only the last 8 bits are useful
  int msk_2 = _mm_movemask_epi8(cmp_2);
  int r_2_k1 = (ffs(msk_2^0x1FFFF)-1+17)%17;
  r_2_k1 += (prev_offset)*17; //cur pos in node + (x-th child in front) * (cur lvl fanout)
  return r_2_k1;
}
