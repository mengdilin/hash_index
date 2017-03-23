import sys

infile = sys.argv[1]
num_entries = int(sys.argv[2])

num = 0;
with open(infile, 'w') as inf:
  while (num < num_entries):
    inf.write(str(num)+" \t "+str(num)+ "\n")
    num = num+1

