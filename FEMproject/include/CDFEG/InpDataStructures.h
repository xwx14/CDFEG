#ifndef INPDATASTRUCTURES_H
#define INPDATASTRUCTURES_H

#include <string>
#include <array>
#include <vector>
#include <map>
// 节点集结构 (用于荷载和边界条件)
struct InpNodeSet {
	std::string name;
    std::string partName;
	std::string instanceName;    // 所属的Instance (如果有)
	std::vector<int> nodeIds;
	InpNodeSet(const std::string& setName) : name(setName) {}
    InpNodeSet() {}
};
// Element Set结构
struct InpElementSet {
	std::string name;
    bool bPart;
	std::vector<int> elementIds;
    
	std::string partName;  
    std::string instanceName;
	bool isInternal;          // 是否为内部集合
    InpElementSet() {};
	InpElementSet(const std::string& name_)
		: name(name_), isInternal(false) {
	}
};
// 节点结构
struct InpNode {
    int id;
    double x, y, z;
    InpNode(int id_, double x_, double y_, double z_ = 0.0)
        : id(id_), x(x_), y(y_), z(z_) {}
};

// 单元结构
struct InpElement {
    int id;
    std::string type;
    std::vector<int> nodeIds;
    int materialId;
    InpElement(int id_, const std::string& type_)
        : id(id_), type(type_), materialId(-1) {}
};

// 材料结构
struct InpMaterial {
    std::string name;
    std::map<std::string, double> properties;
    InpMaterial() {};
    InpMaterial(const std::string& name_)
        : name(name_) {}
};

// 连接器行为结构
struct ConnectorBehavior {
    std::string name;
    std::map<int, double> elasticity;  // component -> stiffness
    std::map<int, double> damping;    // component -> damping coefficient
    ConnectorBehavior(const std::string& name_) : name(name_) {}
};

// 梁截面结构
struct BeamSection {
    std::string elementSet;
    std::string material;
    std::string sectionType;  // I, CIRCULAR, etc.
    std::vector<double> dimensions;
    std::vector<double> dir;
    BeamSection() {}
};

// Part部件结构 (包含节点和单元的定义)
struct InpPart {
    std::string name;
    std::vector<InpNode> nodes;
    std::vector<InpElement> elements;
    std::vector<BeamSection> beamSections;
    std::vector<ConnectorBehavior> connectorBehaviors;
    std::map<std::string, InpNodeSet> nodeSets;
    std::map<std::string, InpElementSet> elementSets;
    InpPart(const std::string& name_) : name(name_) {}
    InpPart()  {}
};

// Instance装配实例结构 (Part的实例化，包含坐标变换)
struct InpInstance {
    std::string name;
    std::string partName;  // 引用的Part名称
    double transform[16]; // 4x4齐次变换矩阵
    std::map<int,int> nodeIdMap;// 节点ID映射,key为part上的节点ID,value为有限元模型中的节点ID
    std::map<int,int> elementIdMap;// 单元ID映射,key为part上的单元ID,value为有限元模型中的单元ID

    InpInstance(const std::string& name_, const std::string& partName_)
        : name(name_), partName(partName_) {
        // 初始化为单位矩阵
        for (int i = 0; i < 16; ++i) {
            transform[i] = (i % 5 == 0) ? 1.0 : 0.0; // 对角线为1
        }
    }

    // 设置平移变换
    void setTranslation(double tx, double ty, double tz) {
        transform[3] = tx;   // X平移
        transform[7] = ty;   // Y平移
        transform[11] = tz;  // Z平移
    }
};

// Section结构 (将材料分配给Element Set)
struct InpSection {
    std::string elementSetName;
    std::string materialName;
    std::string sectionType;   // "SOLID", "BEAM", "SHELL"等
    std::vector<double> dimensions;  // 截面参数

    InpSection() {}
};

// ================== 步、荷载、边界条件数据结构 ==================

// 分析步类型枚举
enum class StepType {
    STATIC,
    DYNAMIC,
    FREQUENCY,
    STEADY_STATE_DYNAMICS,
    BUCKLE,
    HEAT_TRANSFER,
    MASS_DIFFUSION
};

// 分析步结构
struct InpStep {
    std::string name;
    StepType type;
    std::map<std::string, std::string> parameters;  // nlgeom, perturbation等
    std::vector<int> loadCases;                    // 该步包含的荷载工况
    std::vector<int> boundaryConditions;           // 该步包含的边界条件

    InpStep(const std::string& stepName, StepType stepType)
        : name(stepName), type(stepType) {}
};

// 荷载类型枚举
enum class LoadType {
    CONCENTRATED_FORCE,  // Cload - 集中力
    DISTRIBUTED_LOAD,    // Dload - 分布力
    PRESSURE,            // Pressure - 压力
    GRAVITY,            // Gravity - 重力
    BODY_FORCE,          // Body Force - 体力
    TEMPERATURE         // Temperature - 温度荷载
};

// 幅值类型枚举
enum class AmplitudeType {
    TABULAR,     // 表格形式
    EQUALLY_SPACED,  // 等间距
    MODULATED,   // 调制
    DECAY,       // 衰减
    SOLUTION_DEPENDENT,  // 解相关
    USER         // 用户定义
};

// 幅值结构
struct InpAmplitude {
    std::string name;
    AmplitudeType type;
    std::vector<std::array<double,2>> dataPoints;  // 时间-值对
    std::map<std::string, double> parameters; // 其他参数如固定间隔等

    InpAmplitude(const std::string& ampName)
        : name(ampName), type(AmplitudeType::TABULAR) {}
};

// 荷载结构
struct InpLoad {
    int id;
    std::string name;
    LoadType type;
    std::string nodeSetName;     // 施加荷载的节点集
    int dof;                     // 自由度 (1=UX, 2=UY, 3=UZ, 4=URX, 5=URY, 6=URZ)
    double magnitude;            // 荷载大小
    std::map<std::string, double> components;  // 分量 (如x, y, z方向的力)
    std::string amplitudeName;   // 关联的幅值名称
    int stepId;                  // 所属分析步

    InpLoad(int loadId, const std::string& loadName, LoadType loadType)
        : id(loadId), name(loadName), type(loadType), dof(0), magnitude(0.0), stepId(-1) {}
};
//int lineSize = _line.size();
//lineSize + 1

// 边界条件类型枚举
enum class BoundaryType {
    DISPLACEMENT,        // 位移约束
    VELOCITY,           // 速度约束
    ACCELERATION,       // 加速度约束
    ROTATION,           // 转角约束
    ANGULAR_VELOCITY,  // 角速度约束
    TEMPERATURE,       // 温度约束
    EQUATION           // 方程约束
};

// 边界条件结构
struct InpBoundary {
    int id;
    std::string name;
    BoundaryType type;
    std::string nodeSetName;     // 施加约束的节点集
    std::map<int,double> components;
    std::map<std::string, double> components2;
    int sStepId;                  // 所属分析步

    InpBoundary(int boundaryId, const std::string& boundaryName, BoundaryType boundaryType)
        : id(boundaryId), name(boundaryName), type(boundaryType), sStepId(-1) {}
};


#endif // INPDATASTRUCTURES_H