#include <time.h>
#include <stdio.h> 
#include <stdarg.h> /* to accept variable arguments */
#include <string.h>
#include <stdlib.h> /* for atoi */
#include <stdint.h>
#include <assert.h>
#include <limits.h> /* for max int */
#include <math.h> /* for ceiling function */

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

/* Binary search routine, will return the range of the probe within a level (as though this is the only 
 * level in the entire tree); additional calcs done outside of this routine to get the actual range */
Result binarysearch(int *tree, int p, int start, int fanout) {
	int i; int l; int r; Result res;
	l = start; r = start+fanout-1;
	while (l <= r) {
		i = (l+r)/2;
		if (tree[i] == p) { // Equality gets the same range as lesser than
			// Check for duplicate INT_MAX keys in front of this one, follow the first pointer
			while (i-1 >= 0 && tree[i-1] == INT_MAX) {
				i--;
			}
			res = (Result){i};
			return res;
		} else {
		   	if (p < tree[i]) {
				r = i-1;
			} else {
				l = i+1;
			}
		}
	}
	// Element not found
	if (p <= tree[i]) {
		while (i-1 >= 0 && tree[i-1] == INT_MAX) {
			i--;
		}
		res = (Result){i};
	} else {
		while (i-1 >= 0 && tree[i-1] == INT_MAX) {
			i--;
		}
		res = (Result){(i+1)};
	}
	return res;
}

/* Each probe goes through this rountine, params are probe, capacity of each level, max number of levels, 
 * fanout of each level, this routine returns the correct range of the probe (n+1 ranges given n keys in 
 * the tree) */ 
int binary_search(int **tree, int p, int *cap, int maxlvl, int *fanout) {	
	int n; int offset[maxlvl]; int ranges[maxlvl];
	for (n = 0; n < maxlvl; n++) {
		offset[n] = 0;
	}
	for (n = 0; n < maxlvl; n++) {
		Result r;
		if (n == 0) { // Root
			r = binarysearch(tree[n], p, 0, fanout[n]-1);
			if (n == maxlvl-1) {
				return r.range; // For single-level tree, range calculation straightforward
			} else { // Determine offsets for the next level
				offset[0] = r.range;
				offset[1] = offset[0]*(fanout[1]-1);
			}
		} else {
			/* Use the level's offset (calculated by results of binary search in the parent level) 
			 * to figure out which node to conduct binary search on */
			int start;
			start = offset[n];
			r = binarysearch(tree[n], p, start, fanout[n]-1);
			ranges[n] = r.range;
			if (n == maxlvl-1) { // We're done, have to find the range
				// Use the offset of the nodes in preceding levels to figure out how many to skip
				int keysInFront;
			    int lev;
				if (r.range < fanout[n]-1) {
					return r.range;
				} else {
					keysInFront = 0;
					for (lev = 0; lev < maxlvl; lev++) {
						if (lev == 0){
							keysInFront += offset[0];
						} else {
							keysInFront += ranges[lev];
						}
					}
					return keysInFront;
				}
			} else {
				/* Based on results of this level's binary search, figure out what offset to use
				 * for the next level */
				int nodesInFront;
				nodesInFront = start/(fanout[n]-1);
				int pointersInFront;
				pointersInFront = (r.range - start);
				offset[n+1] = fanout[n]*nodesInFront*(fanout[n+1]-1) + pointersInFront*(fanout[n+1]-1);
			}
		}	
	}
	return p;
}

int main(int argc, char *argv[]) {
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
		if (fanout[i] < 2 || fanout[i] > 17) {
			die("Fanout must be between 2 and 17 inclusive.");
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
	int32_t *probes = generate_sorted_unique(numProbes, gen);
    for (j = 0 ; j != numProbes; ++j) {
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

	int v;
	int range;
	range = 0;

	// Must print probe results to stdout as part of specs
	clock_t start = clock(), diff;
	for (v = 0; v < numProbes; v++) {
		range = binary_search(tree, probes[v], capacity, levels, fanout);
		printf("Probe %d range: %d\n", probes[v], range);		
	}
	diff = clock() - start;
	int msec = diff*1000/CLOCKS_PER_SEC;
	fprintf(stderr, "Time taken: %d seconds (%d milliseconds)\n", msec/1000, msec%1000);
	
	// Uncomment for more probe tests (extra ones beyond the specified number of probes)
	/*for (v = 0; v < capacity[0]; v++) {
		range = binary_search(tree, tree[0][v], capacity, levels, fanout);
		printf("Probe %d range: %d\n", tree[0][v], range);	
	}
	range = binary_search(tree, tree[levels-1][0], capacity, levels, fanout);
	for (v = 1; v < levels-1; v++) {
		int w;
		for (w = 0; w < capacity[v]; w++) {
			range = binary_search(tree, tree[v][w], capacity, levels, fanout);
			printf("Probe %d range: %d\n", tree[v][w], range);
		}
	}*/

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
	return 0;
}
