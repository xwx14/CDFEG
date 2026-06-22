** name=baseData,structure="I32"
*npoin  *nelem 
** name=mat_a1eq4g2,structure="F64**6",type="mat",index=0
*set var N=GenData(a1eq4g2,int)
*for(i=1;i<=N;i=i+6)
*GenData(a1eq4g2,*operation(i+0)) *GenData(a1eq4g2,*operation(i+1)) *GenData(a1eq4g2,*operation(i+2)) *GenData(a1eq4g2,*operation(i+3)) *GenData(a1eq4g2,*operation(i+4)) *GenData(a1eq4g2,*operation(i+5)) 
*end for
** name=mat_a2ll2,structure="F64**2",type="mat",index=0
*set var N=GenData(a2ll2,int)
*for(i=1;i<=N;i=i+2)
*GenData(a2ll2,*operation(i+0)) *GenData(a2ll2,*operation(i+1)) 
*end for
** name=mat_beq4g2,structure="F64**2",type="mat",index=1
*set var N=GenData(beq4g2,int)
*for(i=1;i<=N;i=i+2)
*GenData(beq4g2,*operation(i+0)) *GenData(beq4g2,*operation(i+1)) 
*end for
** name=coord,structure="I32**1 F64**2",type="coord",index=1
*loop nodes
*format "%6i %12.6e %12.6e"
  *NodesNum *NodesCoord
*end
** name=idela,structure="I32**3",type="id",index=0
*Set Cond volume-elela *nodes
*Add Cond surface-elela *nodes
*Add Cond line-elela *nodes
*Add Cond point-elela *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(1) *cond(3)
*end  
** name=ubfela,structure="I32**1 F64**2",type="ubf",index=0
*Set Cond volume-elela *nodes
*Add Cond surface-elela *nodes
*Add Cond line-elela *nodes
*Add Cond point-elela *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(2) *cond(4)
*end** name=idelb,structure="I32**4",type="id",index=1
*Set Cond volume-elelb *nodes
*Add Cond surface-elelb *nodes
*Add Cond line-elelb *nodes
*Add Cond point-elelb *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(1) *cond(3) *cond(5)
*end  
** name=ubfelb,structure="I32**1 F64**3",type="ubf",index=1
*Set Cond volume-elelb *nodes
*Add Cond surface-elelb *nodes
*Add Cond line-elelb *nodes
*Add Cond point-elelb *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(2) *cond(4) *cond(6)
*end** name=a1eq4g2,structure="I32**6",type="elem",index=1
*set cond Surface-a1eq4g2 *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*format "%10i %10i %10i %10i %10i %10i "
*end
** name=a2ll2,structure="I32**4",type="elem",index=1
*set cond Line-a2ll2 *elems
*loop elems *OnlyIncond
*ElemsNum *globalnodes *cond(1)
*format "%10i %10i %10i %10i "
*end
** name=beq4g2,structure="I32**6",type="elem",index=1
*set cond Surface-beq4g2 *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*format "%10i %10i %10i %10i %10i %10i "
*end
