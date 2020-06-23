#Diploma Thesis , June 2020
#George Kalitsios ,THMMY AUTH
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
#Insert complete path to the excel file and index of the worksheet
df = pd.read_excel("All_tags.xlsx", sheet_name=0)
# insert the name of the column as a string in brackets
list1 = list(df['X']) 
list2 = list(df['Y'])
list3 = list(df['Z'])
x=list1[:48]
y=list2[:48]
z=list3[:48]
x1=list1[48:]
y1=list2[48:]
z1=list3[48:]
print('Ref Tags : {}'.format(len(x)))
print('Target Tags : {}'.format(len(x1)))
ax.scatter(x, y, z, c='r', marker='s')
ax.scatter(x1, y1, z1, c='b', marker='s')
ax.set_xlabel(' X - Axis ')
ax.set_ylabel(' Y - Axis ')
ax.set_zlabel(' Z - Axis ')
plt.show()