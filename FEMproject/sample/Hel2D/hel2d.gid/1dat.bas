*npoin  *nelem 
-1000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0  
*loop nodes
*format "%6i %12.6e %12.6e %12.6e"
  *NodesNum *NodesCoord
*end
-2001 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
*Set Cond volume-hel2dHeat *nodes
*Add Cond surface-hel2dHeat *nodes
*Add Cond line-hel2dHeat *nodes
*Add Cond point-hel2dHeat *nodes
*loop nodes *OnlyInCond
    *NodesNum *cond(1)
*end
-2001 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
*Set Cond volume-hel2dHeat *nodes
*Add Cond surface-hel2dHeat *nodes
*Add Cond line-hel2dHeat *nodes
*Add Cond point-hel2dHeat *nodes
*loop nodes *OnlyInCond
    *NodesNum *cond(2)
*end
-2002 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
*Set Cond volume-hel2dDelDisp *nodes
*Add Cond surface-hel2dDelDisp *nodes
*Add Cond line-hel2dDelDisp *nodes
*Add Cond point-hel2dDelDisp *nodes
*loop nodes *OnlyInCond
    *NodesNum *cond(1) *cond(3)
*end
-2002 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
*Set Cond volume-hel2dDelDisp *nodes
*Add Cond surface-hel2dDelDisp *nodes
*Add Cond line-hel2dDelDisp *nodes
*Add Cond point-hel2dDelDisp *nodes
*loop nodes *OnlyInCond
    *NodesNum *cond(2) *cond(4)
*end
-2003 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

-2003 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

-4000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
-4 2 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
*set cond Surface-HelQ4g *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*format "%10i %10i %10i %10i %10i %10i "
*end
-4 2 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
*set cond Surface-HelQ4g *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*format "%10i %10i %10i %10i %10i %10i "
*end
-4 2 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
*set cond Surface-HelQ4g *elems
*loop elems *OnlyIncond
*ElemsNum *elemsConec *cond(1)
*format "%10i %10i %10i %10i %10i %10i "
*end
-5000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
