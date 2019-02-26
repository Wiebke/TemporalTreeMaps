/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, November 22, 2017 - 13:13:38
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
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

#include <modules/temporaltreemaps/datastructures/treeport.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeGenerateFromFileSystem, File System Tree Generator}
    ![](org.inviwo.TemporalTreeGenerateFromFileSystem.png?classIdentifier=org.inviwo.TemporalTreeGenerateFromFileSystem)

    Generates a tree from a file system.
*/


/** \class TemporalTreeGenerateFromFileSystem
    \brief Generates a tree from a file system.
    
    Runs from a directory, scans its entire hierarchical content and builds a tree from it.

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeGenerateFromFileSystem : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeGenerateFromFileSystem();
    virtual ~TemporalTreeGenerateFromFileSystem() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    void ScanFileSystem();

protected:
    ///Our main computation function
    virtual void process() override;

//Ports
public:
    ///The output tree
    TemporalTreeOutport portOutTree;

//Properties
public:
    ///Where to start the search
    DirectoryProperty propStartDir;

    ///Maximum depth
    OrdinalProperty<int> propMaxDepth;

    ///Since scanning is expensive and depends on the unknown filesystem,
    ///we scan only on explicit user demand, namely when pressing this button.
    ButtonProperty propAction;

//Attributes
private:

};

} // namespace kth
} // namespace
