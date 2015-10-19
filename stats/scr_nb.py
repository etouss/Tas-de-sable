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
j = 0

f = open("stats_sand_v2.6TjqaM23742475.txt")
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

pnb3 = [];
pnb2 = [];
pnb1 = [];
pnb0 = [];


for i in range(0,len(mass)):
    pnb3 += [nb3[i]/(nb3[i]+nb2[i]+nb1[i]+nb0[i])]
    pnb2 += [nb2[i]/(nb3[i]+nb2[i]+nb1[i]+nb0[i])]
    pnb1 += [nb1[i]/(nb3[i]+nb2[i]+nb1[i]+nb0[i])]
    pnb0 += [nb0[i]/(nb3[i]+nb2[i]+nb1[i]+nb0[i])]


#print(min(nb3/mass*100));

# p = np.poly1d(np.polyfit(mass,time,2))
# plt.plot(mass,[p(x) + 10**10 for x in mass],label = 'quadratic approach')
plt.plot(mass,pnb0,label='prop nb0')
plt.plot(mass,pnb1,label='prop nb1')
plt.plot(mass,pnb2,label='prop nb2')
plt.plot(mass,pnb3,label='prop nb3')

# print("nb0: "+str(sum(nb0)/sum(mass)*100))
# print("nb1: "+str(sum(nb1)/sum(mass)*100))
# print("nb2: "+str(sum(nb2)/sum(mass)*100))
# print("nb3: "+str(sum(nb3)/sum(mass)*100))

# plt.plot(mass,diam)
plt.ylabel('prop')
plt.xlabel('mass')
plt.legend()
plt.show()

# print(np.polyfit(mass,time,2))
