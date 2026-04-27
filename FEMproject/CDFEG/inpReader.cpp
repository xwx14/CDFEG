#include "inpReader.h"
#include "FemData.h"
#include "PhyFieldData.h"
#include <iostream>
#include <algorithm>
#include <cctype>

namespace CDFEG {

inpReader::inpReader(FEMData* data, PhyFieldData* fieldData)
    : Processor(data, fieldData)
    , _hasPartAssembly(false)
    , _logLevel(1)
{
}

inpReader::~inpReader()
{
}

void inpReader::setFilePath(const std::string& filePath)
{
    _filePath = filePath;
    _reader.setFilePath(filePath);
}

void inpReader::setLogLevel(int level)
{
    _logLevel = level;
}

void inpReader::logWarning(const std::string& message)
{
    if (_logLevel >= 1) {
        std::cerr << "[inpReader WARNING] " << message << std::endl;
    }
}

std::string inpReader::readContinuationLines()
{
    std::string result = _reader.getCurrentLine();
    // 检查行末是否有逗号，表示数据继续到下一行
    while (!result.empty() && result.back() == ',') {
        if (!_reader.readNextLine()) {
            break;
        }
        std::string nextLine = _reader.getCurrentLine();
        // 跳过注释行
        if (nextLine.empty() || (nextLine.size() >= 2 && nextLine[0] == '*' && nextLine[1] == '*')) {
            continue;
        }
        // 如果下一行是新关键字，回退并退出
        if (!nextLine.empty() && nextLine[0] == '*' && (nextLine.size() < 2 || nextLine[1] != '*')) {
            _reader.preLine();
            break;
        }
        result += nextLine;
    }
    return result;
}

int inpReader::pre()
{
    // 打开文件
    if (!_reader.open()) {
        logWarning("Cannot open file: " + _filePath);
        return static_cast<int>(InpReaderError::FILE_ERROR);
    }

    // 主解析循环
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        
        // 跳过注释行（以**开头）
        if (line.size() >= 2 && line[0] == '*' && line[1] == '*') {
            continue;
        }
        
        // 识别关键字行（以*开头）
        if (line[0] == '*') {
            std::string lowerLine = TextReader::toLowerCase(line);
            
            // 关键字分发
            if (lowerLine.find("*node") == 0 && lowerLine.find("*nset") != 0) {
                parseNode();
            }
            else if (lowerLine.find("*element") == 0 && lowerLine.find("*elset") != 0) {
                parseElement();
            }
            else if (lowerLine.find("*material") == 0) {
                parseMaterial();
            }
            else if (lowerLine.find("*elastic") == 0) {
                parseElastic();
            }
            else if (lowerLine.find("*nset") == 0) {
                parseNset();
            }
            else if (lowerLine.find("*elset") == 0) {
                parseElset();
            }
            else if (lowerLine.find("*boundary") == 0) {
                parseBoundary();
            }
            else if (lowerLine.find("*part") == 0 && lowerLine.find("*part,") == 0) {
                _hasPartAssembly = true;
                parsePart();
            }
            else if (lowerLine.find("*instance") == 0) {
                parseInstance();
            }
            else if (lowerLine.find("*solid section") == 0) {
                parseSection();
            }
            else if (lowerLine.find("*surface") == 0) {
                parseSurface();
            }
        }
    }

    // 关闭文件
    _reader.close();

    // 如果是Part-Assembly模式，合并Instance到全局数据
    if (_hasPartAssembly) {
        assembleInstances();
    }

    // 转换为FEMData/PhyFieldData
    //convertToFEMData();

    return static_cast<int>(InpReaderError::SUCCESS);
}

void inpReader::parseNode()
{
    // 解析*NODE关键字后的数据行
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        
        // 遇到新关键字，回退并退出
        if (line[0] == '*') {
            _reader.preLine();
            break;
        }
        
        // 解析节点数据: ID, X, Y, Z
        std::vector<double> values = TextReader::splitDoubles(line, ", \t");
        if (values.size() >= 3) {
            int nodeId = static_cast<int>(values[0]);
            double x = values[1];
            double y = values[2];
            double z = (values.size() >= 4) ? values[3] : 0.0;
            
            if (_hasPartAssembly && !_currentPartName.empty()) {
                // Part-Assembly模式：存储到当前Part
                // std::map<std::string, InpPart>::iterator
                auto it = _parts.find(_currentPartName);
                if (it != _parts.end()) {
                    it->second.nodes.emplace_back(nodeId, x, y, z);
                }
            } else {
                // 简单模式：直接存储到全局
                _nodes.emplace_back(nodeId, x, y, z);
            }
        }
    }
}

