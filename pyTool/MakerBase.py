"""
MakerBase.py - 代码生成器基类

提供所有 Maker 类的通用功能：
- Jinja2 环境初始化
- 模板渲染和文件写入
- 通用数据管理

作者：系统自动生成
日期：2025-01-06
"""
from jinja2 import Environment, FileSystemLoader
import os
from abc import ABC, abstractmethod


class MakerBase(ABC):
    """
    代码生成器基类

    提供模板渲染和文件生成的通用功能，
    所有具体的 Maker 类都应继承此类。
    """

    def __init__(self, template_dir: str = None):
        """
        初始化代码生成器

        Args:
            template_dir: 模板目录路径（如果为 None，使用默认的 template 目录）
        """
        # 获取当前脚本所在目录
        self.current_dir = os.path.dirname(os.path.abspath(__file__))

        # 确定模板目录
        if template_dir is None:
            template_dir = os.path.join(self.current_dir, "template")

        # 初始化 Jinja2 环境
        self.env = Environment(
            loader=FileSystemLoader(template_dir),
            trim_blocks=True,      # 移除模板空白行
            lstrip_blocks=True,     # 移除模板行首空白
            autoescape=False        # 不自动转义（用于生成代码）
        )
        

    def write2File(self, template_name: str, output_filename: str, context: dict , output_path: str = None):
        """
        使用模板生成文件

        Args:
            template_name: 模板文件名（如 "main.cpp.j2"）
            output_filename: 输出文件名（如 "MyProjectMain.cpp"）
            context: 额外的模板上下文（可选）
            output_path: 输出路径（可选，如果提供则与 output_filename 拼接）

        Raises:
            Exception: 模板渲染或文件写入失败时抛出异常
        """
        try:
            # 加载并渲染模板
            template = self.env.get_template(template_name)
            output = template.render(**context)

            # 确定最终输出路径
            if output_path is not None:
                final_path = os.path.join(output_path, output_filename)
            else:
                final_path = output_filename

            # 写入文件
            with open(final_path, 'w', encoding='utf-8') as f:
                f.write(output)

            print(f"✅ 已生成: {output_filename}")

        except Exception as e:
            print(f"❌ 生成失败: {output_filename}")
            print(f"   错误信息: {str(e)}")
            raise

    def makeAll(self):
        """
        生成所有文件（抽象方法）

        子类应实现此方法，用于一次性生成所有必要的文件。
        """
        pass
