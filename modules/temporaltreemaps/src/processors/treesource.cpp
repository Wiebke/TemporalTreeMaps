/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Thursday, October 12, 2017 - 14:52:57
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treesource.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeSource::processorInfo_
{
    "org.inviwo.TemporalTreeSource",      // Class identifier
    "Tree Source",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeSource::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeSource::TemporalTreeSource()
    :DataSource<TemporalTree, TemporalTreeOutport>()
{
    DataSource<TemporalTree, TemporalTreeOutport>::file_.setContentType("tree");
    DataSource<TemporalTree, TemporalTreeOutport>::file_.setDisplayName("Tree file");
}

} // namespace kth
} // namespace

