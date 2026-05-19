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
from DataField import DataField
from DataEleSub import DataEleSub
import os
import shutil

CDFEG_LIB_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "FEMproject", "CDFEG")
THIRD_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "FEMproject", "third")


class MakerCpp(MakerBase):
    """
    C++ 代码生成器

    根据 DataProject 配置自动生成 C++ 有限元程序代码，
    支持生成可执行文件、动态库和静态库。

    提供两种模式：
    - 生成新解决方案（mode='new'）：创建完整文件夹，包含 CDFEG 库、third 和解决方案 CMake
    - 添加到现有项目（mode='add'）：仅生成项目文件，在现有解决方案 CMake 中追加 add_subdirectory
    """

    def __init__(self, project: DataProject, output_path: str, iProgramType: int = 0, mode: str = 'new', sln_cmake_path: str = None):
        """
        初始化 C++ 代码生成器

        Args:
            project: DataProject 项目配置对象
            output_path: 输出路径
            iProgramType: 程序类型
                0 - 可执行文件（Executable）
                1 - 动态库（Shared Library）
                2 - 静态库（Static Library）
            mode: 生成模式
                'new' - 生成新解决方案（包含 CDFEG 库、third 和解决方案 CMake）
                'add' - 添加到现有项目（在现有解决方案 CMake 中追加 add_subdirectory）
            sln_cmake_path: 现有解决方案 CMakeLists.txt 的路径（mode='add' 时必须提供）
        """
        self.project = project
        self.output_path = output_path
        self.iProgramType = iProgramType
        self.bNeedMain = self.iProgramType == 0
        self.mode = mode
        self.sln_cmake_path = sln_cmake_path

        if mode == 'new':
            self.sln_dir = output_path
            self.project_subdir = project.name
            self.project_output_path = os.path.join(output_path, project.name)
        else:
            self.project_output_path = output_path

        os.makedirs(self.project_output_path, exist_ok=True)

        self.hFiles = []
        self.cppFiles = []

        super().__init__()
        os.makedirs(self.project_output_path, exist_ok=True)

    def _build_render_data(self):
        for idx, field in enumerate(self.project.fields):
            field.makeData()
            field.index = idx
            if not hasattr(field, 'dof2') or field.dof2 is None:
                field.dof2 = len(field.dispNames)
            if not hasattr(field, 'headerGuard') or field.headerGuard is None:
                field.headerGuard = f"{field.name.upper()}_FIELD_DATA_H"
            if not hasattr(field, 'eleResNames') or not field.eleResNames:
                field.eleResNames = ["stress", "strain"]

    def _get_cmake_target_type(self) -> str:
        if self.iProgramType == 0:
            return "exec"
        elif self.iProgramType == 1:
            return "shared"
        else:
            return "static"

    def _get_program_type_name(self) -> str:
        types = {0: "Executable", 1: "Shared Library", 2: "Static Library"}
        return types.get(self.iProgramType, "Executable")

    # ========== 文件生成方法框架 ==========

    def makeMain(self):
        if not self.bNeedMain:
            print("⚠️  跳过 main.cpp 生成（当前程序类型不需要 main 函数）")
            return

        output_filename = "main.cpp"
        context = {
            "femDataClassName": self.femDataClassName,
            "project": self.project
        }
        self.write2File("main.cpp.j2", output_filename, context, output_path=self.project_output_path)
        self.cppFiles.append(output_filename)

    def makeFEMData(self):
        context = {
            "femDataClassName": self.femDataClassName,
            "headerGuard": f"{self.project.name.upper()}_DATA_H",
            "project": self.project.toDict()
        }
        h_filename = f"{self.project.name}Data.h"
        self.write2File("femdata.h.j2", h_filename, context, output_path=self.project_output_path)
        self.hFiles.append(h_filename)
        cpp_filename = f"{self.project.name}Data.cpp"
        self.write2File("femdata.cpp.j2", cpp_filename, context, output_path=self.project_output_path)
        self.cppFiles.append(cpp_filename)

    def makePhyFieldData(self, field):
        context = {
            "field": field,
            "headerGuard": f"{field.name.upper()}_FIELD_DATA_H",
            "femDataClassName": self.femDataClassName
        }
        h_filename = f"{field.name}FieldData.h"
        self.write2File("phyfielddata.h.j2", h_filename, context, output_path=self.project_output_path)
        self.hFiles.append(h_filename)
        cpp_filename = f"{field.name}FieldData.cpp"
        self.write2File("phyfielddata.cpp.j2", cpp_filename, context, output_path=self.project_output_path)
        self.cppFiles.append(cpp_filename)

    def makeEleSub(self, field, ele):
        base_class = self._get_ele_base_class(ele)
        context = {
            "ele": ele.toDict(),
            "femDataClassName": self.femDataClassName,
            "field": field,
            "baseClass": base_class,
            "headerGuard": f"{ele.name.upper()}_H",
            "baseClassParam": f"{ele.nNodes}, pData",
            "dim": self.project.dim
        }
        h_filename = f"{ele.name}.h"
        self.write2File("elesub.h.j2", h_filename, context, output_path=self.project_output_path)
        self.hFiles.append(h_filename)
        cpp_filename = f"{ele.name}.cpp"
        self.write2File("elesub.cpp.j2", cpp_filename, context, output_path=self.project_output_path)
        self.cppFiles.append(cpp_filename)

    def makeCMakeLists(self):
        context = {
            "project": self.project,
            "projectName": self.project.name,
            "iProgramType": self.iProgramType,
            "hFiles": self.hFiles,
            "cppFiles": self.cppFiles
        }
        self.write2File("cmake.j2", "CMakeLists.txt", context, output_path=self.project_output_path)

    def makeSlnCMake(self):
        context = {
            "slnName": self.project.name,
            "projects": [{"dir": self.project.name}]
        }
        self.write2File("cmakeSln.j2", "CMakeLists.txt", context, output_path=self.sln_dir)

    def _copy_cdfeg_lib(self):
        dst = os.path.join(self.sln_dir, "CDFEG")
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(CDFEG_LIB_DIR, dst)
        print(f"✅ 已复制 CDFEG 库到: {dst}")

    def _copy_third(self):
        dst = os.path.join(self.sln_dir, "third")
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(THIRD_DIR, dst)
        print(f"✅ 已复制 third 目录到: {dst}")

    def _add_to_existing_sln(self):
        if not self.sln_cmake_path:
            print("⚠️  未提供 sln_cmake_path，跳过追加 add_subdirectory")
            return

        cmake_file = os.path.abspath(self.sln_cmake_path)
        if not os.path.isfile(cmake_file):
            print(f"⚠️  解决方案 CMakeLists.txt 不存在: {cmake_file}")
            return

        sub_dir = os.path.relpath(self.project_output_path, os.path.dirname(cmake_file)).replace("\\", "/")
        add_line = f"add_subdirectory({sub_dir})"

        with open(cmake_file, 'r', encoding='utf-8') as f:
            content = f.read()

        if add_line in content:
            print(f"⚠️  add_subdirectory({sub_dir}) 已存在，跳过追加")
            return

        with open(cmake_file, 'a', encoding='utf-8') as f:
            f.write(f"\n{add_line}\n")
        print(f"✅ 已追加 add_subdirectory({sub_dir}) 到: {cmake_file}")

    def makeAll(self):
        self.hFiles.clear()
        self.cppFiles.clear()
        self.femDataClassName = f"{self.project.name}Data"

        print(f"\n🚀 开始生成 C++ 代码...")
        print(f"📁 输出路径: {self.project_output_path}")
        print(f"📦 项目名称: {self.project.name}")
        print(f"🔧 程序类型: {self._get_program_type_name()}")
        print(f"📊 场数量: {len(self.project.fields)}")
        print(f"📋 生成模式: {'新解决方案' if self.mode == 'new' else '添加到现有项目'}")

        if self.mode == 'new':
            print(f"\n📦 复制 CDFEG 基础库...")
            self._copy_cdfeg_lib()
            print(f"\n📦 复制 third 依赖目录...")
            self._copy_third()

        if self.bNeedMain:
            self.makeMain()
        print(f"\n📝 生成全局 FEMData 类...")
        self.makeFEMData()
        for field in self.project.fields:
            print(f"\n📝 生成场 '{field.name}' 的文件...")
            self.makePhyFieldData(field)
            for ele in field.eleSubs:
                self.makeEleSub(field, ele)

        print(f"\n📄 生成项目 CMakeLists.txt...")
        self.makeCMakeLists()

        if self.mode == 'new':
            print(f"\n📄 生成解决方案 CMakeLists.txt...")
            self.makeSlnCMake()
        else:
            print(f"\n📄 追加到现有解决方案 CMakeLists.txt...")
            self._add_to_existing_sln()

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
