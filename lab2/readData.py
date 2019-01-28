from __future__ import print_function
import os
import sys
import matplotlib as plt

true_e_path = "true_e"        
possible_e_path = "222"

true_e = []
temp = []
possible_e = []
T_val = 0
#True Posotive
TP = []
#False Posotive
FP = []
#read standard e
with open(true_e_path,'r') as f:
    for line in f.readlines():
        true_e.append(line.strip("\n").split(" ")[1:3])


for i in range(len(true_e)):
    for j in range(2):
        true_e[i][j] = int(true_e[i][j])

#for i in range (len(true_e)):
#    print(true_e[i][0], true_e[i][1]);
print("\n")
print("\n")
#read my e
with open(possible_e_path,'r') as f:
    for line in f.readlines():
        temp.append(line.strip("\n"))

T_val = temp[0].split(" ")[-1]
T_val = int(T_val)

temp = temp[1:]
for i in range (len(temp)):
    possible_e.append(temp[i].split(" ")[1:3]);

for i in range(len(possible_e)):
    for j in range(2):
        possible_e[i][j] = int(possible_e[i][j])

print("T:",T_val)
print(possible_e);
print(possible_e[0])

matched = 0
count = 0
i,j=0,0
count_i = i
true_lens = len(true_e)
possible_lens = len(possible_e)
while count_i < true_lens - 1:
    count_j = j;
    while count_j < possible_lens - 1:
        #check whether pixel have the same y-axis.
        if((possible_e[j+1][1] is possible_e[j][1]) or 
          (possible_e[j+1][1] is not possible_e[j][1] and possible_e[j+1][1] is not possible_e[j][1])  ):
            #error check +-15 +-9
            if (true_e[i][1]+15 >= possible_e[j][1] and true_e[i][1]-15 <= possible_e[j][1] and
                true_e[i][0]+9  >= possible_e[j][0] and true_e[i][1]-9  <= possible_e[j][1]     ):
                    matched+=1;
                    print("matched:",true_e[i]," - ",possible_e[j])
                    j+=1
                    i+=1
                    break
            else:
                print("not matched", true_e[i]," - ",possible_e[j])
                j+=1
                i = i - 1
                break
        else:
            print("diff y")
            j+=1
            i+=1
            break
 
#[][0][0]:X
#  [0][1]:Y
#print(true_e[0][0])
#print(true_e[0][1])
print("count:",matched)
