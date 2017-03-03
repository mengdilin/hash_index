import sys
import re
import numpy as np
import pandas as pd
from scipy import stats, integrate
import matplotlib.pyplot as plt

infile = sys.argv[1]

line_start = "primary bucket count: "
merge_keyword = " merged"
overflow_keyword = " -> "
key_distribution_word = "bucket: "
with open(infile) as inf:
  content = inf.readlines()

merge_count = 0
key_distribution = [];
entry_distribution = []
overflow_distribution = []
pages = 0
overflows = 0
empty_page = 0
max_overflow_chain = 0
for line in content:
  line = line.strip('\n')
  if line.startswith(line_start):
    #print line
    line = line[len(line_start):]
    if merge_keyword in line:
      count = sum(1 for _ in re.finditer(r'\b%s\b' % re.escape(merge_keyword), line))
      merge_count = merge_count + count

      #print line
      primary_bucket = line.split(" ")[0] + " "
      line = primary_bucket+(" ".join([x for x in line.split(merge_keyword) if x.startswith("-> ")]))
      #print line
      #print line
    if overflow_keyword in line:
      buckets = line.split(overflow_keyword)
      buckets = [int(x) for x in buckets if len(x)!=0]
      pages = pages + len(buckets)
      #print "buckets: ", buckets
      #print "sum: ", sum(buckets)
      entry_distribution.append(sum(buckets))
      overflow_distribution.append(sum(buckets[1:]))
      overflows = overflows + len(buckets) - 1
      max_overflow_chain = max(len(buckets) - 1, max_overflow_chain)
    else:
      #print "no overflow: ", int(line)
      entry_distribution.append(int(line))
      pages = pages + 1
  elif line.startswith(key_distribution_word):
    key_distribution.append(int(line[len(key_distribution_word):]))

print all(isinstance(x, int) for x in entry_distribution)
print "pages: " , pages
print "overflows: ", overflows
print "merge count: ", merge_count
print "max overflow chain: ", max_overflow_chain
print "key num min: ", min(key_distribution)
print "key num max: ", max(key_distribution)
#plt.hist(key_distribution, range=[0, 400], alpha=0.5, bins=50, label="key dist")
#plt.hist(entry_distribution, range=[0, 400], alpha=0.5, bins=50, label="merge dist")
#plt.legend(loc='upper right')

print len(entry_distribution)
print len(key_distribution)

key_distribution_plot = [0]*max(key_distribution)
entry_distribution_plot = [0]*max(entry_distribution)
for key in key_distribution:
  key_distribution_plot[key-1] += 1
for key in entry_distribution:
  entry_distribution_plot[key-1] += 1

print entry_distribution_plot[230:260]
#hmean = np.mean(key_distribution)
#hstd = np.std(key_distribution)
#pdf = stats.norm.pdf(key_distribution, hmean, hstd)
#plt.plot(key_distribution, pdf) # including h here is crucial
#kde = stats.gaussian_kde( key_distribution )
#dist_space = np.linspace( min(key_distribution), max(key_distribution), 100 )
# plot the results
before_merge, = plt.plot(range(0,len(key_distribution_plot)), key_distribution_plot, label="before merge")
after_merge, = plt.plot(range(0,len(entry_distribution_plot)), entry_distribution_plot, label="after merge")
plt.legend(handles=[after_merge, before_merge])
plt.show()
