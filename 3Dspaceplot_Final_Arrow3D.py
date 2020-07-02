#Diploma Thesis , June 2020
#George Kalitsios ,THMMY AUTH
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from mpl_toolkits.mplot3d import proj3d
from matplotlib.patches import FancyArrowPatch

class Arrow3D(FancyArrowPatch):
    def __init__(self, xs, ys, zs, *args, **kwargs):
        FancyArrowPatch.__init__(self, (0,0), (0,0), *args, **kwargs)
        self._verts3d = xs, ys, zs

    def draw(self, renderer):
        xs3d, ys3d, zs3d = self._verts3d
        xs, ys, zs = proj3d.proj_transform(xs3d, ys3d, zs3d, renderer.M)
        self.set_positions((xs[0],ys[0]),(xs[1],ys[1]))
        FancyArrowPatch.draw(self, renderer)

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
df = pd.read_excel("All_tags.xlsx", sheet_name=0)
EPC=list(df['EPC_TAG']) 
list1 = list(df['X']) 
list2 = list(df['Y'])
list3 = list(df['Z'])

x=list1[:48]
y=list2[:48]
z=list3[:48]
x1=list1[48:]
y1=list2[48:]
z1=list3[48:]
epcknown=EPC[48:]


print('Ref Tags : {}'.format(len(x)))
print('Target Tags : {}'.format(len(x1)))
file1 = open("Estimation_results_RSSI_Fingerprinting.txt","r") 
allline=[]
for line in file1:
    currentline = line.split()
    allline.append(currentline)
l=len(allline)

i=1
epc=[]
xq=[]
yq=[]
zq=[]
errr=[]

while i<l:
    epc.append(allline[i][0])
    xq.append(float(allline[i][1]))
    yq.append(float(allline[i][2]))
    zq.append(float(allline[i][3]))
    errr.append(float(allline[i][4]))
    i=i+1
i=1
x2=[]
y2=[]
z2=[]
epcvel=[]
disvel=[]
k=0
xmin=min(x)
xmax=max(x)
zmin=min(z)
zmax=max(z)
ymin=min(y)
ymax=max(y)
while i <= len(epc):
    if errr[i-1]>0:
        if (xmin<=xq[i-1] and xq[i-1]<=xmax and ymin<=yq[i-1] and yq[i-1]<=ymax and zmin<=zq[i-1] and zq[i-1]<=zmax ):
            x2.append(xq[i-1])
            y2.append(yq[i-1])
            z2.append(zq[i-1])
            epcvel.append(epc[i-1])
            disvel.append(errr[i-1])
            k=k+1
    i=i+1

#mexri edw ola komple 

i = 0
dis=[]
x1velos=[]
y1velos=[]
z1velos=[]
xvelos=[]
yvelos=[]
zvelos=[]
# print(len(epcknown))
# print(len(x2))
while i <= len(epcknown)-1:
    j= 0 #<-- here
    while j <= len(epcvel)-1:
        if epcknown[i]==epcvel[j] :
           
            xvelos.append(x1[i])
            yvelos.append(y1[i])
            zvelos.append(z1[i])


            x1velos.append(x2[j])
            y1velos.append(y2[j])
            z1velos.append(z2[j])

            #apost
            dis.append(disvel[j])
        j=j+1
    i=i+1


print('Unknown Tags Found: {}'.format(k))
ax.scatter(x, y, z, s=50 , c='r', marker='s')
ax.scatter(x1, y1, z1, s=50 ,c='b', marker='s')
ax.scatter(x2, y2, z2, s=50 , c='g', marker='s')

print(x1velos[0],y1velos[0],z1velos[0])
print(xvelos[0],yvelos[0],zvelos[0])
print(dis[0])
aver=sum(dis)/len(dis)

q=0
for i in range(0,len(dis)):
	if dis[i]<=aver and q<=len(dis)/2:
		arw = Arrow3D([xvelos[i],x1velos[i]],[yvelos[i],y1velos[i]],[zvelos[i],z1velos[i]], arrowstyle="->", lw = 1, mutation_scale=25)
		ax.add_artist(arw)
		q=q+1

ax.set_xlabel(' X - Axis ')
ax.set_ylabel(' Y - Axis ')
ax.set_zlabel(' Z - Axis ')
plt.savefig("result.png")
plt.show()  