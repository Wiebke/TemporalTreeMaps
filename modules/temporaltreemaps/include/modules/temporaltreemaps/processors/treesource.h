/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Thursday, October 12, 2017 - 14:52:57
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/base/processors/datasource.h>
#include <modules/temporaltreemaps/datastructures/treeport.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeSource, Tree Source}
    ![](org.inviwo.TemporalTreeSource.png?classIdentifier=org.inviwo.TemporalTreeSource)

    Loads a tree.
    
    ### Outports
      * __Outport__ The loaded tree.
    
    ### Properties
      * __File name__ File to load.
*/


/** \class TemporalTreeSource
    \brief Loads a Tree.
    
    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeSource : public DataSource<TemporalTree, TemporalTreeOutport>
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeSource();
    virtual ~TemporalTreeSource() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

//Ports
public:

//Properties
public:

//Attributes
private:

};

} // namespace kth
} // namespace
