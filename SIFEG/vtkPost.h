#ifndef VTKPOST_H
#define VTKPOST_H
#include "Processor.h"

namespace SIFEG {
    class vtkPost :
        public Processor
    {
        public:
        vtkPost(FEMData* data, PhyFieldData* fieldData);
        ~vtkPost();

        virtual int post(int it = 0);
		int writeVTK(const std::string& fn);
    };
}
#endif
