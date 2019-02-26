/*********************************************************************
 *  Author  : Wiebke Köpp
 *  Init    : Monday, October 09, 2017 - 16:29:42
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo
{

class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeMapsModule : public InviwoModule
{
public:
    TemporalTreeMapsModule(InviwoApplication* app);
    virtual ~TemporalTreeMapsModule() = default;
};

} // namespace
