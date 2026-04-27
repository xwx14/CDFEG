#include "vtkPost.h"
#include <fstream>
#include "FemData.h"
namespace CDFEG {
	vtkPost::vtkPost(FEMData* data, PhyFieldData* fieldData):Processor(data, fieldData)
	{

	}

	vtkPost::~vtkPost()
	{
	}
	int vtkPost::post(int it) {
		if (it == 0)writeVTK("result.vtk");

		return 0;
	}

	int vtkPost::writeVTK(const std::string& fn)
	{
		std::ofstream ofs(fn);
		if (!ofs.is_open())return -1;
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
			//ofs << _femData->_nodes[3 * iPt] << " " << _femData->_nodes[3 * iPt + 1] << " " << _femData->_nodes[3 * iPt + 2];
			for(int iDim = 0; iDim < dim; iDim++)
			{
				ofs << _femData->_nodes[dim * iPt + iDim] << " ";
			}
			for (int iDim = dim; iDim < 3; iDim++)
			{
				ofs << 0.0 << " ";
			}
			if (iPt < nPt - 1) ofs << " ";
		}
		ofs << std::endl;
		ofs << "        </DataArray>" << std::endl;
		ofs << "      </Points>" << std::endl;

		// 输出单元连接关系
		ofs << "      <Cells>" << std::endl;

		// connectivity
		ofs << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << std::endl;
		ofs << "          ";
		for (size_t i = 0; i < _femData->_eleNodes.size(); i++)
		{
			ofs << _femData->_eleNodes[i];
			if (i < _femData->_eleNodes.size() - 1) ofs << " ";
		}
		ofs << std::endl;
		ofs << "        </DataArray>" << std::endl;

		// offsets
		ofs << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << std::endl;
		ofs << "          ";
		for (int iEle = 0; iEle < nEle; iEle++)
		{
			ofs << _femData->_elePt[iEle + 1];
			if (iEle < nEle - 1) ofs << " ";
		}
		ofs << std::endl;
		ofs << "        </DataArray>" << std::endl;

		// types
		ofs << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">" << std::endl;
		ofs << "          ";
		
		for (int iEle = 0; iEle < nEle; iEle++)
		{
			ofs << static_cast<int>(_femData->_eleTypes[iEle]);
			if (iEle < nEle - 1) ofs << " ";
		}
		ofs << std::endl;
		ofs << "        </DataArray>" << std::endl;
		ofs << "      </Cells>" << std::endl;

		// 输出节点数据
		ofs << "      <PointData>" << std::endl;
		for (PhyFieldData* p : _femData->_phyDatas)
		{
			for (const std::string& name : p->_dispNames) {
				ofs << "        <DataArray type=\"Float64\" Name=\"" <<name << "\" format=\"ascii\">" << std::endl;
				ofs << "          ";
				for (int iPt = 0; iPt < nPt; iPt++)
				{

					ofs << p->_nodeRes[name][iPt];
					if (iPt < nPt - 1) ofs << " ";
				}
				ofs << std::endl;
				ofs << "        </DataArray>" << std::endl;
			}
		}
		ofs << "      </PointData>" << std::endl;
		//todo:  输出单元数据
		
		// 关闭标签
		ofs << "    </Piece>" << std::endl;
		ofs << "  </UnstructuredGrid>" << std::endl;
		ofs << "</VTKFile>" << std::endl;

		return 0;
	}

}

