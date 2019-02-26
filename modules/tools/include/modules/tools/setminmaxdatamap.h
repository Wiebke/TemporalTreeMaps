#pragma once

#include <modules/tools/toolsmoduledefine.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo
{
namespace kth
{


    ///Sets the value and data range of the data map.
    void IVW_MODULE_TOOLS_API SetMinMaxForInviwoDataMap(const char* pData, const size_t NumOfBytes, std::shared_ptr<inviwo::Volume> pVolume);
}
};
