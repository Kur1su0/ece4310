from __future__ import print_function

import sys


conv = 0
sep = 0
sw = 0
list = []
list = [line.rstrip('\n').split(" ") for line in open('AVG')]
#print (list[0][1])

for i in range(10):  
    conv=conv+(float)(list[i][1].split("sec")[0])
    #print(conv)
i = 0   
print("conv ",conv/10)

for i in range(10):  
    sep=sep+(float)(list[i][3].split("sec")[0])
    #print(sep)
i = 0       
print("sep ",sep/10)

for i in range(10):  
    sw=sw+(float)(list[i][5].split("sec")[0])
    #print(sw)
    
print("sw ",sw/10)
