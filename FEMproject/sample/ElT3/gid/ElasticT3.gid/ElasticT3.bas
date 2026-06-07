** name=baseData,structure="I32"
*npoin  *nelem 
** name=time,structure="F64"
*GenData(TimeStep) *GenData(TotalTime)
** name=mat_ElT3,structure="F64**5",type="mat",index=0
*set var N=GenData(ElT3,int)
*for(i=1;i<=N;i=i+5)
*GenData(ElT3,*operation(i+0)) *GenData(ElT3,*operation(i+1)) *GenData(ElT3,*operation(i+2)) *GenData(ElT3,*operation(i+3)) *GenData(ElT3,*operation(i+4)) 
*end for
** name=coord,structure="I32**1 F64**2",type="coord",index=1
*loop nodes
*format "%6i %12.6e %12.6e"
  *NodesNum *NodesCoord
*end
** name=idElastic2DDisp,structure="I32**3",type="id",index=0
*Set Cond volume-ElasticT3Elastic2DDisp *nodes
*Add Cond surface-ElasticT3Elastic2DDisp *nodes
*Add Cond line-ElasticT3Elastic2DDisp *nodes
*Add Cond point-ElasticT3Elastic2DDisp *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(1) *cond(3)
*end  
** name=ubfElastic2DDisp,structure="I32**1 F64**2",type="ubf",index=0
*Set Cond volume-ElasticT3Elastic2DDisp *nodes
*Add Cond surface-ElasticT3Elastic2DDisp *nodes
*Add Cond line-ElasticT3Elastic2DDisp *nodes
*Add Cond point-ElasticT3Elastic2DDisp *nodes
*loop nodes *OnlyInCond
*NodesNum *cond(2) *cond(4)
*end  
** name=ElT3,structure="I32**5",type="elem",index=1
*set cond Surface-ElT3 *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*format "%10i %10i %10i %10i %10i "
*end
