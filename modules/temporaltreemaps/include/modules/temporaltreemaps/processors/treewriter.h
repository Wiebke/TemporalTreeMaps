/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Monday, October 09, 2017 - 16:06:40
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
//#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/meshport.h>
//#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
//#include <inviwo/core/properties/buttonproperty.h>
//#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/fileproperty.h>
//#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
//#include <inviwo/core/properties/stringproperty.h>
//#include <inviwo/core/properties/transferfunctionproperty.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeWriter, Tree Writer}
    ![](org.inviwo.TemporalTreeWriter.png?classIdentifier=org.inviwo.TemporalTreeWriter)

    Writes a tree to disk.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeWriter
    \brief Writes a simple text file containing a tree structure.
    
    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeWriter : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeWriter();
    virtual ~TemporalTreeWriter() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    static json createJSON(std::shared_ptr<const TemporalTree> tree, const bool bNTG, const bool bNTGAddWeights, const bool bNTGAddOrder, double nTGScaleWeights);

protected:
    ///Our main computation function
    virtual void process() override;

//Ports
public:
    ///Tree to be written to disk.
    TemporalTreeInport portInTree;

//Properties
public:
    ///Filename of the tree file.
    FileProperty propFilename;

    ///Whether to pretty-print the output
    BoolProperty propPrettyPrint;

    ///Wheter we can overwrite the output file
    BoolProperty propOverwrite;

    ///Whether to add the weights to the NTG output
    BoolProperty propNTGAddWeights;

    ///Whether to add the order to the NTG output
    BoolProperty propNTGAddOrder;

    ///Scale the weights for NTG output
    DoubleProperty propNTGScaleWeights;

//Attributes
private:

};

} // namespace kth
} // namespace
