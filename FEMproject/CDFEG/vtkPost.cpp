// SPDX-License-Identifier: GPL-3.0
// This file is part of CDFEG.
//
// CDFEG is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDFEG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.
#include "vtkPost.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include "PhyFieldData.h"
#include "DomainData.h"
#include "ResItem.h"
namespace CDFEG {
    vtkPost::vtkPost(DomainData* data):Processor(data)
    {
    }

    vtkPost::~vtkPost()
    {
    }

    void vtkPost::setFilePath(const std::string& parentPath, const std::string& baseName)
    {
        _outPath = parentPath;
        _baseName = baseName;
    }

    int vtkPost::post(int it)
    {
        std::ostringstream oss;
        oss << _outPath << "/" << _baseName << "_" << std::setw(4) << std::setfill('0') << it << ".vtu";
        std::string vtuFn = oss.str();
        if (writeVTU(vtuFn) != 0) return -1;
        double time = it * _femData->_dt;
        _steps.push_back({it, time});
        std::string pvdFn = _outPath + "/" + _baseName + ".pvd";
        return writePVD(pvdFn);
    }

    int vtkPost::writeVTU(const std::string& fn)
    {
        std::ofstream ofs(fn);
        if (!ofs.is_open()) return -1;
        ofs << std::setprecision(15) << std::scientific;   // 高精度，支撑紧回归容差
        int dim = _femData->_dim;
        int nPt = _femData->_nPts;
        int nEle = _femData->_nElem;

        // XML VTU 文件头
        ofs << "<?xml version=\"1.0\"?>" << std::endl;
        ofs << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
        ofs << "  <UnstructuredGrid>" << std::endl;
        ofs << "    <Piece NumberOfPoints=\"" << nPt << "\" NumberOfCells=\"" << nEle << "\">" << std::endl;

        // 输出节点坐标
        ofs << "      <Points>" << std::endl;
        ofs << "        <DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">" << std::endl;
        ofs << "          ";
        for (int iPt = 0; iPt < nPt; iPt++)
        {
            for (int iDim = 0; iDim < dim; iDim++)
            {
                ofs << _femData->_nodes[dim * iPt + iDim] << " ";
            }
            for (int iDim = dim; iDim < 3; iDim++)
            {
                ofs << 0.0 << " ";
            }
        }
        ofs << std::endl;
        ofs << "        </DataArray>" << std::endl;
        ofs << "      </Points>" << std::endl;

        // 输出单元连接关系
        ofs << "      <Cells>" << std::endl;
        ofs << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << std::endl;
        ofs << "          ";
        for (size_t i = 0; i < _femData->_eleNodes.size(); i++)
        {
            ofs << _femData->_eleNodes[i] << " ";
        }
        ofs << std::endl;
        ofs << "        </DataArray>" << std::endl;
        ofs << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << std::endl;
        ofs << "          ";
        for (int iEle = 0; iEle < nEle; iEle++)
        {
            ofs << _femData->_elePt[iEle + 1] << " ";
        }
        ofs << std::endl;
        ofs << "        </DataArray>" << std::endl;
        ofs << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">" << std::endl;
        ofs << "          ";
        for (int iEle = 0; iEle < nEle; iEle++)
        {
            ofs << static_cast<int>(_femData->_eleTypes[iEle]) << " ";
        }
        ofs << std::endl;
        ofs << "        </DataArray>" << std::endl;
        ofs << "      </Cells>" << std::endl;

        ofs << "      <PointData>" << std::endl;
        for (ResItem& item : _femData->_prePostConfig._nodeResItems)
        {
            if (item._iFields.empty()) continue;
            PhyFieldData* phy = _femData->_phyDatas[item._iFields[0]];
            ofs << "        <DataArray type=\"Float64\" Name=\"" << item._name
                << "\" NumberOfComponents=\"" << item._ValNames.size() << "\" format=\"ascii\">" << std::endl;
            ofs << "          ";
            for (int iPt = 0; iPt < nPt; iPt++)
            {
                for (const std::string& vn : item._ValNames)
                {
                    auto& col = phy->_nodeRes[vn];
                    ofs << (iPt < (int)col.size() ? col[iPt] : 0.0) << " ";
                }
            }
            ofs << std::endl;
            ofs << "        </DataArray>" << std::endl;
        }
        ofs << "      </PointData>" << std::endl;

        ofs << "      <CellData>" << std::endl;
        for (ResItem& item : _femData->_prePostConfig._eleResItems)
        {
            if (item._iFields.empty()) continue;
            PhyFieldData* phy = _femData->_phyDatas[item._iFields[0]];
            ofs << "        <DataArray type=\"Float64\" Name=\"" << item._name
                << "\" NumberOfComponents=\"" << item._ValNames.size() << "\" format=\"ascii\">" << std::endl;
            ofs << "          ";
            for (int iEle = 0; iEle < nEle; iEle++)
            {
                for (const std::string& vn : item._ValNames)
                {
                    auto& col = phy->_elemRes[vn];
                    ofs << (iEle < (int)col.size() ? col[iEle] : 0.0) << " ";
                }
            }
            ofs << std::endl;
            ofs << "        </DataArray>" << std::endl;
        }
        ofs << "      </CellData>" << std::endl;

        // 关闭标签
        ofs << "    </Piece>" << std::endl;
        ofs << "  </UnstructuredGrid>" << std::endl;
        ofs << "</VTKFile>" << std::endl;
        return 0;
    }

    int vtkPost::writePVD(const std::string& fn)
    {
        std::ofstream ofs(fn);
        if (!ofs.is_open()) return -1;
        ofs << "<?xml version=\"1.0\"?>" << std::endl;
        ofs << "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\">" << std::endl;
        ofs << "  <Collection>" << std::endl;
        for (auto& s : _steps)
        {
            std::ostringstream vf;
            vf << _baseName << "_" << std::setw(4) << std::setfill('0') << s.first << ".vtu";
            ofs << "    <DataSet timestep=\"" << s.second << "\" part=\"0\" file=\"" << vf.str() << "\"/>" << std::endl;
        }
        ofs << "  </Collection>" << std::endl;
        ofs << "</VTKFile>" << std::endl;
        return 0;
    }
}
