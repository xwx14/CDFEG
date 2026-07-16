** name=baseData,structure="I32"
*npoin  *nelem 
** name=mat_HelQ4g,structure="F64**10",type="mat",index=0
*set var N=GenData(HelQ4g,int)
*for(i=1;i<=N;i=i+10)
*GenData(HelQ4g,*operation(i+0)) *GenData(HelQ4g,*operation(i+1)) *GenData(HelQ4g,*operation(i+2)) *GenData(HelQ4g,*operation(i+3)) *GenData(HelQ4g,*operation(i+4)) *GenData(HelQ4g,*operation(i+5)) *GenData(HelQ4g,*operation(i+6)) *GenData(HelQ4g,*operation(i+7)) *GenData(HelQ4g,*operation(i+8)) *GenData(HelQ4g,*operation(i+9)) 
*end for
** name=coord,structure="I32**1 F64**2",type="coord",index=1
*loop nodes
*format "%6i %12.6e %12.6e"
  *NodesNum *NodesCoord
*end
** name=idHeat,structure="I32**2",type="id",index=0
*Set Cond volume-hel2dHeat *nodes
*Add Cond surface-hel2dHeat *nodes
*Add Cond line-hel2dHeat *nodes
*Add Cond point-hel2dHeat *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(1)
*end  
** name=ubfHeat,structure="I32**1 F64**1",type="ubf",index=0
*Set Cond volume-hel2dHeat *nodes
*Add Cond surface-hel2dHeat *nodes
*Add Cond line-hel2dHeat *nodes
*Add Cond point-hel2dHeat *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(2)
*end
** name=idDelDisp,structure="I32**3",type="id",index=1
*Set Cond volume-hel2dDelDisp *nodes
*Add Cond surface-hel2dDelDisp *nodes
*Add Cond line-hel2dDelDisp *nodes
*Add Cond point-hel2dDelDisp *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(1) *cond(3)
*end  
** name=ubfDelDisp,structure="I32**1 F64**2",type="ubf",index=1
*Set Cond volume-hel2dDelDisp *nodes
*Add Cond surface-hel2dDelDisp *nodes
*Add Cond line-hel2dDelDisp *nodes
*Add Cond point-hel2dDelDisp *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(2) *cond(4)
*end
** name=HelQ4g,structure="I32**6",type="elem",index=1
*set cond Surface-HelQ4g *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*format "%10i %10i %10i %10i %10i %10i "
*end
