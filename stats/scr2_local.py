import numpy as np
import matplotlib.pyplot as plt

mass = []
diam = []
time = []
poly = []

f = open("tas_sable_stats_jqa_1701414.txt")
for ligne in f.readlines():
    data = ligne.split(" ")
    mass += [int(data[0].split("=")[1])]
    # time += [int(data[1].split("=")[1])]
    diam += [int(data[2].split("=")[1])*2*10**7]
f.close()

p = np.poly1d(np.polyfit(mass,[p*p for p in diam],1))
# plt.plot(mass,[p(x) for x in mass],label = 'linear approach')

plt.plot(mass,[np.sqrt(p(mass[x]))-diam[x]  for x in range(0,len(diam))],label='diffenrential')
plt.ylabel('difference')
plt.xlabel('mass')
plt.legend()
plt.show()

print(np.polyfit(mass,np.log(diam),1))