void inpReader::parseElement()
{
    // 解析*ELEMENT关键字，获取TYPE参数
    std::string headerLine = _reader.getCurrentLine();
    // std::map<std::string, std::string>
    auto params = TextReader::parseInfoLine(headerLine, true);
    
    std::string elemType = "C3D8R";
    if (params.find("type") != params.end()) {
        elemType = params["type"];
        // 转换为大写
        std::transform(elemType.begin(), elemType.end(), elemType.begin(), ::toupper);
    }
    
    // 检查是否支持的单元类型
    bool supported = (elemType == "C3D8R" || elemType == "C3D8");
    if (!supported) {
        logWarning("Unsupported element type: " + elemType + ", skipping");
    }
    
    // 解析单元数据
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        
        // 遇到新关键字，回退并退出
        if (line[0] == '*') {
            _reader.preLine();
            break;
        }
        
        if (!supported) {
            continue;
        }
        
        // 处理跨行单元定义
        std::string fullLine = line;
        while (!fullLine.empty() && fullLine.back() == ',') {
            if (!_reader.readNextLine()) {
                break;
            }
            std::string nextLine = _reader.getCurrentLine();
            if (nextLine.empty() || nextLine[0] == '*') {
                _reader.preLine();
                break;
            }
            fullLine += nextLine;
        }
        
        // 解析单元数据: ID, N1, N2, N3, ...
        std::vector<int> values = TextReader::splitInts(fullLine, ", \t");
        if (values.size() >= 2) {
            int elemId = values[0];
            InpElement elem(elemId, elemType);
            for (size_t i = 1; i < values.size(); ++i) {
                elem.nodeIds.push_back(values[i]);
            }
            
            if (_hasPartAssembly && !_currentPartName.empty()) {
                // Part-Assembly模式：存储到当前Part
                // std::map<std::string, InpPart>::iterator
                auto it = _parts.find(_currentPartName);
                if (it != _parts.end()) {
                    it->second.elements.push_back(elem);
                }
            } else {
                // 简单模式：直接存储到全局
                _elements.push_back(elem);
            }
        }
    }
}

void inpReader::parseMaterial()
{
    // 解析*MATERIAL关键字，获取NAME参数
    std::string headerLine = _reader.getCurrentLine();
    // std::map<std::string, std::string>
    auto params = TextReader::parseInfoLine(headerLine, true);
    
    if (params.find("name") != params.end()) {
        _currentMaterialName = params["name"];
        _materials[_currentMaterialName] = InpMaterial(_currentMaterialName);
    }
}

void inpReader::parseElastic()
{
    // 解析*ELASTIC关键字后的数据行
    if (_currentMaterialName.empty()) {
        logWarning("*ELASTIC without preceding *MATERIAL");
        return;
    }
    
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        
        // 遇到新关键字，回退并退出
        if (line[0] == '*') {
            _reader.preLine();
            break;
        }
        
        // 解析弹性参数: E, nu
        std::vector<double> values = TextReader::splitDoubles(line, ", \t");
        if (values.size() >= 2) {
            // std::map<std::string, InpMaterial>::iterator
            auto it = _materials.find(_currentMaterialName);
            if (it != _materials.end()) {
                it->second.properties["E"] = values[0];
                it->second.properties["nu"] = values[1];
            }
        }
        break; // 只读取第一行数据
    }
}

