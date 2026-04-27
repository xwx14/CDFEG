import sys
sys.path.append("./")
sys.path.append("../")
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
from MakerCpp import MakerCpp

projectDict={'name':"TimBeam1D",
             "fields":[
                 {'name':"Disp",
                  'eles':[{
                      'name':"TimBeam1D",
                      'dispNames':['v','phi'],
                      'params':['E','I']

                  }

                  ]
                  }
             ]
             }

