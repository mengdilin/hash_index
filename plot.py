import sys, os
import matplotlib.pyplot as plt
import re


probe_flag = "index probe time: "
build_flag = "index build time: "

probe_time = {}
build_time = {}
probe_time_y = []
build_time_y = []
page_size = [64, 128, 256, 512, 1024, 2048, 4096]
capacities = [80, 90, 100, 70, 60];
for size in page_size:
  probe_time[size] = []
  build_time[size] = []
for file in os.listdir("./data_perf"):
  if file.endswith("log_2"):
    print file
    with open("./data_perf/"+file) as inf:
      content = inf.readlines()
      size = 4096
      for item in page_size:
        if file.startswith(str(item)):
          size = item
      capacity = 0
      for item in capacities:
        if str(item) in file:
          capacity = item
      entry_size = 172705571
      probe_time[size].append((capacity,int(content[-2][len("average per probe: "):])))
      build_time[size].append((capacity,float((int(content[-4][len(build_flag):]))/entry_size)))
for key in probe_time:
  print "page size: " , key
  probe_time[key] = sorted(probe_time[key], key=lambda k: k[1])
  build_time[key] = sorted(build_time[key], key=lambda k: k[1])

  print "probe time: " ,  probe_time[key]
  print "build time: " ,  build_time[key]