void inpReader::parseNset()
{
    // 解析*NSET关键字
    std::string headerLine = _reader.getCurrentLine();
    // std::map<std::string, std::string>
    auto params = TextReader::parseInfoLine(headerLine, true);
    
    std::string setName;
    if (params.find("nset") != params.end()) {
        setName = params["nset"];
    }
    
    if (setName.empty()) {
        return;
    }
    
    InpNodeSet nset(setName);
    
    // 检查是否使用GENERATE选项
    bool useGenerate = (params.find("generate") != params.end());
    
    // 解析节点ID列表
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        
        // 遇到新关键字，回退并退出
        if (line[0] == '*') {
            _reader.preLine();
            break;
        }
        
        if (useGenerate) {
            // GENERATE格式: start, end, increment
            std::vector<int> values = TextReader::splitInts(line, ", \t");
            if (values.size() >= 2) {
                int start = values[0];
                int end = values[1];
                int inc = (values.size() >= 3) ? values[2] : 1;
                for (int i = start; i <= end; i += inc) {
                    nset.nodeIds.push_back(i);
                }
            }
        } else {
            // 直接列表格式
            std::vector<int> values = TextReader::splitInts(line, ", \t");
            for (int val : values) {
                nset.nodeIds.push_back(val);
            }
        }
    }
    
    if (_hasPartAssembly && !_currentPartName.empty()) {
        // Part-Assembly模式：存储到当前Part
        // std::map<std::string, InpPart>::iterator
        auto it = _parts.find(_currentPartName);
        if (it != _parts.end()) {
            it->second.nodeSets[setName] = nset;
        }
    } else {
        // 简单模式：直接存储到全局
        _nodeSets[setName] = nset;
    }
}

void inpReader::parseElset()
{
    // 解析*ELSET关键字
    std::string headerLine = _reader.getCurrentLine();
    // std::map<std::string, std::string>
    auto params = TextReader::parseInfoLine(headerLine, true);
    
    std::string setName;
    if (params.find("elset") != params.end()) {
        setName = params["elset"];
    }
    
    if (setName.empty()) {
        return;
    }
    
    InpElementSet elset(setName);
    
    // 检查是否使用GENERATE选项
    bool useGenerate = (params.find("generate") != params.end());
    
    // 解析单元ID列表
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        
        // 遇到新关键字，回退并退出
        if (line[0] == '*') {
            _reader.preLine();
            break;
        }
        
        if (useGenerate) {
            // GENERATE格式: start, end, increment
            std::vector<int> values = TextReader::splitInts(line, ", \t");
            if (values.size() >= 2) {
                int start = values[0];
                int end = values[1];
                int inc = (values.size() >= 3) ? values[2] : 1;
                for (int i = start; i <= end; i += inc) {
                    elset.elementIds.push_back(i);
                }
            }
        } else {
            // 直接列表格式
            std::vector<int> values = TextReader::splitInts(line, ", \t");
            for (int val : values) {
                elset.elementIds.push_back(val);
            }
        }
    }
    
    if (_hasPartAssembly && !_currentPartName.empty()) {
        // Part-Assembly模式：存储到当前Part
        // std::map<std::string, InpPart>::iterator
        auto it = _parts.find(_currentPartName);
        if (it != _parts.end()) {
            it->second.elementSets[setName] = elset;
        }
    } else {
        // 简单模式：直接存储到全局
        _elementSets[setName] = elset;
    }
}

void inpReader::parseBoundary()
{
    // 解析*BOUNDARY关键字后的数据
    static int boundaryId = 0;
    
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        
        // 遇到新关键字，回退并退出
        if (line[0] == '*') {
            _reader.preLine();
            break;
        }
        
        // 解析边界条件: nodeSetName, dof1, dof2 [, value]
        std::vector<std::string> parts = TextReader::split(line, ", \t");
        if (parts.size() >= 2) {
            std::string nodeSetName = parts[0];
            
            InpBoundary bc(boundaryId++, nodeSetName, BoundaryType::DISPLACEMENT);
            bc.nodeSetName = nodeSetName;
            
            // 解析自由度范围
            int dof1 = 1, dof2 = 1;
            double value = 0.0;
            
            if (parts.size() >= 2) {
                TextReader::string2Int(parts[1], dof1);
            }
            if (parts.size() >= 3) {
                TextReader::string2Int(parts[2], dof2);
            }
            if (parts.size() >= 4) {
                TextReader::string2Double(parts[3], value);
            }
            
            // 存储约束的自由度
            for (int dof = dof1; dof <= dof2; ++dof) {
                bc.components[dof] = value;
            }
            
            _boundaries.push_back(bc);
        }
    }
}

