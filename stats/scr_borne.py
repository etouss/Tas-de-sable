import numpy as np
import matplotlib.pyplot as plt

mass = []
diam = []
time = []
poly = []

f = open("stats_sand_v2.6TjqaM23742475.txt")
for ligne in f.readlines():
    data = ligne.split(" ")
    mass += [int(data[0].split("=")[1])]
    time += [int(data[1].split("=")[1])]
    # diam += [int(data[2].split("=")[1])*2*10**7]
f.close()

# p = np.poly1d(np.polyfit(mass,time,2))
plt.plot([mass[x] for x in range(1,len(mass))],[time[x+1] - time[x] for x in range(0,len(mass)-1)],label = 'differential approch')

# plt.plot(mass,time,label='mass time')
# plt.plot(mass,diam)
plt.ylabel('T[n] - T[n-1]')
plt.xlabel('mass')
plt.legend()
plt.show()

print(np.polyfit(mass,time,2))
