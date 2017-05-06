import os
import sys
import math

def convert_size(size_bytes):
   if size_bytes == 0:
       return "0B"
   size_name = ("B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB")
   i = int(math.floor(math.log(size_bytes, 1024)))
   p = math.pow(1024, i)
   s = round(size_bytes / p, 2)
   return "%s %s" % (s, size_name[i])

path = sys.argv[1]
l = []
for filename in os.listdir(path):
  if filename.endswith("_index"):
    size = os.path.getsize(path+filename)
    size = convert_size(size)
    tmp = filename.split('_')
    page_size = tmp[0]
    load = tmp[1]
    l.append(page_size + " | " + load + " | " + str(size))
  l.sort()
print "\n".join(l)

