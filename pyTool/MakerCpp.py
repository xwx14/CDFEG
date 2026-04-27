from MakerBase import MakerBase
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
import os
import uuid
class MakerCpp(MakerBase):
    """
    C++ 代码生成器

    根据 DataProject 配置自动生成 C++ 有限元程序代码，
    支持生成可执行文件、动态库和静态库。

    作者：系统自动生成
    日期：2025-01-06
    """

    def __init__(self, project: DataProject, output_path: str, iProgramType: int = 0):
        """
        初始化 C++ 代码生成器

        Args:
            project: DataProject 项目配置对象
            output_path: 输出路径
            iProgramType: 程序类型
                0 - 可执行文件（Executable）
                1 - 动态库（Shared Library）
                2 - 静态库（Static Library）
        """
        self.project = project
        self.output_path = output_path
        os.makedirs(output_path, exist_ok=True)
        self.iProgramType = iProgramType
        self.bNeedMain=self.iProgramType == 0

        # 文件列表跟踪
        self.hFiles = []  # 生成的头文件列表
        self.cppFiles = []  # 生成的 cpp 文件列表

        # 初始化基类（会调用 _build_render_data()）
        super().__init__()
        # 确保输出目录存在
        os.makedirs(output_path, exist_ok=True)

    def _build_render_data(self):
        """
        初始化项目数据

        为每个场添加必要的辅助信息。
        """
        # 为每个场添加辅助信息
        for idx, field in enumerate(self.project.fields):
            # 提取场数据（包括 dispNames）
            field.makeData()
            # 修正索引（C++ 数组从 0 开始）
            field.index = idx
            # 设置可选属性（如果未设置）
            if not hasattr(field, 'dof2') or field.dof2 is None:
                field.dof2 = len(field.dispNames)
            if not hasattr(field, 'headerGuard') or field.headerGuard is None:
                field.headerGuard = f"{field.name.upper()}_FIELD_DATA_H"
            if not hasattr(field, 'eleResNames') or not field.eleResNames:
                field.eleResNames = ["stress", "strain"]

    def _get_cmake_target_type(self) -> str:
        """
        获取 CMake 目标类型
        Returns:
            "exec", "shared", 或 "static"
        """
        if self.iProgramType == 0:
            return "exec"
        elif self.iProgramType == 1:
            return "shared"
        else:
            return "static"

    def _get_program_type_name(self) -> str:
        """
        获取程序类型名称
        Returns:
            程序类型名称字符串
        """
        types = {
            0: "Executable",
            1: "Shared Library",
            2: "Static Library"
        }
        return types.get(self.iProgramType, "Executable")

    # ========== 文件生成方法框架 ==========

    def makeMain(self):
        """
        生成主程序文件（main.cpp）
        仅在 iProgramType=0（可执行文件）时生成
        """
        if not self.bNeedMain:
            print("⚠️  跳过 main.cpp 生成（当前程序类型不需要 main 函数）")
            return

        output_filename = "main.cpp"
        context = {
            "femDataClassName": self.femDataClassName,
            "project": self.project
        }
        self.write2File("main.cpp.j2", output_filename, context, output_path=self.output_path)
        self.cppFiles.append(output_filename)  # 添加到文件列表

    def makeFEMData(self):
        """
        生成 FEMData 派生类文件（.h 和 .cpp）

        FEMData 是项目级别的全局数据类，包含所有场的信息。
        此方法应该只调用一次，生成整个项目的 FEMData 类。
        """
        # 统一的上下文
        context = {
            "femDataClassName": self.femDataClassName,
            "headerGuard": f"{self.project.name.upper()}_DATA_H",
            "project": self.project.toDict()
        }
        # 头文件
        h_filename = f"{self.project.name}Data.h"
        self.write2File("femdata.h.j2", h_filename, context, output_path=self.output_path)
        self.hFiles.append(h_filename)
        # 实现文件
        cpp_filename = f"{self.project.name}Data.cpp"
        self.write2File("femdata.cpp.j2", cpp_filename, context, output_path=self.output_path)
        self.cppFiles.append(cpp_filename)

    def makePhyFieldData(self, field):
        """
        生成 PhyFieldData 派生类文件（.h 和 .cpp）
        Args:
            field: DataField 场对象
        """
        # 统一的上下文
        context = {
            "field": field,
            "headerGuard": f"{field.name.upper()}_FIELD_DATA_H",
            "femDataClassName": self.femDataClassName
        }
        # 头文件
        h_filename = f"{field.name}FieldData.h"
        self.write2File("phyfielddata.h.j2", h_filename, context, output_path=self.output_path)
        self.hFiles.append(h_filename)
        # 实现文件
        cpp_filename = f"{field.name}FieldData.cpp"
        self.write2File("phyfielddata.cpp.j2", cpp_filename, context, output_path=self.output_path)
        self.cppFiles.append(cpp_filename)

    def makeEleSub(self, field, ele):
        """
        生成单元子程序文件（.h 和 .cpp）
        Args:
            field: DataField 场对象
            ele: DataEleSub 单元对象
        """
        # 判断单元基类类型
        base_class = self._get_ele_base_class(ele)
        # 构建基类构造参数
        
        # 统一的上下文
        context = {
            "ele": ele.toDict(),
            "femDataClassName":self.femDataClassName,
            "field": field,  # 传递 field 对象，模板中使用 field.fieldDataClassName
            "baseClass": base_class,
            "headerGuard": f"{ele.name.upper()}_H",
            "baseClassParam":f"{ele.nNodes}, pData"
        }
        # 头文件
        h_filename = f"{ele.name}.h"
        self.write2File("elesub.h.j2", h_filename, context, output_path=self.output_path)
        self.hFiles.append(h_filename)
        # 实现文件
        cpp_filename = f"{ele.name}.cpp"
        self.write2File("elesub.cpp.j2", cpp_filename, context, output_path=self.output_path)
        self.cppFiles.append(cpp_filename)

    def makeCMakeLists(self):
        """
        生成 CMakeLists.txt 文件

        使用已收集的文件列表（self.hFiles 和 self.cppFiles）生成构建配置。
        """
        context = {
            "project": self.project,
            "projectName":self.project.name,
            "iProgramType":self.iProgramType,
            "hFiles": self.hFiles,
            "cppFiles": self.cppFiles
        }
        self.write2File("cmake.j2", "CMakeLists.txt", context, output_path=self.output_path)

    def makeVSPro(self):
        """
        生成 Visual Studio 项目文件

        生成以下文件（可添加到已有解决方案中）：
        - {projectName}.vcxproj: Visual Studio 项目文件
        - {projectName}.vcxproj.filters: 文件过滤器配置
        """
        # 生成项目 GUID（保持一致）
        project_guid = str(uuid.uuid4()).upper()
        # 转换为 VS 格式 {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
        project_guid = "{" + project_guid[:8] + "-" + project_guid[8:12] + "-" + project_guid[12:16] + "-" + project_guid[16:20] + "-" + project_guid[20:] + "}"

        # 根据程序类型确定配置
        config_type_map = {
            0: ("Application", "Console", "_CONSOLE", "CDFEG.lib"),
            1: ("DynamicLibrary", "Windows", "_CONSOLE", "CDFEG.lib"),
            2: ("StaticLibrary", "Windows", "_CONSOLE", "")
        }
        configuration_type, sub_system, preprocessor_defs, additional_deps = config_type_map.get(self.iProgramType, config_type_map[0])

        # 生成上下文
        context = {
            "projectName": self.project.name,
            "projectGuid": project_guid,
            "configurationType": configuration_type,
            "subSystem": sub_system,
            "preprocessorDefinitions": preprocessor_defs,
            "additionalDependencies": additional_deps,
            "hFiles": self.hFiles,
            "cppFiles": self.cppFiles
        }

        # 生成 vcxproj 文件
        vcxproj_filename = f"{self.project.name}.vcxproj"
        self.write2File("vcxproj.j2", vcxproj_filename, context, output_path=self.output_path)

        # 生成 vcxproj.filters 文件
        filters_filename = f"{self.project.name}.vcxproj.filters"
        self.write2File("vcxproj.filters.j2", filters_filename, context, output_path=self.output_path)

        print(f"🎨 Visual Studio 项目文件生成完成！")

    def makeAll(self):
        """
        生成所有文件
        这是一个便捷方法，一次性生成所有必要的 C++ 文件
        """
        # 清空文件列表（防止重复运行时累积）
        self.hFiles.clear()
        self.cppFiles.clear()

        self.femDataClassName=f"{self.project.name}Data"
        print(f"\n🚀 开始生成 C++ 代码...")
        print(f"📁 输出路径: {self.output_path}")
        print(f"📦 项目名称: {self.project.name}")
        print(f"🔧 程序类型: {self._get_program_type_name()}")
        print(f"📊 场数量: {len(self.project.fields)}")
        # 生成主程序（如果需要）
        if self.bNeedMain:
            self.makeMain()
        # 生成全局 FEMData 类（只生成一次，包含所有场）
        print(f"\n📝 生成全局 FEMData 类...")
        self.makeFEMData()
        # 为每个场生成 PhyFieldData 和单元文件
        for field in self.project.fields:
            print(f"\n📝 生成场 '{field.name}' 的文件...")
            self.makePhyFieldData(field)
            # 为场中的每个单元生成文件
            for ele in field.eleSubs:
                self.makeEleSub(field, ele)
        # 生成 CMakeLists.txt
        print(f"\n📄 生成构建配置...")
        self.makeCMakeLists()
        # 生成 Visual Studio 项目文件
        print(f"\n🎨 生成 Visual Studio 项目文件...")
        self.makeVSPro()
        print(f"\n✅ 代码生成完成！")
    # ========== 辅助方法 ==========
    def _get_ele_base_class(self, ele: DataEleSub) -> str:
        """
        根据单元类型返回基类名称
        Args:
            ele: DataEleSub 对象

        Returns:
            基类名称："IsoEleBase" 或 "EleSubBase"
        """
        if ele.baseClass!="":
            return ele.baseClass

        # 这里可以根据实际情况调整
        if ele.type >= 2:
            return "IsoEleBase"
        else:
            return "EleSubBase"
