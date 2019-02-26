/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Friday, December 01, 2017 - 08:55:23
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
#include <modules/tools/volumesourceseriesdata.h>
#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
//#include <inviwo/core/properties/compositeproperty.h>
//#include <inviwo/core/properties/fileproperty.h>
//#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
//#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <modules/temporaltreemaps/datastructures/treeport.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeGenerateFromTrackingGraph, Tree Generate From Tracking Graph}
    ![](org.inviwo.TemporalTreeGenerateFromTrackingGraph.png?classIdentifier=org.inviwo.TemporalTreeGenerateFromTrackingGraph)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeGenerateFromTrackingGraph
    \brief Generates a temporal tree by tracking sublevelsets in time-dependent scalar fields.
    
    Bears similarity to Nested Tracking Graphs.

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeGenerateFromTrackingGraph : public Processor
{ 
//Friends
//Types
public:
    ///Relations used for predicates
    enum RELATION
    {
        SMALLER,
        SMALLEREQUAL,
        GREATEREQUAL,
        GREATER
    };

//Construction / Deconstruction
public:
    TemporalTreeGenerateFromTrackingGraph();
    virtual ~TemporalTreeGenerateFromTrackingGraph() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    void GenerateNestedTrackingGraphs();

protected:
    ///Our main computation function
    virtual void process() override;

    ///Updates the min/max range of all isovalue sliders according to CurrentValueRange
    void UpdateMinMaxAllSliders();

    ///Updates the min/max range of the given isovalue slider according to CurrentValueRange
    void UpdateMinMaxSlider(DoubleProperty& Slider);

    ///Returns a pointer to the output example field
    std::shared_ptr<Volume> CreateOrReuseResultVolume(std::shared_ptr<Volume> pInVolume);

    ///Returns a pointer to the output tree
    std::shared_ptr<TemporalTree> CreateOrReuseResultTree();

    ///Fills given array with isovalues from the properties in a sorted manner.
    void GetIsovaluesSorted(std::vector<double>& Isovalues);

//Ports
public:
    ///The time-dependent scalar field
    VolumeSeriesInport portSeries;

    ///The output tree
    TemporalTreeOutport portOutTree;

    ///An example segmentation output for a selected time step
    VolumeOutport portOutSegmentationExample;

//Properties
public:
    ///Selection of the example time step
    IntSizeTProperty propTimeStepExample;

    ///Colormap for the example
    TransferFunctionProperty propColormap;

    ///How to compare against the isovalues
    TemplateOptionProperty<RELATION> propRelation;

    ///Group of isovalues
    CompositeProperty propHierarchyLevelGroup;

    ///Number of isovalues
    IntSizeTProperty propNumLevels;

    ///Whether to create ghost children
    BoolCompositeProperty propGhostChildrenCreate;

    ///Whether ghost children mirror their parent's tracks or not (connected in time like their parents.)
    BoolProperty propGhostChildrenTrackLikeParents;

    ///Whether to run in parallel or not
    BoolProperty propParallel;

    ///Since scanning is expensive,
    ///we scan only on explicit user demand, namely when pressing this button.
    ButtonProperty propAction;

//Attributes
private:
    ///Cached value range of the input data used in several places for UI settings and coloring
    dvec2 CurrentValueRange;

    ///The output example field
    std::shared_ptr<Volume> pOutExampleVolume;

    ///The output tree
    std::shared_ptr<TemporalTree> pOutTree;
};

} // namespace kth
} // namespace
