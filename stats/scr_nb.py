import numpy as np
import matplotlib.pyplot as plt

mass = []
nb1 = []
nb2 = []
nb3 = []
# diam = []
# time = []
# poly = []

f = open("tas_sable_stats_jqa_1701414.txt")
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

plt.plot(mass,nb1,label='nb1')
plt.plot(mass,nb2,label='nb2')
plt.plot(mass,nb3,label='nb3')



# plt.plot(mass,diam)
plt.ylabel('cardinal')
plt.xlabel('mass')
plt.legend()
plt.show()

# print(np.polyfit(mass,time,2))
