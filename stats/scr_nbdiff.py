import numpy as np
import matplotlib.pyplot as plt

mass = []
nb1 = []
nb2 = []
nb3 = []
# diam = []
# time = []
# poly = []

f = open("stats_sand_v2.6TjqaM23742475.txt")
for ligne in f.readlines():
    data = ligne.split(" ")
    mass += [int(data[0].split("=")[1])]
    nb1 += [int(data[3].split("=")[1])]
    nb2 += [int(data[4].split("=")[1])]
    nb3 += [int(data[5].split("=")[1])]



    # time += [int(data[1].split("=")[1])]
    # diam += [int(data[2].split("=")[1])*2*10**7]
f.close()

# p = np.poly1d(np.polyfit(mass,time,2))
# plt.plot(mass,[p(x) + 10**10 for x in mass],label = 'quadratic approach')

plt.plot([mass[x] for x in range(1,len(mass))],[abs(nb1[x]-nb1[x-1]) for x in range(1,len(mass))],label='nb1')
plt.plot([mass[x] for x in range(1,len(mass))],[abs(nb2[x]-nb2[x-1]) for x in range(1,len(mass))],label='nb2')
plt.plot([mass[x] for x in range(1,len(mass))],[abs(nb3[x]-nb3[x-1]) for x in range(1,len(mass))],label='nb3')




# plt.plot(mass,diam)
plt.ylabel('|card(n)-card(n-1)|')
plt.xlabel('mass')
plt.legend()
plt.show()

# print(np.polyfit(mass,time,2))
