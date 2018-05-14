import os
import sys
from pprint import pprint, pformat
import subprocess
import math
from timeit import timeit
import json

sizes = sys.argv[1:]

buckets_cnt = 1000

cores_cnts = range(1, os.cpu_count() + 1)

res = {}

def exec_mc(scale):
  print(f"executing, scale: {scale}")
  for s in sizes:
    label = "{} {}".format(s, ("scaled" if scale else "non-scaled"))
    res[label] = []
    for n in cores_cnts:
      ss = int(float(s))*(n if scale else 1)
      t = timeit(
        lambda: subprocess.call(
            f"OMP_NUM_THREADS={str(n)} ./bin/bs_v3 {str(ss)} {str(buckets_cnt)}",
            shell=True
        ),
        number=1
        )
      res[label].append(t)
      # print(json.dumps(res, indent=2))



exec_mc(scale=False)
exec_mc(scale=True)
print(json.dumps(res, indent=2))
