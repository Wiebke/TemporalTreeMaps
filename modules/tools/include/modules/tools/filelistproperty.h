/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_FILELISTPROPERTY_H
#define IVW_FILELISTPROPERTY_H

#include <modules/tools/toolsmoduledefine.h>
#include <inviwo/core/properties/property.h>


namespace inviwo
{
namespace kth
{

    /** class FileListProperty
        *  Represents a list of files.
        *  The file names are stored as a vector of strings.
        *
        */
    class IVW_MODULE_TOOLS_API FileListProperty : public Property
    {
        //Friends
        //Types
    public:
        InviwoPropertyInfo();

        //Construction / Deconstruction
    public:
        /**
            * \brief Constructor.
            *
            * @param identifier identifier for the property
            * @param displayName displayName for the property
            * @param value the path to the file
            * @param semantics Can be set to Editor
            */
        FileListProperty(const std::string& identifier,
            const std::string& displayName,
            const std::string& contentType = "default",
            const InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
            const PropertySemantics semantics = PropertySemantics::Default);

        //FileListProperty(const FileListProperty& rhs);
        //FileListProperty& operator=(const FileListProperty& that);
        FileListProperty& operator=(const std::vector< std::string >& argFileList);
        //virtual FileListProperty* clone() const;

        virtual ~FileListProperty() = default;

        //Methods
    public:
        ///Clones the property
        virtual FileListProperty* clone() const override
        {
            return new FileListProperty(*this);
        }

        ///Sets the list of file names
        virtual void set(const std::vector< std::string >& argFileList)
        {
            if (fileList_ == argFileList) return; fileList_ = argFileList; propertyModified();
        }

        ///Sets property
        virtual void set(const Property* src) override
        {
            if (auto prop = dynamic_cast<const FileListProperty*>(src)) set(prop);
        }

        ///Sets property
        void set(const FileListProperty* src)
        {
            if (fileList_ == src->fileList_) return;
            fileList_ = src->fileList_;
            Property::set(src);
        }

        ///Returns the list of file names
        const std::vector< std::string >& get() const
        {
            return fileList_;
        }

        ///Saves state
        virtual void serialize(Serializer& s) const;

        ///Loads state
        virtual void deserialize(Deserializer& d);

        ///Adds a file extension filter to the list of filters
        void addNameFilter(const std::string& newFilter)
        {
            nameFilters_.push_back(newFilter);
        }

        ///Clears all file extension filters
        void clearNameFilters()
        {
            nameFilters_.clear();
        }

        ///Returns the list of file extension filters
        const std::vector<std::string>& getNameFilters() const
        {
            return nameFilters_;
        }

        ///Sets the content type describing the content of the selected files.
        ///@todo implement
        void setContentType(const std::string& contentType)
        {
            contentType_ = contentType;
        }

        ///Returns the content type describing the content of the selected files.
        ///@todo implement
        const std::string& getContentType() const
        {
            return contentType_;
        }

        //Attributes
    private:
        ///The list of files.
        std::vector< std::string > fileList_;

        ///File extension filters.
        std::vector< std::string > nameFilters_;

        ///The content type of the files. We go with one type right now.
        std::string contentType_;
    };

} // namespace kth
} // namespace

#endif // !IVW_FILELISTPROPERTY_H
