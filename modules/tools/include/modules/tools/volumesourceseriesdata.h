/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <modules/tools/toolsmoduledefine.h>
//#include <KTH/tools/volumesourceseries.h>
//#include <inviwo/core/common/inviwo.h>
//#include <inviwo/core/processors/processor.h>
//#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>


namespace inviwo
{
namespace kth
{

//Forward Declaration
class VolumeSourceSeries;

class IVW_MODULE_TOOLS_API VolumeSourceSeriesData
{
//Friends
//Types
public:

//Statics
public:
    static uvec3 COLOR_CODE;
    static const std::string CLASS_IDENTIFIER;

//Construction / Deconstruction
public:
    VolumeSourceSeriesData(VolumeSourceSeries* argpSeries)
        :pSeries(argpSeries)
    {}

    virtual ~VolumeSourceSeriesData() = default;

//Methods
public:
    ///Returns number of volumes in the series.
    size_t GetNumFiles() const;

    ///Returns a particular volume of the series. @note May be NULL.
    std::shared_ptr< Volume > GetVolume(const size_t Idx) const;

//Attributes
protected:
    /** Reference to the volume series processor.
    
        We have a pointer here to reflect changes in the UI
        even when not auto-processing.
    */
    VolumeSourceSeries* pSeries;
};

///Ports
using VolumeSeriesOutport = DataOutport<VolumeSourceSeriesData>;
using VolumeSeriesInport = DataInport<VolumeSourceSeriesData>;

} // namespace kth
} // namespace

