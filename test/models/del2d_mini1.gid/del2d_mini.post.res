GID Post Results File 1.0
GaussPoints "GP_DelQ4g" ElemType Quadrilateral
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
GaussPoints "GP_StressBL2g" ElemType Line
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleStress" "Load Analysis"           1 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
GaussPoints "GP_DelQ4g" ElemType Quadrilateral
Number Of Gauss Points: 1
Natural Coordinates: Internal
End GaussPoints
Result "eleVolume" "Load Analysis"           1 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
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
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"           1 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"           1 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"           1 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           2 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           2 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"           2 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"           2 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"           2 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"           2 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"           2 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"           2 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           3 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           3 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"           3 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"           3 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"           3 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"           3 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"           3 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"           3 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           4 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           4 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"           4 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"           4 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"           4 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"           4 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"           4 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"           4 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           5 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           5 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"           5 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"           5 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"           5 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"           5 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"           5 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"           5 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           6 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           6 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"           6 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"           6 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"           6 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"           6 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"           6 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"           6 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           7 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           7 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"           7 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"           7 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"           7 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"           7 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"           7 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"           7 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           8 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           8 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"           8 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"           8 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"           8 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"           8 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"           8 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"           8 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           9 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"           9 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"           9 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"           9 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"           9 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"           9 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"           9 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"           9 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"          10 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"          10 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"          10 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"          10 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"          10 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"          10 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"          10 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"          10 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"          11 Matrix OnGaussPoints "GP_DelQ4g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
Result "eleStress" "Load Analysis"          11 Matrix OnGaussPoints "GP_StressBL2g"
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
End Values
Result "eleVolume" "Load Analysis"          11 Scalar OnGaussPoints "GP_DelQ4g"
ComponentNames "volume" 
Values
         1                1.0000000e+00
End Values
Result "eleVolume" "Load Analysis"          11 Scalar OnGaussPoints "GP_StressBL2g"
ComponentNames "volume" 
Values
End Values
Result "disp" "Load Analysis"          11 Vector OnNodes
ComponentNames "u" "v" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "velocity" "Load Analysis"          11 Vector OnNodes
ComponentNames "velU" "velV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "acceleration" "Load Analysis"          11 Vector OnNodes
ComponentNames "accU" "accV" 
Values
         1                0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00
End Values
Result "stress" "Load Analysis"          11 Matrix OnNodes
ComponentNames "sigmaXX" "sigmaYY" "sigmaXY" 
Values
         1                0.0000000e+00 0.0000000e+00 0.0000000e+00
         2                0.0000000e+00 0.0000000e+00 0.0000000e+00
         3                0.0000000e+00 0.0000000e+00 0.0000000e+00
         4                0.0000000e+00 0.0000000e+00 0.0000000e+00
End Values
