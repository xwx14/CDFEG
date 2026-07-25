GID Post Results File 1.0
GaussPoints "GP_ElT3" ElemType Triangle
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_ElT3"
ComponentNames "Sxx" "Syy" "Sxy" 
Values
         1                1.0000000e+03 0.0000000e+00 0.0000000e+00
         2                1.0000000e+03 0.0000000e+00 0.0000000e+00
         3                1.0000000e+03 0.0000000e+00 0.0000000e+00
         4                1.0000000e+03 0.0000000e+00 0.0000000e+00
         5                1.0000000e+03 0.0000000e+00 0.0000000e+00
         6                1.0000000e+03 0.0000000e+00 0.0000000e+00
         7                1.0000000e+03 0.0000000e+00 0.0000000e+00
         8                1.0000000e+03 -1.1368684e-13 5.6843419e-14
End Values
Result "disp" "Load Analysis"           1 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                5.0000000e-04 0.0000000e+00
         3                1.0000000e-03 0.0000000e+00
         4                0.0000000e+00 -1.5000000e-04
         5                5.0000000e-04 -1.5000000e-04
         6                1.0000000e-03 -1.5000000e-04
         7                0.0000000e+00 -3.0000000e-04
         8                5.0000000e-04 -3.0000000e-04
         9                1.0000000e-03 -3.0000000e-04
End Values
Result "stress" "Load Analysis"           1 Matrix OnNodes
ComponentNames "Sxx" "Syy" "Sxy" 
Values
         1                1.0000000e+03 0.0000000e+00 0.0000000e+00
         2                1.0000000e+03 0.0000000e+00 0.0000000e+00
         3                1.0000000e+03 0.0000000e+00 0.0000000e+00
         4                1.0000000e+03 0.0000000e+00 0.0000000e+00
         5                1.0000000e+03 -1.8947806e-14 9.4739031e-15
         6                1.0000000e+03 0.0000000e+00 0.0000000e+00
         7                1.0000000e+03 0.0000000e+00 0.0000000e+00
         8                1.0000000e+03 -3.7895613e-14 1.8947806e-14
         9                1.0000000e+03 -5.6843419e-14 2.8421709e-14
End Values
