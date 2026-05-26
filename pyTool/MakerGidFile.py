# SPDX-License-Identifier: GPL-3.0
# This file is part of CDFEG.
#
# CDFEG is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CDFEG is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.

from MakerBase import MakerBase
from DataProject import DataProject
import os

def getEleTypeName(iType:int):
    """将单元类型整数转换为字符串"""
    if iType==0:
        return "Point"
    elif iType==1:
        return "Line"
    elif iType==2:
        return "Surface"
    elif iType==3:
        return "Volume"

class MakerGidFile(MakerBase):
    def __init__(self,arg1,arg2,arg3=None):
        # 初始化基类
        super().__init__()

        # 根据参数类型选择初始化方式
        if isinstance(arg1, DataProject):
            self._initFromProject(arg1, arg2)
        else:
            if arg3 is None:
                arg3=arg1
            self._initFromParams(arg1, arg2, arg3)
    def _initFromProject(self,pro:DataProject,path0:str):
        """从 DataProject 对象初始化"""
        # 确保输出目录存在
        os.makedirs(path0, exist_ok=True)
        path=path0+"/"+pro.name+".gid/"
        os.makedirs(path, exist_ok=True)

        self.data={"name":pro.name,
        "dim":pro.dim,"fields":[],"elems":[],"dbcs":[],"materials":[],"bDynamic":False}
        for field in pro.fields:
            # 添加场
            dof=len(field.dispNames)
            index=field.index
            field1={'name':field.name,
            'fullname':self.data['name']+field.name,
            'dof':dof,
            "dispNames":field.dispNames,
            "bDynamic":field.bDynamic,
            "index":index}
            if field.bDynamic:
                self.data["bDynamic"] = True
            self.data["fields"].append(field1)
            for ele in field.eleSubs:
                # 添加单元
                idType="elemsConec"
                if ele.bBC:
                    idType="globalnodes"
                self.data['elems'].append({
                    "name":ele.name,
                    "nNodes":ele.nNodes,
                    "type":getEleTypeName(ele.type),
                    "idType":idType,
                    "index":ele.index
                    })
                # 添加材料
                nd=len(ele.paramValues)
                nParams=len(ele.paramNames)
                if nd<nParams:
                    for i in range(nParams-nd):
                        ele.paramValues.append(0.0)
                self.data["materials"].append( {
                    "name":ele.name,
                    "nParams":nParams,
                    "params":ele.paramNames,
                    "defaultParams":ele.paramValues,
                    "strParams":",".join(ele.paramNames),
                    "index":index
                    })

        self.basFn=path+"\\"+pro.name+".bas"
        self.prbFn=path+"\\"+pro.name+".prb"
        self.cndFn=path+"\\"+pro.name+".cnd"

    def _initFromParams(self,name:str,dim:int,proName):
        """从参数初始化"""
        self.data={"name":name,
        "dim":dim,"fields":[],"elems":[],"dbcs":[],"materials":[]}
        self.basFn=proName+".bas"
        self.prbFn=proName+".prb"
        self.cndFn=proName+".cnd"

    def addField(self,name,dof,index=1,bDynamic=False):
        self.data['fields'].append({
            'name':name,
            'fullname':self.data['name']+name,
            'dof':dof, 
            "index":index,
            "bDynamic":bDynamic,
        })
    def addElem(self,name,nNodes,type,index=1,bBc=False):
        idType="elemsConec"
        if bBc:
            idType="globalnodes"
        self.data['elems'].append({
            "name":name,
            "nNodes":nNodes,
            "type":type,
            "idType":idType, 
            "index":index
        })
    def addElemWithMat(self,name,nNodes,type,matParams,defaultParams=[],index=1,bBc=False):
        self.addElem(name,nNodes,type,index,bBc)
        self.addMaterial(name,matParams,defaultParams,index)

    def addDbc(self,name,n,index=1):
        self.data['dbcs'].append({
            "name":name,
            "n":n,
            "index":index
        })
    def addMaterial(self,name:str,params:list,defaultParams:list=[],index=1):
        nd=len(defaultParams)
        nParams=len(params)
        if nd<nParams:
            for i in range(nParams-nd):
                defaultParams.append(0.0)
        self.data["materials"].append( {
            "name":name,
            "nParams":nParams,
            "params":params,
            "defaultParams":defaultParams,
            "strParams":",".join(params),
            "index":index
            })

    # write2File 方法已移至基类 MakerBase

    def makeBas(self):
        self.write2File('mfelbas.j2',self.basFn,self.data)
    def makePrb(self):
        self.write2File('mfelprb.j2',self.prbFn,self.data)
    def makeCnd(self):
        self.write2File('mfelcnd.j2',self.cndFn,self.data)
    def makeAll(self):
        self.makeBas()
        self.makePrb()
        self.makeCnd()