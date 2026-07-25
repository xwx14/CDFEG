GID Post Results File 1.0
GaussPoints "GP_ElQ4g" ElemType Quadrilateral
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_ElQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                1.3461538e+03 5.7692308e+02 -2.8668807e-13
         2                1.3461538e+03 5.7692308e+02 -9.2522060e-13
         3                1.3461538e+03 5.7692308e+02 4.8997598e-13
         4                1.3461538e+03 5.7692308e+02 2.4603049e-12
         5                1.3461538e+03 5.7692308e+02 2.6844429e-13
         6                1.3461538e+03 5.7692308e+02 1.5741782e-12
         7                1.3461538e+03 5.7692308e+02 6.5677632e-13
         8                1.3461538e+03 5.7692308e+02 2.0224541e-12
         9                1.3461538e+03 5.7692308e+02 1.8738975e-12
        10                1.3461538e+03 5.7692308e+02 3.3855255e-12
        11                1.3461538e+03 5.7692308e+02 2.9346434e-12
        12                1.3461538e+03 5.7692308e+02 3.3464317e-12
        13                1.3461538e+03 5.7692308e+02 -2.0328791e-13
        14                1.3461538e+03 5.7692308e+02 -3.2056939e-13
        15                1.3461538e+03 5.7692308e+02 -4.3263837e-13
        16                1.3461538e+03 5.7692308e+02 3.1275063e-13
End Values
GaussPoints "GP_ElT3g" ElemType Triangle
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_ElT3g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
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
         1                6.2500000e-02
         2                6.2500000e-02
         3                6.2500000e-02
         4                6.2500000e-02
         5                6.2500000e-02
         6                6.2500000e-02
         7                6.2500000e-02
         8                6.2500000e-02
         9                6.2500000e-02
        10                6.2500000e-02
        11                6.2500000e-02
        12                6.2500000e-02
        13                6.2500000e-02
        14                6.2500000e-02
        15                6.2500000e-02
        16                6.2500000e-02
End Values
GaussPoints "GP_ElT3g" ElemType Triangle
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleVolume" "Load Analysis"           1 Scalar OnGaussPoints "GP_ElT3g"
ComponentNames "volume" 
Values
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
         2                2.5000000e-04 0.0000000e+00
         3                5.0000000e-04 0.0000000e+00
         4                7.5000000e-04 0.0000000e+00
         5                1.0000000e-03 0.0000000e+00
         6                0.0000000e+00 0.0000000e+00
         7                2.5000000e-04 1.1052216e-19
         8                5.0000000e-04 1.2332800e-18
         9                7.5000000e-04 2.3716923e-19
        10                1.0000000e-03 0.0000000e+00
        11                0.0000000e+00 0.0000000e+00
        12                2.5000000e-04 5.2198405e-19
        13                5.0000000e-04 1.3810872e-18
        14                7.5000000e-04 0.0000000e+00
        15                1.0000000e-03 0.0000000e+00
        16                0.0000000e+00 0.0000000e+00
        17                2.5000000e-04 4.4384526e-19
        18                5.0000000e-04 8.7752613e-19
        19                7.5000000e-04 -6.0986372e-20
        20                1.0000000e-03 0.0000000e+00
        21                0.0000000e+00 0.0000000e+00
        22                2.5000000e-04 0.0000000e+00
        23                5.0000000e-04 0.0000000e+00
        24                7.5000000e-04 0.0000000e+00
        25                1.0000000e-03 0.0000000e+00
End Values
Result "stress" "Load Analysis"           1 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                1.3461538e+03 5.7692308e+02 -2.5826428e-13
         2                1.3461538e+03 5.7692308e+02 -1.1810601e-12
         3                1.3461538e+03 5.7692308e+02 -3.5870395e-12
         4                1.3461538e+03 5.7692308e+02 3.4265461e-12
         5                1.3461538e+03 5.7692308e+02 -2.7636989e-12
         6                1.3461538e+03 5.7692308e+02 4.9360676e-14
         7                1.3461538e+03 5.7692308e+02 8.5320925e-13
         8                1.3461538e+03 5.7692308e+02 -9.3537850e-13
         9                1.3461538e+03 5.7692308e+02 2.7689186e-12
        10                1.3461538e+03 5.7692308e+02 8.1930902e-13
        11                1.3461538e+03 5.7692308e+02 9.8175632e-13
        12                1.3461538e+03 5.7692308e+02 2.4659617e-12
        13                1.3461538e+03 5.7692308e+02 9.2530837e-13
        14                1.3461538e+03 5.7692308e+02 3.5442361e-12
        15                1.3461538e+03 5.7692308e+02 1.2441910e-12
        16                1.3461538e+03 5.7692308e+02 7.8671931e-13
        17                1.3461538e+03 5.7692308e+02 1.8253219e-12
        18                1.3461538e+03 5.7692308e+02 3.1173014e-13
        19                1.3461538e+03 5.7692308e+02 2.5525093e-12
        20                1.3461538e+03 5.7692308e+02 7.1435927e-13
        21                1.3461538e+03 5.7692308e+02 -6.6657824e-14
        22                1.3461538e+03 5.7692308e+02 1.2254419e-12
        23                1.3461538e+03 5.7692308e+02 4.2876673e-13
        24                1.3461538e+03 5.7692308e+02 5.5797196e-13
        25                1.3461538e+03 5.7692308e+02 3.5366212e-12
End Values
