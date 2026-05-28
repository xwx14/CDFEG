** name=baseData,structure="I32"
*npoin  *nelem 
** name=time,structure="F64"
*GenData(TimeStep) *GenData(TotalTime)
** name=mat_ElQ4g,structure="F64**6",type="mat",index=0
*set var N=GenData(ElQ4g,int)
*for(i=1;i<=N;i=i+6)
*GenData(ElQ4g,*operation(i+0)) *GenData(ElQ4g,*operation(i+1)) *GenData(ElQ4g,*operation(i+2)) *GenData(ElQ4g,*operation(i+3)) *GenData(ElQ4g,*operation(i+4)) *GenData(ElQ4g,*operation(i+5)) 
*end for
** name=mat_ElT3g,structure="F64**6",type="mat",index=0
*set var N=GenData(ElT3g,int)
*for(i=1;i<=N;i=i+6)
*GenData(ElT3g,*operation(i+0)) *GenData(ElT3g,*operation(i+1)) *GenData(ElT3g,*operation(i+2)) *GenData(ElT3g,*operation(i+3)) *GenData(ElT3g,*operation(i+4)) *GenData(ElT3g,*operation(i+5)) 
*end for
** name=mat_StressBL2g,structure="F64**2",type="mat",index=0
*set var N=GenData(StressBL2g,int)
*for(i=1;i<=N;i=i+2)
*GenData(StressBL2g,*operation(i+0)) *GenData(StressBL2g,*operation(i+1)) 
*end for
** name=coord,structure="I32**1 F64**2",type="coord",index=1
*loop nodes
*format "%6i %12.6e %12.6e"
  *NodesNum *NodesCoord
*end
** name=idElDisp,structure="I32**3",type="id",index=0
*Set Cond volume-el2dElDisp *nodes
*Add Cond surface-el2dElDisp *nodes
*Add Cond line-el2dElDisp *nodes
*Add Cond point-el2dElDisp *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(1) *cond(3)
*end  
** name=ubfElDisp,structure="I32**1 F64**2",type="ubf",index=0
*Set Cond volume-el2dElDisp *nodes
*Add Cond surface-el2dElDisp *nodes
*Add Cond line-el2dElDisp *nodes
*Add Cond point-el2dElDisp *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(2) *cond(4)
*end  
** name=ElQ4g,structure="I32**6",type="elem",index=1
*set cond Surface-ElQ4g *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*format "%10i %10i %10i %10i %10i %10i "
*end
** name=ElT3g,structure="I32**5",type="elem",index=1
*set cond Surface-ElT3g *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*format "%10i %10i %10i %10i %10i "
*end
** name=StressBL2g,structure="I32**4",type="elem",index=1
*set cond Line-StressBL2g *elems
*loop elems *OnlyIncond
*ElemsNum *globalnodes *cond(1)
*format "%10i %10i %10i %10i "
*end
