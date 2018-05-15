import matplotlib.pyplot as plt
from pprint import pprint
import json
import re
import os

nodes_cnt = 8

buckets_sizes = [10, 100, 1000]

versions = {"bs": "dashed", "bs3": "solid"}

data = {}

for ver in versions.keys():
    for bs in buckets_sizes:
        for k, v in json.load(open(f"results/res_{ver}_{bs}.json")).items():
            if k not in data:
                data[k] = {}
            data[k][(ver, bs)] = v


os.makedirs("plots", exist_ok=True)

# pprint(data)

def save_plot(res, label, oy=None):
    oy = label if oy == None else oy
    title = label + f" for array size: {res}"
    plt.title(title)
    plt.xlabel("Number of threads")
    plt.ylabel(oy)
    plt.legend()
    file_path = "plots/" + title.replace(" ", "_") + ".png"
    plt.savefig(file_path)
    print(f"![{title}]({file_path})")
    plt.clf()

def plot(x, y, version, bucket_size):
    plt.plot(x, y, linestyle=versions[ver], label=f"{ver}, {bs} buckets")

for res, ver_times in data.items():
    is_scaled = re.match(r".*non-scaled", res) is None
    procs = range(1, nodes_cnt+1)
    for (ver, bs), times in ver_times.items():
        plot(procs, times, ver, bs)
    save_plot(res, "Times", oy="Time [s]")
    speedups = {}
    for (ver, bs), times in ver_times.items():
        speedup = speedups[(ver, bs)] = []
        for i in range(1, len(times)):
            t0 = times[0] if not is_scaled else times[0]*procs[i]
            speedup.append(t0/times[i])
        plot(procs[1:], speedup, ver, bs)
    save_plot(res, "Speedup")
    for (ver, bs), times in ver_times.items():
        speedup = speedups[(ver, bs)]
        efficency = []
        for i in range(1, len(times)):
            efficency.append(speedup[i-1]/procs[i])
        plot(procs[1:], efficency, ver, bs)
    save_plot(res, "Efficency")
    for (ver, bs), times in ver_times.items():
        speedup = speedups[(ver, bs)]
        serial_fraction = []
        for i in range(1, len(times)):
            sf = (1/speedup[i-1] - 1/procs[i])/(1 - 1/procs[i])
            serial_fraction.append(sf)
        plot(procs[1:], serial_fraction, ver, bs)
    save_plot(res, "Serial fraction")
