import numpy as np
import matplotlib.pyplot as plt

mass = []
nb0 = []
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
    nb0 += [int(data[3].split("=")[1])]
    nb1 += [int(data[4].split("=")[1])]
    nb2 += [int(data[5].split("=")[1])]
    nb3 += [int(data[6].split("=")[1])]



    # time += [int(data[1].split("=")[1])]
    # diam += [int(data[2].split("=")[1])*2*10**7]
f.close()

print(min(nb3/mass*100));    

# p = np.poly1d(np.polyfit(mass,time,2))
# plt.plot(mass,[p(x) + 10**10 for x in mass],label = 'quadratic approach')
# plt.plot(mass,nb1,label='nb0')
# plt.plot(mass,nb1,label='nb1')
# plt.plot(mass,nb2,label='nb2')
# plt.plot(mass,nb3,label='nb3')

# print("nb0: "+str(sum(nb0)/sum(mass)*100))
# print("nb1: "+str(sum(nb1)/sum(mass)*100))
# print("nb2: "+str(sum(nb2)/sum(mass)*100))
# print("nb3: "+str(sum(nb3)/sum(mass)*100))

# plt.plot(mass,diam)
plt.ylabel('cardinal')
plt.xlabel('mass')
plt.legend()
plt.show()

# print(np.polyfit(mass,time,2))
