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
#ifndef VTKPOST_H
#define VTKPOST_H
#include "Processor.h"
#include <string>
#include <vector>
#include <utility>

namespace CDFEG {
    class CDFEG_API vtkPost :
        public Processor
    {
        public:
        vtkPost(DomainData* data);
        ~vtkPost();

        // 与 GidPrePost 一致：输出 parentPath/baseName_<it>.vtu + baseName.pvd
        void setFilePath(const std::string& parentPath, const std::string& baseName);
        virtual int post(int it = 0);

    private:
        int writeVTU(const std::string& fn);
        int writePVD(const std::string& fn);
        std::string _outPath = ".";
        std::string _baseName = "result";
        // 已写步：(it, time=it*_dt)，供 pvd 汇总；每次 post 全量重写 pvd
        std::vector<std::pair<int, double>> _steps;
    };
}
#endif