void inpReader::parsePart()
{
    // 解析*PART关键字
    std::string headerLine = _reader.getCurrentLine();
    // std::map<std::string, std::string>
    auto params = TextReader::parseInfoLine(headerLine, true);
    
    if (params.find("name") != params.end()) {
        _currentPartName = params["name"];
        _parts[_currentPartName] = InpPart(_currentPartName);
    }
    
    // 解析Part内容，直到遇到*END PART
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        // 跳过空行和注释
        if (line.empty()) {
            continue;
        }
        if (line.size() >= 2 && line[0] == '*' && line[1] == '*') {
            continue;
        }
        
        // 检查*END PART
        std::string lowerLine = TextReader::toLowerCase(line);
        if (lowerLine.find("*end part") == 0) {
            _currentPartName.clear();
            break;
        }
        
        // 处理Part内的关键字
        if (line[0] == '*') {
            if (lowerLine.find("*node") == 0 && lowerLine.find("*nset") != 0) {
                parseNode();
            }
            else if (lowerLine.find("*element") == 0 && lowerLine.find("*elset") != 0) {
                parseElement();
            }
            else if (lowerLine.find("*nset") == 0) {
                parseNset();
            }
            else if (lowerLine.find("*elset") == 0) {
                parseElset();
            }
        }
    }
}

void inpReader::parseInstance()
{
    // 解析*INSTANCE关键字
    std::string headerLine = _reader.getCurrentLine();
    // std::map<std::string, std::string>
    auto params = TextReader::parseInfoLine(headerLine, true);
    
    std::string instanceName, partName;
    if (params.find("name") != params.end()) {
        instanceName = params["name"];
    }
    if (params.find("part") != params.end()) {
        partName = params["part"];
    }
    
    if (instanceName.empty() || partName.empty()) {
        return;
    }
    
    InpInstance instance(instanceName, partName);
    _currentInstanceName = instanceName;
    
    // 解析Instance内容，直到遇到*END INSTANCE
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        // 跳过空行和注释
        if (line.empty()) {
            continue;
        }
        if (line.size() >= 2 && line[0] == '*' && line[1] == '*') {
            continue;
        }
        
        // 检查*END INSTANCE
        std::string lowerLine = TextReader::toLowerCase(line);
        if (lowerLine.find("*end instance") == 0) {
            break;
        }
        
        // 解析平移变换（非关键字行）
        if (line[0] != '*') {
            std::vector<double> values = TextReader::splitDoubles(line, ", \t");
            if (values.size() >= 3) {
                instance.setTranslation(values[0], values[1], values[2]);
            }
        }
    }
    
    _instances.push_back(instance);
    _currentInstanceName.clear();
}

void inpReader::parseSection()
{
    // 解析*SOLID SECTION关键字
    std::string headerLine = _reader.getCurrentLine();
    // std::map<std::string, std::string>
    auto params = TextReader::parseInfoLine(headerLine, true);
    
    InpSection section;
    if (params.find("elset") != params.end()) {
        section.elementSetName = params["elset"];
    }
    if (params.find("material") != params.end()) {
        section.materialName = params["material"];
    }
    section.sectionType = "SOLID";
    
    _sections.push_back(section);
}

void inpReader::parseSurface()
{
    // 解析*SURFACE关键字（简化实现，跳过数据）
    std::string headerLine = _reader.getCurrentLine();
    
    // 跳过Surface数据
    while (_reader.readNextLine()) {
        std::string line = _reader.getCurrentLine();
        
        if (line.empty()) {
            continue;
        }
        
        if (line[0] == '*') {
            _reader.preLine();
            break;
        }
    }
}

