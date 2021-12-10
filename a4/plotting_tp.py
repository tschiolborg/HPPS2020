import matplotlib.pyplot as plt
import re
import numpy as np

steps_re = re.compile(r"\(\d+x\d+:(\d+)\)")
threads_re = re.compile(r"with (\S+) threads")
stencil_re = re.compile(r"stencil: (\S+)")
delta_re = re.compile(r"delta: (\S+)")
total_re = re.compile(r"total time:\s+(\S+)")

##plotting weak scaling (throughput speedup)
out = ""
with open('slurm-65563.out') as f:
  out = "".join(f.readlines())

steps = [int(x) for x in steps_re.findall(out)]
threads = [int(x) for x in threads_re.findall(out)]
stencil = [float(x) for x in stencil_re.findall(out)]
delta = [float(x) for x in delta_re.findall(out)]
total = [float(x) for x in total_re.findall(out)]

# Q_i / Q_0
stencil_speedup = [((steps[i]/stencil[i]) / (steps[0]/stencil[0])) for i in range(len(steps))]
delta_speedup = [((steps[i]/delta[i]) / (steps[0]/delta[0])) for i in range(len(steps))]
total_speedup = [((steps[i]/total[i]) / (steps[0]/total[0])) for i in range(len(steps))]

plt.plot(threads, total_speedup, label='Weak')


##plotting weak scaling (runtime speedup)
out = ""
with open('slurm-64708.out') as f:
    out = "".join(f.readlines())

threads = [int(x) for x in threads_re.findall(out)]


stencil = [float(x) for x in stencil_re.findall(out)]
delta = [float(x) for x in delta_re.findall(out)]
total = [float(x) for x in total_re.findall(out)]

stencil_speedup = [stencil[0] / x for x in stencil]
delta_speedup = [delta[0] / x for x in delta]
total_speedup = [total[0] / x for x in total]

plt.plot(threads, total_speedup, label="Strong")

plt.title('Scaling of 1000x1000 grid for 1000 / 10*threads iterations')
plt.xlabel('threads')
plt.ylabel('speedup')
plt.legend()
plt.savefig("test.png")


