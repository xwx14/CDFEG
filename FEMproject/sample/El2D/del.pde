\ ................ elastic deformation equation .........
\ sij,j + fi = 0
\ s = d*e, where eij = (ui,j+uj,i)*aij
\ aij = 1/2 if i=j else aij=1
\ where u denote displacement, s denote stress, e denote strain
\ d denote the constitutive matrix
\ ......................................................
\ pde in weak form
\ (d*e,de) = (f,du)
\ where de denotes the variation of e
\ ......................................................
disp u v
coor x y
func exx eyy exy
shap %1 %2
gaus %3
$c6 double pe,pv,fu,fv,rou,alpha;
mate pe pv fu fv rou alpha 1.0e10 0.3 0.0 0.0 3000.0 0.6
mass %1 rou*vol
damp %1 rou*alpha*vol
 
func
$c6 vol = 1.0;
$c6 fact = pe/(1.0+pv)/(1.0-pv*2.0)*vol;
$c6 shear = (0.5-pv);
exx=+[u/x]

eyy=+[v/y]

exy=+[u/y]+[v/x]

stif
dist=+[exx;exx]*(1.-pv)*fact+[exx;eyy]*pv*fact+[eyy;exx]*pv*fact
+[eyy;eyy]*(1.-pv)*fact+[exy;exy]*shear*fact


load=+[u]*fu*vol+[v]*fv*vol

end
   
