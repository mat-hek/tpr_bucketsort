import matplotlib.pyplot as plt
from pprint import pprint
import json
import re
import os

nodes_cnt = 4

data = json.load(open('results.json'))
os.makedirs("plots", exist_ok=True)

# pprint(data)

def save_plot(res, label, oy=None):
    oy = label if oy == None else oy
    title = label + f" for array size: {res}"
    plt.title(title)
    plt.xlabel("Number of threads")
    plt.ylabel(oy)
    file_path = "plots/" + title.replace(" ", "_") + ".png"
    plt.savefig(file_path)
    print(f"![{title}]({file_path})")
    plt.clf()

for res, times in data.items():
    is_scaled = re.match(r".*non-scaled", res) is None
    procs = range(1, len(times)+1)
    plt.plot(procs, times)
    save_plot(res, "Times", oy="Time [s]")
    speedup = []
    for i in range(1, len(times)):
        t0 = times[0] if not is_scaled else times[0]*procs[i]
        speedup.append(t0/times[i])
    plt.plot(procs[1:], speedup)
    save_plot(res, "Speedup")
    efficency = []
    for i in range(1, len(times)):
        efficency.append(speedup[i-1]/procs[i])
    plt.plot(procs[1:], efficency)
    save_plot(res, "Efficency")
    serial_fraction = []
    for i in range(1, len(times)):
        sf = (1/speedup[i-1] - 1/procs[i])/(1 - 1/procs[i])
        serial_fraction.append(sf)
    plt.plot(procs[1:], serial_fraction)
    save_plot(res, "Serial fraction")
