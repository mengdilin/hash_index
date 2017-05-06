import os
import sys

path = sys.argv[1]
l = []

for filename in os.listdir(path):
  if filename.endswith("_probe_log"):
    with open(path+filename) as inf:
      content = inf.readlines()
    entry_size = -1;
    avg_probe = -1
    total_probe = -1
    for line in content:
      if line.startswith("size of entry: "):
        entry_size = int(line[len("size of entry: "):])
      if line.startswith("average per probe (ns): "):
        avg_probe = int(line[len("average per probe (ns): "):])
      if line.startswith("index probe time (ns): "):
        total_probe = int(line[len("index probe time (ns): "):])
    tmp = filename.split('_')
    page_size = tmp[0]
    load = tmp[1]
    l.append(page_size + " | " + load + " | " + str(entry_size) + " | " + str(avg_probe) + " | " + str(total_probe))
  l.sort()
print "\n".join(l)
