import subprocess
import matplotlib.pyplot as plt
import time

#comparison of python speed vs C (single thread) speed
#outcomment plots and prints in "heat2d.py" and pragmas in "heat-equation.c" before running

N = "100"
M = "100"
S_max = 100
speedups = []

#get speedups
for S in range(S_max):
    #python
    start = time.time()
    if subprocess.call(["python","heat2d.py",N,M,str(S)]): #if return code is non-zero
        raise Exception("System call error occured")
    stop = time.time()
    pyth_lat = stop - start

    #C
    start = time.time()
    if subprocess.call(["./heat-equation",N,M,str(S)]):
        raise Exception("System call error occured")
    stop = time.time()
    c_lat = stop - start

    speedups.append(pyth_lat/c_lat)

#plot
plt.figure(figsize=(10,5))
plt.xlim([0,100])
plt.ylim([0,200])
plt.xlabel("Steps")
plt.ylabel("Speedup in latency")
plt.plot([S for S in range(S_max)],speedups,"g")
plt.grid(True)
plt.show()


print("speedups:",speedups)