void inpReader::assembleInstances()
{
    // 合并所有Instance到全局数据
    int globalNodeId = 1;
    int globalElementId = 1;
    
    for (InpInstance& instance : _instances) {
        // std::map<std::string, InpPart>::iterator
        auto partIt = _parts.find(instance.partName);
        if (partIt == _parts.end()) {
            logWarning("Instance references unknown part: " + instance.partName);
            continue;
        }
        
        InpPart& part = partIt->second;
        
        // 获取平移变换
        double tx = instance.transform[3];
        double ty = instance.transform[7];
        double tz = instance.transform[11];
        
        // 复制节点并应用变换
        for (const InpNode& node : part.nodes) {
            int newNodeId = globalNodeId++;
            instance.nodeIdMap[node.id] = newNodeId;
            
            // 应用平移变换
            double newX = node.x + tx;
            double newY = node.y + ty;
            double newZ = node.z + tz;
            
            _nodes.emplace_back(newNodeId, newX, newY, newZ);
        }
        
        // 复制单元并更新节点ID引用
        for (const InpElement& elem : part.elements) {
            int newElemId = globalElementId++;
            instance.elementIdMap[elem.id] = newElemId;
            
            InpElement newElem(newElemId, elem.type);
            for (int nodeId : elem.nodeIds) {
                // std::map<int, int>::iterator
                auto nodeIt = instance.nodeIdMap.find(nodeId);
                if (nodeIt != instance.nodeIdMap.end()) {
                    newElem.nodeIds.push_back(nodeIt->second);
                } else {
                    newElem.nodeIds.push_back(nodeId);
                }
            }
            newElem.materialId = elem.materialId;
            
            _elements.push_back(newElem);
        }
        
        // 合并节点集
        for (const auto& nsetPair : part.nodeSets) {
            InpNodeSet globalNset(instance.name + "." + nsetPair.first);
            globalNset.instanceName = instance.name;
            for (int nodeId : nsetPair.second.nodeIds) {
                // std::map<int, int>::iterator
                auto nodeIt = instance.nodeIdMap.find(nodeId);
                if (nodeIt != instance.nodeIdMap.end()) {
                    globalNset.nodeIds.push_back(nodeIt->second);
                }
            }
            _nodeSets[globalNset.name] = globalNset;
        }
        
        // 合并单元集
        for (const auto& elsetPair : part.elementSets) {
            InpElementSet globalElset(instance.name + "." + elsetPair.first);
            globalElset.instanceName = instance.name;
            for (int elemId : elsetPair.second.elementIds) {
                // std::map<int, int>::iterator
                auto elemIt = instance.elementIdMap.find(elemId);
                if (elemIt != instance.elementIdMap.end()) {
                    globalElset.elementIds.push_back(elemIt->second);
                }
            }
            _elementSets[globalElset.name] = globalElset;
        }
    }
}

void inpReader::convertToFEMData()
{
    if (!_femData)  return;
    
    // 转换节点
    for (const InpNode& node : _nodes) {
        _femData->addNode(node.x, node.y, node.z);
        _femData->_nodeIdMap[node.id] = _femData->_nPts - 1;
    }
    _nPts = static_cast<int>(_nodes.size());
    
    // 转换单元
    for (const InpElement& elem : _elements) {
        std::vector<int> mappedNodeIds;
        for (int nodeId : elem.nodeIds) {
            // std::map<int, int>::iterator
            auto it = _femData->_nodeIdMap.find(nodeId);
            if (it != _femData->_nodeIdMap.end()) {
                mappedNodeIds.push_back(it->second);
            }
        }
        
        if (!mappedNodeIds.empty()) {
            //_femData->_eleNodes.push_back(mappedNodeIds);
            // 设置单元类型为VTK_HEXAHEDRON (12)
            //_femData->_eleTypes.push_back(12);
        }
    }
    _nEles = static_cast<int>(_elements.size());
    
    // 转换材料
    for (const auto& matPair : _materials) {
        const InpMaterial& mat = matPair.second;
        std::vector<double> params;
        // std::map<std::string, double>::const_iterator
        auto eIt = mat.properties.find("E");
        auto nuIt = mat.properties.find("nu");
        if (eIt != mat.properties.end()) {
            params.push_back(eIt->second);
        }
        if (nuIt != mat.properties.end()) {
            params.push_back(nuIt->second);
        }
        if (!params.empty()) {
            //_femData->_mateParams.push_back(params);
        }
    }
    
    // 转换边界条件到PhyFieldData
    if (_phyFieldData) {
        for (const InpBoundary& bc : _boundaries) {
            // 查找节点集
            // std::map<std::string, InpNodeSet>::iterator
            auto nsetIt = _nodeSets.find(bc.nodeSetName);
            if (nsetIt != _nodeSets.end()) {
                for (int nodeId : nsetIt->second.nodeIds) {
                    // std::map<int, int>::iterator
                    auto nodeMapIt = _femData->_nodeIdMap.find(nodeId);
                    if (nodeMapIt != _femData->_nodeIdMap.end()) {
                        int internalNodeId = nodeMapIt->second;
                        for (const auto& compPair : bc.components) {
                            int dof = compPair.first;
                            double value = compPair.second;
                            // 设置边界条件
                            //_phyFieldData->addTypeIBC(internalNodeId, dof - 1, value);
                        }
                    }
                }
            }
        }
    }
}

} // namespace CDFEG
