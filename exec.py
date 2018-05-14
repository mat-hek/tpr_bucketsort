import sys
from pprint import pprint, pformat
import subprocess
import math
from timeit import timeit
import json

sizes = [int(x) for x in sys.argv[1:]]

cores_cnts = range(1, 4 + 1)

res = {}

def exec_mc(scale):
  print(f"executing, scale: {scale}")
  for s in sizes:
    label = "{} {}".format(s, ("scaled" if scale else "non-scaled"))
    res[label] = []
    for n in cores_cnts:
      ss = s*n if scale else s
      t = timeit(
        lambda: subprocess.call(f"OMP_NUM_THREADS={str(n)} ./bin/bs {str(ss)}", shell=True),
        number=1
        )
      res[label].append(t)
      print(json.dumps(res, indent=2))



exec_mc(scale=False)
exec_mc(scale=True)
