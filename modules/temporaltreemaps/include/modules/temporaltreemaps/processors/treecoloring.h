/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Thursday, November 30, 2017 - 16:22:24
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/colorbrewer.h>
#include <random>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeColoring, Tree Coloring}
    ![](org.inviwo.TemporalTreeColoring.png?classIdentifier=org.inviwo.TemporalTreeColoring)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeColoring
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeColoring : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeColoring();
    virtual ~TemporalTreeColoring() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

    float rand(const float min, const float max) const;

//Ports
public:
    TemporalTreeInport portInTree;

    TemporalTreeOutport portOutTree;

//Properties
public:
    OptionPropertyInt propColorScheme;

    TransferFunctionProperty propValueTranserFunc;
    FloatVec4Property propColorUniform;
    IntProperty propColorSeed;
    TemplateOptionProperty<colorbrewer::Family> propColorBrewerScheme;

    /// Alternate setting the colors
    BoolProperty propAlternate;
    /// HSV or color
    OptionPropertyInt propColorSpace;
    IntProperty propColorFrom;
    IntProperty propColorTo;
    TransferFunctionProperty propColorSpaceMap;
    FloatProperty propHSVOffset;
    FloatProperty propRangeDecay;
    FloatProperty propSaturationMin;
    FloatProperty propSaturationMax;

//Attributes
private:
    mutable std::mt19937 randomGen;
    mutable std::uniform_real_distribution<float> randomDis;
};

} // namespace kth
} // namespace
