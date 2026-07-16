1
*set var N1=GenData(HelQ4g,int)
*set var nn1=operation(N1/10)
*nn1 3
*for(i=1;i<=N1;i=i+10)
*GenData(HelQ4g,*operation(i+0)) *GenData(HelQ4g,*operation(i+1)) *GenData(HelQ4g,*operation(i+2)) 
*end for
1
*nn1 7
*for(i=1;i<=N2;i=i+10)
*GenData(DelQ4g,*operation(i+3)) *GenData(DelQ4g,*operation(i+4)) *GenData(DelQ4g,*operation(i+5)) *GenData(DelQ4g,*operation(i+6)) *GenData(DelQ4g,*operation(i+7)) *GenData(DelQ4g,*operation(i+8)) *GenData(DelQ4g,*operation(i+9))
*end for
1
*nn1 3
*for(i=1;i<=N2;i=i+10)
*GenData(DelQ4g,*operation(i+3)) *GenData(DelQ4g,*operation(i+4)) *GenData(DelQ4g,*operation(i+5)) 
*end for