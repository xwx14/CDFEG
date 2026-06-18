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
# MERCHANTABILITY or FITNESS A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.

from MakerBase import MakerBase, safePrint
from DataProject import DataProject
from DataField import DataField
from DataEleSub import DataEleSub
import os
import shutil

CDFEG_LIB_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "FEMproject", "CDFEG")
THIRD_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "FEMproject", "third")
# 两种模式的使用示例：
# # 模式1：生成新解决方案
# maker = MakerCpp(project, "MySolution", mode='new')
# maker.makeAll()
# # 输出结构：
# # MySolution/
# # ├── CMakeLists.txt          (解决方案级)
# # ├── CDFEG/                  (基础库)
# # ├── third/                  (依赖)
# # └── Truss2D/                (项目代码)
# #     ├── CMakeLists.txt
# #     ├── main.cpp
# #     └── ...
# # 模式2：添加到现有项目
# maker = MakerCpp(project, "sample/Truss2D", mode='add', sln_cmake_path="FEMproject/CMakeLists.txt")
# maker.makeAll()
# # 在 FEMproject/CMakeLists.txt 末尾追加: add_subdirectory(sample/Truss2D)
# mode='new' 保持向后兼容，仍是默认行为。现有的测试代码 MakerCpp(project, outPath) 无需修改即可正常工作。

