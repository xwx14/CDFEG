#ifndef INPREADER_H
#define INPREADER_H

#include "CDFEG.h"
#include "Processor.h"
#include "TextReader.h"
#include "InpDataStructures.h"
#include <string>
#include <vector>
#include <map>

namespace CDFEG {

// 错误返回码定义
enum class InpReaderError {
    SUCCESS = 0,
    FILE_ERROR = -1,
    PARSE_ERROR = -2
};

class CDFEG_API inpReader : public Processor
{
public:
    inpReader(FEMData* data, PhyFieldData* fieldData);
    virtual ~inpReader();

    // 设置方法
    void setFilePath(const std::string& filePath);
    void setLogLevel(int level);

    // 主解析方法
    virtual int pre() override;

    // 访问器方法
    const std::vector<InpNode>& getNodes() const { return _nodes; }
    const std::vector<InpElement>& getElements() const { return _elements; }
    // std::map<std::string, InpMaterial>
    const std::map<std::string, InpMaterial>& getMaterials() const { return _materials; }
    // std::map<std::string, InpNodeSet>
    const std::map<std::string, InpNodeSet>& getNodeSets() const { return _nodeSets; }
    // std::map<std::string, InpElementSet>
    const std::map<std::string, InpElementSet>& getElementSets() const { return _elementSets; }
    const std::vector<InpBoundary>& getBoundaries() const { return _boundaries; }
    // std::map<std::string, InpPart>
    const std::map<std::string, InpPart>& getParts() const { return _parts; }
    const std::vector<InpInstance>& getInstances() const { return _instances; }
    bool hasPartAssembly() const { return _hasPartAssembly; }

private:
    // 解析方法声明
    void parseNode();
    void parseElement();
    void parseMaterial();
    void parseElastic();
    void parseNset();
    void parseElset();
    void parseBoundary();
    void parsePart();
    void parseInstance();
    void parseSection();
    void parseSurface();

    // 辅助方法
    void assembleInstances();
    void convertToFEMData();
    void logWarning(const std::string& message);
    std::string readContinuationLines();

private:
    // 基础成员
    std::string _filePath;
    TextReader _reader;
    bool _hasPartAssembly;
    int _logLevel;

    // 全局数据存储
    std::vector<InpNode> _nodes;
    std::vector<InpElement> _elements;
    std::map<std::string, InpNodeSet> _nodeSets;
    std::map<std::string, InpElementSet> _elementSets;
    std::map<std::string, InpMaterial> _materials;
    std::vector<InpSection> _sections;
    std::vector<InpStep> _steps;
    std::vector<InpLoad> _loads;
    std::vector<InpBoundary> _boundaries;
    std::map<std::string, InpPart> _parts;
    std::vector<InpInstance> _instances;

    // 解析状态
    std::string _currentPartName;
    std::string _currentMaterialName;
    std::string _currentInstanceName;
};

} // namespace CDFEG

#endif // INPREADER_H
