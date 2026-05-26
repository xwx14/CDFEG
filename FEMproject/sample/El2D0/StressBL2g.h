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

#ifndef STRESS_BL2G_H
#define STRESS_BL2G_H
#include "CDFEG/IsoEleBase.h"
#include <vector>
#include <map>

class StressBL2g : public CDFEG::IsoEleBase {
public:
    StressBL2g(CDFEG::PhyFieldData* pData);
    ~StressBL2g();

    virtual CDFEG::EleSubResult& run(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;

    virtual CDFEG::uResult uEle(
        const std::vector<double>& r,
        const std::map<std::string, std::vector<double>>& coef,
        const std::map<std::string, double>& matParams
    ) override;

    virtual std::vector<double> shapeFun(const std::vector<double>& refc) override;

private:
    void run1D(
        const std::vector<double>& yy,
        double fu,
        double fv,
        std::vector<std::vector<double>>& els,
        std::vector<std::vector<double>>& elm,
        std::vector<std::vector<double>>& eld,
        std::vector<double>& ell
    );
};

#endif