class MakerCpp(MakerBase):
    """
    C++ 代码生成器

    根据 DataProject 配置自动生成 C++ 有限元程序代码，
    支持生成可执行文件、动态库和静态库。

    支持多项目生成，提供两种模式：
    - 生成新解决方案（mode='new'）：创建完整文件夹，包含 CDFEG 库、third 和解决方案 CMake
    - 添加到现有项目（mode='add'）：仅生成项目文件，在现有解决方案 CMake 中追加 add_subdirectory
    """

    def __init__(self, projects, output_path: str, mode: str = 'new', sln_cmake_path: str = None, sln_name: str = None):
        """
        初始化 C++ 代码生成器

        Args:
            projects: 项目配置，支持两种形式：
                - 单个 DataProject 对象（向后兼容，默认 iProgramType=0）
                - 列表，每个元素为 (DataProject, iProgramType) 元组
                  或 DataProject 对象（默认 iProgramType=0）
            output_path: 输出路径
            mode: 生成模式
                'new' - 生成新解决方案
                'add' - 添加到现有项目
            sln_cmake_path: 现有解决方案 CMakeLists.txt 的路径（mode='add' 时必须提供）
            sln_name: 解决方案名称（mode='new' 时使用，默认取第一个项目名称）
        """
        if isinstance(projects, DataProject):
            self.projects = [(projects, 0)]
        else:
            self.projects = []
            for item in projects:
                if isinstance(item, DataProject):
                    self.projects.append((item, 0))
                elif isinstance(item, (list, tuple)) and len(item) == 2:
                    self.projects.append((item[0], item[1]))
                else:
                    raise ValueError(f"无效的项目配置: {item}")

        self.output_path = output_path
        self.mode = mode
        self.sln_cmake_path = sln_cmake_path
        self.sln_name = sln_name if sln_name else self.projects[0][0].name
        # 主函数形式，0，使用makeData生成数据；1，使用GiD数据文件
        self.mainMode=0

        if mode == 'new':
            self.sln_dir = output_path
        else:
            self.sln_dir = None

        super().__init__()

    def _get_project_output_path(self, project) -> str:
        if self.mode == 'new':
            return os.path.join(self.output_path, project.name)
        else:
            return self.output_path

    @staticmethod
    def _get_program_type_name(iProgramType: int) -> str:
        types = {0: "Executable", 1: "Shared Library", 2: "Static Library"}
        return types.get(iProgramType, "Executable")

    # ========== 单项目内部生成方法 ==========

    def _makeMain(self, project, output_path: str, iProgramType: int, file_lists: dict):
        if iProgramType != 0:
            return
        femDataClassName = f"{project.name}Data"
        context = {
            "femDataClassName": femDataClassName,
            "project": project
        }
        if self.mainMode==0:
            self.write2File("main.cpp.j2", "main.cpp", context, output_path=output_path)
        elif self.mainMode==1:
            self.write2File("mainGid.cpp.j2", "main.cpp", context, output_path=output_path)
        file_lists["cpp"].append("main.cpp")

    def _makeFEMData(self, project, output_path: str, file_lists: dict):
        femDataClassName = f"{project.name}Data"
        project.caculateCode=self.parseCmds(project)
        context = {
            "femDataClassName": femDataClassName,
            "headerGuard": f"{project.name.upper()}_DATA_H",
            "project": project.toDict(),

        }
        h_filename = f"{project.name}Data.h"
        self.write2File("femdata.h.j2", h_filename, context, output_path=output_path)
        file_lists["h"].append(h_filename)
        cpp_filename = f"{project.name}Data.cpp"
        self.write2File("femdata.cpp.j2", cpp_filename, context, output_path=output_path)
        file_lists["cpp"].append(cpp_filename)

    def _makePhyFieldData(self, project, field, output_path: str, file_lists: dict):
        femDataClassName = f"{project.name}Data"
        context = {
            "field": field,
            "headerGuard": f"{field.name.upper()}_FIELD_DATA_H",
            "femDataClassName": femDataClassName
        }
        h_filename = f"{field.name}FieldData.h"
        self.write2File("phyfielddata.h.j2", h_filename, context, output_path=output_path)
        file_lists["h"].append(h_filename)
        cpp_filename = f"{field.name}FieldData.cpp"
        self.write2File("phyfielddata.cpp.j2", cpp_filename, context, output_path=output_path)
        file_lists["cpp"].append(cpp_filename)
    def _replaceCoordVars(self, shapeFun: str, coordVars: list) -> str:
        for i, var in enumerate(coordVars):
            shapeFun = shapeFun.replace(f"x[{i+1}]", var)
        return shapeFun
    def _makeEleSub(self, project, field, ele, output_path: str, file_lists: dict):
        base_class = self._get_ele_base_class(ele)
        femDataClassName = f"{project.name}Data"
        if ele.vtkCellType is None:
            ele.inferVTKCellType()
        shapefuns = [self._replaceCoordVars(x, project.coordVars) for x in ele.shapeFuns] if hasattr(ele, 'shapeFuns') else []
        context = {
            "ele": ele.toDict(),
            "project": project,
            "femDataClassName": femDataClassName,
            "field": field,
            "baseClass": base_class,
            "headerGuard": f"{ele.name.upper()}_H",
            "baseClassParam": f"{ele.nNodes}, pData",
            "dim": project.dim,
            "shapeFuns": shapefuns
        }
        h_filename = f"{ele.name}.h"
        self.write2File("elesub.h.j2", h_filename, context, output_path=output_path)
        file_lists["h"].append(h_filename)
        cpp_filename = f"{ele.name}.cpp"
        self.write2File("elesub.cpp.j2", cpp_filename, context, output_path=output_path)
        file_lists["cpp"].append(cpp_filename)

    def _makeCMakeLists(self, project, output_path: str, iProgramType: int, file_lists: dict):
        context = {
            "project": project,
            "projectName": project.name,
            "iProgramType": iProgramType,
            "hFiles": file_lists["h"],
            "cppFiles": file_lists["cpp"]
        }
        self.write2File("cmake.j2", "CMakeLists.txt", context, output_path=output_path)

    # ========== 多项目 / 解决方案级方法 ==========

    def _build_render_data(self, project):
        for idx, field in enumerate(project.fields):
            field.makeData()
            field.index = idx
            if not hasattr(field, 'dof2') or field.dof2 is None:
                field.dof2 = len(field.dispNames)
            if not hasattr(field, 'headerGuard') or field.headerGuard is None:
                field.headerGuard = f"{field.name.upper()}_FIELD_DATA_H"
            if not hasattr(field, 'eleResNames') or not field.eleResNames:
                field.eleResNames = ["stress", "strain"]

    def makeProject(self, project, output_path: str, iProgramType: int):
        """
        生成单个项目的全部文件

        Args:
            project: DataProject 项目对象
            output_path: 项目输出路径
            iProgramType: 程序类型 (0=可执行, 1=动态库, 2=静态库)
        """
        os.makedirs(output_path, exist_ok=True)
        self._build_render_data(project)

        file_lists = {"h": [], "cpp": []}

        if iProgramType == 0:
            self._makeMain(project, output_path, iProgramType, file_lists)
        safePrint(f"\n📝 生成全局 FEMData 类...")
        self._makeFEMData(project, output_path, file_lists)
        for field in project.fields:
            safePrint(f"\n📝 生成场 '{field.name}' 的文件...")
            self._makePhyFieldData(project, field, output_path, file_lists)
            for ele in field.eleSubs:
                self._makeEleSub(project, field, ele, output_path, file_lists)

        safePrint(f"\n📄 生成项目 CMakeLists.txt...")
        self._makeCMakeLists(project, output_path, iProgramType, file_lists)

    def makeSlnCMake(self):
        """
        生成解决方案级 CMakeLists.txt
        """
        context = {
            "slnName": self.sln_name,
            "projects": [{"dir": p.name} for p, _ in self.projects]
        }
        self.write2File("cmakeSln.j2", "CMakeLists.txt", context, output_path=self.sln_dir)

    def _copy_cdfeg_lib(self):
        dst = os.path.join(self.sln_dir, "CDFEG")
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(CDFEG_LIB_DIR, dst)
        safePrint(f"✅ 已复制 CDFEG 库到: {dst}")

    def _copy_third(self):
        dst = os.path.join(self.sln_dir, "third")
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(THIRD_DIR, dst)
        safePrint(f"✅ 已复制 third 目录到: {dst}")

    def _add_to_existing_sln(self, project_output_path: str):
        if not self.sln_cmake_path:
            safePrint("⚠️  未提供 sln_cmake_path，跳过追加 add_subdirectory")
            return

        cmake_file = os.path.abspath(self.sln_cmake_path)
        if not os.path.isfile(cmake_file):
            safePrint(f"⚠️  解决方案 CMakeLists.txt 不存在: {cmake_file}")
            return

        sub_dir = os.path.relpath(project_output_path, os.path.dirname(cmake_file)).replace("\\", "/")
        add_line = f"add_subdirectory({sub_dir})"

        with open(cmake_file, 'r', encoding='utf-8') as f:
            content = f.read()

        if add_line in content:
            safePrint(f"⚠️  add_subdirectory({sub_dir}) 已存在，跳过追加")
            return

        with open(cmake_file, 'a', encoding='utf-8') as f:
            f.write(f"\n{add_line}\n")
        safePrint(f"✅ 已追加 add_subdirectory({sub_dir}) 到: {cmake_file}")
    def parseCmds(self,project):
        """
        解析求解步骤命令列表，生成计算代码字符串

        Args:
            project: DataProject 项目对象

        Returns:
            生成的计算代码字符串
        """
        code_lines=""
        for cmd in project.cmds:
            pIndex = cmd[1]
            phyClassName = project.fields[pIndex].fieldDataClassName 
            dict0={"PhyClassName":phyClassName,"PhyIndex":pIndex}
            if len(cmd)>2:
                for i in range(2,len(cmd)):
                    pIndex2=cmd[i]
                    phyClassName2 = project.fields[pIndex2].fieldDataClassName
                    dict0[f"PhyClassName{i}"]=phyClassName2
                    dict0[f"PhyIndex{i}"]=pIndex2
            code_lines += self.write2String(f"{cmd[0]}.cmd.j2", dict0) + "\n"
        return code_lines

    def makeAll(self):
        """
        生成所有项目的全部文件
        """
        safePrint(f"\n🚀 开始生成 C++ 代码...")
        safePrint(f"📦 项目数量: {len(self.projects)}")
        safePrint(f"📋 生成模式: {'新解决方案' if self.mode == 'new' else '添加到现有项目'}")

        if self.mode == 'new':
            safePrint(f"\n📦 复制 CDFEG 基础库...")
            self._copy_cdfeg_lib()
            safePrint(f"\n📦 复制 third 依赖目录...")
            self._copy_third()

        for project, iProgramType in self.projects:
            project_output_path = self._get_project_output_path(project)
            safePrint(f"\n{'='*50}")
            safePrint(f"📦 项目名称: {project.name}")
            safePrint(f"📁 输出路径: {project_output_path}")
            safePrint(f"🔧 程序类型: {self._get_program_type_name(iProgramType)}")
            safePrint(f"📊 场数量: {len(project.fields)}")
            safePrint(f"{'='*50}")
            self.makeProject(project, project_output_path, iProgramType)

            if self.mode == 'add':
                self._add_to_existing_sln(project_output_path)

        if self.mode == 'new':
            safePrint(f"\n📄 生成解决方案 CMakeLists.txt...")
            self.makeSlnCMake()

        safePrint(f"\n✅ 代码生成完成！")

    # ========== 辅助方法 ==========
    def _get_ele_base_class(self, ele: DataEleSub) -> str:
        if ele.baseClass != "":
            return ele.baseClass
        if ele.type >= 2:
            return "IsoEleBase"
        else:
            return "EleSubBase"
