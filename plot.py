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
capacities = [80, 90, 100];
for size in page_size:
  probe_time[size] = []
  build_time[size] = []
for file in os.listdir("./data_perf"):
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
    probe_time[size].append((capacity,int(content[-1][len(probe_flag):])/1000000000))
    build_time[size].append((capacity,(int(content[-2][len(build_flag):])/1000000000)))
    print content[-2][len(build_flag):]
#fig, ax = plt.subplots()
for key in probe_time:
  print "page size: " , key
  probe_time[key] = sorted(probe_time[key], key=lambda k: k[1])
  build_time[key] = sorted(build_time[key], key=lambda k: k[1])

  print "probe time: " ,  probe_time[key]
  print "build time: " ,  build_time[key]
  #ax.scatter(probe_time[key][1], probe_time[key][0], color='r', label=str(key))
#plt.show()


#ax.scatter(range(0,len(entry_distribution_plot)), entry_distribution_plot, color='b')


#plt.hist(entry_distribution, range=[0, 600], alpha=0.5, bins=50, label="key dist")
#plt.legend(loc='upper right')
#plt.gca().set_yscale("log")
#plt.show()

