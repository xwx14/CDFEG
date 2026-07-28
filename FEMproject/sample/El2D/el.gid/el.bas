** name=baseData,structure="I32"
*npoin  *nelem
** name=time,structure="F64"
*GenData(TimeStep) *GenData(TotalTime)
** name=El,structure="F64**6",type="mat",index=0
*set var N=GenData(El,int)
*for(i=1;i<=N;i=i+6)
*GenData(El,*operation(i+0)) *GenData(El,*operation(i+1)) *GenData(El,*operation(i+2)) *GenData(El,*operation(i+3)) *GenData(El,*operation(i+4)) *GenData(El,*operation(i+5))
*end for
** name=StressBL2g,structure="F64**2",type="mat",index=0
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
*Set Cond volume-elElDisp *nodes
*Add Cond surface-elElDisp *nodes
*Add Cond line-elElDisp *nodes
*Add Cond point-elElDisp *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(1) *cond(3)
*end
** name=ubfElDisp,structure="I32**1 F64**2",type="ubf",index=0
*Set Cond volume-elElDisp *nodes
*Add Cond surface-elElDisp *nodes
*Add Cond line-elElDisp *nodes
*Add Cond point-elElDisp *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(2) *cond(4)
*end
** name=El,type="elem",index=1
*set cond Surface-El *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*end
** name=StressBL2g,type="elem",index=1
*set cond Line-StressBL2g *elems
*loop elems *OnlyIncond
*ElemsNum *globalnodes *cond(1)
*end
