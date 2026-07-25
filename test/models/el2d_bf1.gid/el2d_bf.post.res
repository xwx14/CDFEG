GID Post Results File 1.0
GaussPoints "GP_ElQ4g" ElemType Quadrilateral
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_ElQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                7.5000000e-01 -2.7841145e-03 2.3174132e-03
         2                7.5000000e-01 -2.5346707e-03 -2.3174132e-03
End Values
GaussPoints "GP_ElT3g" ElemType Triangle
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_ElT3g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         3                1.7872848e-01 1.2061814e-02 1.2061814e-02
         4                2.5121604e-01 -5.5682289e-03 -1.5366944e-02
         5                2.5121604e-01 1.1188761e-02 1.4493891e-02
         6                3.1883944e-01 -5.0693413e-03 -1.1188761e-02
End Values
GaussPoints "GP_StressBL2g" ElemType Line
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
GaussPoints "GP_ElQ4g" ElemType Quadrilateral
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleVolume" "Load Analysis"           1 Scalar OnGaussPoints "GP_ElQ4g"
ComponentNames "volume" 
Values
         1                2.5000000e-01
         2                2.5000000e-01
End Values
GaussPoints "GP_ElT3g" ElemType Triangle
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleVolume" "Load Analysis"           1 Scalar OnGaussPoints "GP_ElT3g"
ComponentNames "volume" 
Values
         3                1.2500000e-01
         4                1.2500000e-01
         5                1.2500000e-01
         6                1.2500000e-01
End Values
GaussPoints "GP_StressBL2g" ElemType Line
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleVolume" "Load Analysis"           1 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"           1 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                3.7401228e-07 2.7217535e-09
         3                4.6337652e-07 -2.3435646e-08
         4                0.0000000e+00 0.0000000e+00
         5                3.7598772e-07 -6.2360945e-11
         6                5.0159574e-07 -1.7404739e-08
         7                0.0000000e+00 0.0000000e+00
         8                3.7401228e-07 -2.5970316e-09
         9                5.3343200e-07 -1.1810358e-08
End Values
Result "stress" "Load Analysis"           1 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                7.4934152e-01 -1.8560763e-03 2.4521933e-03
         2                4.4398766e-01 2.6438742e-04 3.8882193e-04
         3                1.7872848e-01 1.2061814e-02 1.2061814e-02
         4                7.5065848e-01 -1.7729284e-03 -2.0786982e-05
         5                5.1220783e-01 -1.6810633e-03 -2.0206958e-03
         6                2.2705352e-01 5.8941151e-03 3.7295868e-03
         7                7.4934152e-01 -1.6897804e-03 -2.4106193e-03
         8                5.7714069e-01 -4.0554731e-03 -6.3169626e-03
         9                2.8502774e-01 3.0597096e-03 1.6525650e-03
End Values
