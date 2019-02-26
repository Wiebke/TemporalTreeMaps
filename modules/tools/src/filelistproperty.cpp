/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <modules/tools/filelistproperty.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo
{
namespace kth
{ 

PropertyClassIdentifier(FileListProperty, "org.inviwo.FileListProperty");

FileListProperty::FileListProperty(const std::string& identifier,
                                   const std::string& displayName,
                                   const std::string& contentType,
                                   const InvalidationLevel invalidationLevel,
                                   const PropertySemantics semantics)
    :Property(identifier, displayName, invalidationLevel, semantics)
    ,contentType_(contentType)
{
    addNameFilter("All Files (*)");
}


//FileListProperty::FileListProperty(const FileListProperty& rhs)
//    :TemplateProperty< std::vector< std::string > >(rhs)
//    ,nameFilters_(rhs.nameFilters_)
//    ,contentType_(rhs.contentType_)
//{
//
//}
//
//
//FileListProperty& FileListProperty::operator=(const FileListProperty& that)
//{
//    if (this != &that) {
//        TemplateProperty< std::vector< std::string > >::operator=(that);
//        nameFilters_ = that.nameFilters_;
//        contentType_ = that.contentType_;
//    }
//    return *this;
//}


FileListProperty& FileListProperty::operator=(const std::vector< std::string >& value)
{
    fileList_ = value;
    return *this;
}


//FileListProperty* FileListProperty::clone() const
//{
//    return new FileListProperty(*this);
//}

void FileListProperty::serialize(Serializer& s) const
{
    Property::serialize(s);

    s.serialize("fileList", fileList_, "file");
    s.serialize("nameFilter", nameFilters_, "filter");
    s.serialize("contentType", contentType_);
}


void FileListProperty::deserialize(Deserializer& d)
{
    Property::deserialize(d);

    try
    {
        fileList_.clear();
        d.deserialize("fileList", fileList_, "file");
        nameFilters_.clear();
        d.deserialize("nameFilter", nameFilters_, "filter");
        d.deserialize("contentType", contentType_);
    }
    catch (SerializationException& e)
    {
        LogInfo("Problem deserializing File List Property: " << e.getMessage());
    }
}

} // namespace kth
} // namespace
