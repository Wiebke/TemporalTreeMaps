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

#include <QStandardPaths>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QList>
#include <QSettings>
#include <QUrl>
#include <QHBoxLayout>
#include <modules/qtwidgets/inviwofiledialog.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/toolsqt/filelistpropertywidgetqt.h>

namespace inviwo
{
namespace kth 
{ 

FileListPropertyWidgetQt::FileListPropertyWidgetQt(FileListProperty* property)
    : PropertyWidgetQt(property), property_(property)
{
    generateWidget();
    updateFromProperty();
}

void FileListPropertyWidgetQt::generateWidget()
{
    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);
    setLayout(hLayout);

    label_ = new EditableLabelQt(this, property_);
    hLayout->addWidget(label_);

    QHBoxLayout* hWidgetLayout = new QHBoxLayout();
    hWidgetLayout->setContentsMargins(0,0,0,0);
    QWidget* widget = new QWidget();
    widget->setLayout(hWidgetLayout);

    infoLabel_ = new QLabel(this);

    QSizePolicy sp = infoLabel_->sizePolicy();
    sp.setHorizontalStretch(3);
    infoLabel_->setSizePolicy(sp);

    openButton_ = new QToolButton(this);
    openButton_->setIcon(QIcon(":/icons/open.png"));
    hWidgetLayout->addWidget(infoLabel_);
    hWidgetLayout->addWidget(openButton_);

    sp = widget->sizePolicy();
    sp.setHorizontalStretch(3);
    widget->setSizePolicy(sp);

    hLayout->addWidget(widget);
    connect(openButton_, SIGNAL(pressed()), this, SLOT(setPropertyValue()));
}

void FileListPropertyWidgetQt::setPropertyValue()
{
    //Previously selected files
    std::vector< std::string > allPreviousFiles(property_->get());

    //Find the previously open directory
    std::string basePath(allPreviousFiles.size() ? allPreviousFiles.front() : "");
    if (!basePath.empty())
    {
        // only accept basePath if it exists
        if (filesystem::directoryExists(basePath))
        {
            // TODO: replace with filesystem:: functionality!
            basePath = QDir(QString::fromStdString(basePath)).absolutePath().toStdString();
        }
    }

    //Setup Extensions
    std::vector<std::string> filters = property_->getNameFilters();
    InviwoFileDialog importFileDialog(this, property_->getDisplayName(),
                                      property_->getContentType(),
                                      basePath);
    importFileDialog.setFileMode(inviwo::FileMode::ExistingFiles);
    for (std::vector<std::string>::const_iterator it = filters.begin(); it != filters.end(); ++it)
        importFileDialog.addExtension(*it);

    if (importFileDialog.exec())
    {
        QStringList paths = importFileDialog.selectedFiles();
        std::vector< std::string > fileList(paths.size());
        for(int i=0; i<paths.size(); i++)
        {
            fileList[i] = paths[i].toStdString();
        }
        property_->set(fileList);
    }

    updateFromProperty();
}

void FileListPropertyWidgetQt::updateFromProperty()
{
    infoLabel_->setText(QString("%1 files").arg(property_->get().size()));
}

} // namespace kth
} // namespace
