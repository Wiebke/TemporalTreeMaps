/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, March 28, 2018 - 11:46:34
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */


#include <modules/temporaltreemaps/processors/treeordercomputationsanodes.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeOrderComputationSANodes::processorInfo_
{
    "org.inviwo.TemporalTreeOrderComputationSANodes",      // Class identifier
    "Tree Order Computation SANodes",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeOrderComputationSANodes::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeOrderComputationSANodes::TemporalTreeOrderComputationSANodes()
    :Processor()
{
    //addPort();
    //addProperty();
}


void TemporalTreeOrderComputationSANodes::process()
{
	// Do nothing
}

} // namespace
} // namespace

