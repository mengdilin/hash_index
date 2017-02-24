import sys
import matplotlib.pyplot as plt
import re

infile = sys.argv[1]

line_start = "primary bucket count: "
merge_keyword = " merged"
overflow_keyword = " -> "
with open(infile) as inf:
  content = inf.readlines()

merge_count = 0
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
      line = line.replace(merge_keyword, "", 1)
    if overflow_keyword in line:
      buckets = line.split(overflow_keyword)
      buckets = [int(x) for x in buckets]
      pages = pages + len(buckets)
      #print "buckets: ", buckets
      #print "sum: ", sum(buckets)
      entry_distribution.append(sum(buckets))
      overflow_distribution.append(sum(buckets[1:]))
      '''
      tmp = sum(buckets[1:])
      if tmp >= 255:
        print line
      '''
      overflows = overflows + len(buckets) - 1
      max_overflow_chain = max(len(buckets) - 1, max_overflow_chain)
    else:
      #print "no overflow: ", int(line)
      entry_distribution.append(int(line))
      pages = pages + 1
print all(isinstance(x, int) for x in entry_distribution)
print "pages: " , pages
print "overflows: ", overflows
print "merge count: ", merge_count
print "max overflow chain: ", max_overflow_chain
plt.hist(entry_distribution, range=[0, 600], alpha=0.5, bins=50, label="key dist")
plt.legend(loc='upper right')
#plt.gca().set_yscale("log")
plt.show()

