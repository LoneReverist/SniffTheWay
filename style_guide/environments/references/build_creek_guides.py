from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
P=Path('style_guide/environments/references')
# Map x is east; convert map y to north-positive world coordinates.
def poly(ax,xy,z,c):
 ax.add_collection3d(Poly3DCollection([[(x,100-y,z) for x,y in xy]],facecolors=c,edgecolors='#665e4b',linewidths=.6,zorder=z+1))
def stone(ax,x,y,rx,ry,kind):
 t=np.linspace(0,2*np.pi,40,endpoint=False);xy=[(x+rx*np.cos(a),y+ry*np.sin(a)) for a in t]
 if kind==2:xy[0]=(x+rx-2,y)
 if kind==3:xy=[(a,max(b,y-ry+1.2)) for a,b in xy]
 poly(ax,xy,1.0,'#cfbd98')
 for i in range(len(xy)):
  a,b=xy[i],xy[(i+1)%len(xy)]
  ax.add_collection3d(Poly3DCollection([[(a[0],100-a[1],0),(b[0],100-b[1],0),(b[0],100-b[1],1),(a[0],100-a[1],1)]],facecolors='#998c73',edgecolors='none',zorder=1.9))
 ax.text(x,100-y,1.4,f'S{kind}',ha='center',zorder=10,fontsize=13,color='#382b1c')
for name,az in [('creek_approach_guide',-108),('creek_reverse_guide',80)]:
 fig=plt.figure(figsize=(14,8));ax=fig.add_subplot(111,projection='3d',computed_zorder=False);fig.patch.set_facecolor('#f7f2e8')
 poly(ax,[(5,32),(95,32),(95,68),(5,68)],0,'#8cb9c0')
 poly(ax,[(5,68),(95,68),(95,98),(5,98)],.6,'#8d9e75');poly(ax,[(5,2),(95,2),(95,32),(5,32)],.6,'#8d9e75')
 poly(ax,[(40,98),(47,98),(53,68),(43,68)],.7,'#c3ad86');poly(ax,[(45,32),(55,32),(69,2),(60,2)],.7,'#c3ad86')
 for row in [(48,60,8,4.5,1),(52,50,9,4.5,2),(50,40,8,4.5,3)]:stone(ax,*row)
 for x,y,label in [(25,75,'T1 / Bank A'),(25,23,'T2 / Bank B')]:
  ax.plot([x,x],[100-y,100-y],[.6,20],color='#756849',linewidth=16,zorder=5);ax.text(x,100-y,22,label,ha='center',zorder=10,fontsize=11)
 ax.scatter([72],[28],[1],zorder=6,s=700,c='#8b9185');ax.text(72,28,3,'R / A boulder',zorder=10,fontsize=10)
 ax.scatter([74],[75],[1],zorder=6,s=700,c='#b198b5');ax.text(74,75,3,'F / B flowers',zorder=10,fontsize=10)
 ax.quiver(15,50,1,14,0,0,color='#315c73',zorder=8,linewidth=2,arrow_length_ratio=.2)
 ax.set(xlim=(5,95),ylim=(2,98),zlim=(0,35));ax.set_box_aspect((1.4,1.5,.55));ax.view_init(elev=24,azim=az);ax.set_axis_off();ax.set_title(name.replace('_',' ').title()+' — fixed shared geometry',fontsize=17)
 plt.tight_layout();fig.savefig(P/(name+'.png'),dpi=130);plt.close(fig)
