/*********************************************************************
 *  File    : amirameshvolumereader.h
 *  Author  : Tino Weinkauf
 *  Init    : 11.10.15 - 12:01:34
 *
 *  Project : Inviwo KTH Tools
 *  Package : tools
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/tools/toolsmoduledefine.h>

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/io/datareader.h>
         
namespace inviwo
{
namespace kth
{

    /** Reads AmiraMesh files with a uniform lattice into an Inviwo Volume.

        AmiraMesh is the native file format of Amira.
        The academic version of Amira is developed by
        the Visualization and Data Analysis Group at Zuse Institute Berlin.
        Commercial versions are available from Visage Imaging, Berlin
        and VSG - Visualization Sciences Group, France.

        Rest assured, that the routines in Amira itself
        look entirely different and are way more advanced and general.
        This here is a rather quick hack, but it gets the job done.

        We read directly into RAM here, since AmiraMesh does not store Min-Max Metadata.

        @author Tino Weinkauf
    */
    class IVW_MODULE_TOOLS_API AmiraMeshVolumeReader : public DataReaderType<Volume>
    {
        //Friends
        //Types
    public:

        //Construction / Deconstruction
    public:
        AmiraMeshVolumeReader();
        AmiraMeshVolumeReader(const AmiraMeshVolumeReader& rhs);
        AmiraMeshVolumeReader& operator=(const AmiraMeshVolumeReader& that);
        virtual AmiraMeshVolumeReader* clone() const;
        virtual ~AmiraMeshVolumeReader() {}

        //Methods
    public:
        ///Reads the data
        virtual std::shared_ptr<Volume> readData(const std::string& filePath) override;
        //Attributes
    private:
        std::string amFileName_;
        size3_t dimensions_;
        const DataFormatBase* format_;

        ///Maps Amira's primitive data types to inviwo's equivalent.
        static const std::map< std::string, std::pair< inviwo::NumericType, size_t > > PrimTypeMap;
    };

} // namespace kth
} // namespace



