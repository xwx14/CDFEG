GID Post Results File 1.0
Result "disp" "Load Analysis"           1 Vector OnNodes
ComponentNames "u" "v"
Values
         1                -1.0214411e-05 -9.7636690e-06
         2                -8.3579323e-06 -9.7639201e-06
         3                -1.0214667e-05 -7.8254008e-06
End Values
Result "stress" "Load Analysis"           1 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY"
Values
         1                -3.9022409e+00 3.3982224e+00 -1.7954747e-01
         2                -3.7875871e+00 5.5141562e+00 -1.1488301e+00
         3                -8.6155805e+00 2.1783795e+00 -7.5649531e-01
End Values
Result "disp" "Load Analysis"           2 Vector OnNodes
ComponentNames "u" "v"
Values
         1                -5.0607906e-05 -4.8384212e-05
End Values
GaussPoints "GP_DelQ4g" ElemType Quadrilateral
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
GaussPoints "GP_StressBL2g" ElemType Line
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY"
Values
         1 1.0 2.0 3.0
         2 4.0 5.0 6.0
End Values
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY"
Values
         1 0.0 0.0 0.0
End Values
