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

sat = [nb3[x]/(nb1[x]+nb2[x]+nb3[x]) for x in range(0,len(mass))]
per = [x for x in range(1,len(mass)) if nb3[x-1] > 2*nb3[x]]
# per = [x for x in range(1,len(mass)) if nb2[x-1] > 2*nb2[x] +1]
# per = [x for x in range(1,len(mass)) if nb3[x-1] > 2*nb3[x] +1]

# diff = []
# for x in range(1,len(per)):
#     if  (per[x]-per[x-1]) not in diff:
#         diff += [per[x]-per[x-1]]

# print(per)
# print([sat[x-1] for x in per])
print(sum([sat[x-1] for x in per])/len(per))

# print([s for s in sat if s == 0.3333333333333333])
# plt.plot(mass,nb1,label='nb1')
# # plt.plot(mass,nb2,label='nb2')
# # plt.plot(mass,nb3,label='nb3')
#
#
#
# # plt.plot(mass,diam)
# plt.ylabel('cardinal')
# plt.xlabel('mass')
# plt.legend()
# plt.show()

# print(np.polyfit(mass,time,2))
