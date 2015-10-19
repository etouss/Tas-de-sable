import numpy as np
import matplotlib.pyplot as plt
import math

mass = []
diam = []
time = []
poly = []

f = open("stats_sand_v2.6TjqaM23742475.txt")
for ligne in f.readlines():
    data = ligne.split(" ")
    mass += [int(data[0].split("=")[1])]
    # time += [int(data[1].split("=")[1])]
    diam += [int(data[2].split("=")[1])*2*10**7]
f.close()

p = np.poly1d(np.polyfit(mass,[p*p for p in diam],1))
plt.plot(mass,[p(x) for x in mass],label = 'linear approach')

plt.plot(mass,[p*p + 10**20 for p in diam],label='mass /  diam*diam')
plt.ylabel('diam * diam')
plt.xlabel('mass')
plt.legend()
plt.show()

print(np.polyfit(mass,np.log(diam),1))
