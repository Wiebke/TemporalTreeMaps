/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Thursday, October 12, 2017 - 10:03:53
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/datareader.h>

#include <modules/temporaltreemaps/datastructures/tree.h>

namespace inviwo
{
namespace kth
{

/** \class TemporalTreeJSONReader
    \brief Reads a TemporalTree from a JSON ASCII file.

    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeJSONReader : public DataReaderType<TemporalTree>
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeJSONReader();
    TemporalTreeJSONReader(const TemporalTreeJSONReader& rhs);
    TemporalTreeJSONReader& operator=(const TemporalTreeJSONReader& that);
    virtual TemporalTreeJSONReader* clone() const;
    virtual ~TemporalTreeJSONReader() = default;

//Methods
public:
    ///Reads the data
    virtual std::shared_ptr<TemporalTree> readData(const std::string& filePath) override;
};


/** \class TemporalTreeJSONReaderCBOR
    \brief Reads a TemporalTree from a JSON CBOR file.

    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeJSONReaderCBOR : public DataReaderType<TemporalTree>
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeJSONReaderCBOR();
    TemporalTreeJSONReaderCBOR(const TemporalTreeJSONReaderCBOR& rhs);
    TemporalTreeJSONReaderCBOR& operator=(const TemporalTreeJSONReaderCBOR& that);
    virtual TemporalTreeJSONReaderCBOR* clone() const;
    virtual ~TemporalTreeJSONReaderCBOR() = default;

//Methods
public:
    ///Reads the data
    virtual std::shared_ptr<TemporalTree> readData(const std::string& filePath) override;
};



/** \class TemporalTreeJSONReaderMsgPack
    \brief Reads a TemporalTree from a JSON MsgPack file.

    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeJSONReaderMsgPack : public DataReaderType<TemporalTree>
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeJSONReaderMsgPack();
    TemporalTreeJSONReaderMsgPack(const TemporalTreeJSONReaderMsgPack& rhs);
    TemporalTreeJSONReaderMsgPack& operator=(const TemporalTreeJSONReaderMsgPack& that);
    virtual TemporalTreeJSONReaderMsgPack* clone() const;
    virtual ~TemporalTreeJSONReaderMsgPack() = default;

//Methods
public:
    ///Reads the data
    virtual std::shared_ptr<TemporalTree> readData(const std::string& filePath) override;
};


/** \class TemporalTreeJSONReaderNTG
    \brief Reads a TemporalTree in the form of a Nested Tracking Graph from a JSON ASCII file.

    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeJSONReaderNTG : public DataReaderType<TemporalTree>
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    TemporalTreeJSONReaderNTG();
    TemporalTreeJSONReaderNTG(const TemporalTreeJSONReaderNTG& rhs);
    TemporalTreeJSONReaderNTG& operator=(const TemporalTreeJSONReaderNTG& that);
    virtual TemporalTreeJSONReaderNTG* clone() const;
    virtual ~TemporalTreeJSONReaderNTG() = default;

//Methods
public:
    ///Reads the data
    virtual std::shared_ptr<TemporalTree> readData(const std::string& filePath) override;
};

} // namespace kth
} // namespace
