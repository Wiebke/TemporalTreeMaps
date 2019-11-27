/*********************************************************************
 *  File    : amirameshvolumereader.cpp
 *  Author  : Tino Weinkauf
 *  Init    : 11.10.15 - 12:01:38
 *
 *  Project : Inviwo KTH Tools
 *  Package : tools
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/tools/amirameshvolumereader.h>
#include <modules/tools/setminmaxdatamap.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/formatconversion.h>

namespace inviwo
{
namespace kth
{

//Map between Amira's PrimType and Inviwo DataFormats
const std::map< std::string, std::pair< inviwo::NumericType, size_t > > AmiraMeshVolumeReader::PrimTypeMap =
{
    {"float",	{inviwo::NumericType::Float,			 32}},
    {"short",	{inviwo::NumericType::SignedInteger,	 16}},
    {"byte",	{inviwo::NumericType::UnsignedInteger,    8}},
    {"double",	{inviwo::NumericType::Float,			 64}},
    {"int",		{inviwo::NumericType::SignedInteger,	 32}},
    {"ushort",	{inviwo::NumericType::UnsignedInteger,   16}},
    {"uint",	{inviwo::NumericType::UnsignedInteger,   32}},
    {"sbyte",	{inviwo::NumericType::SignedInteger,	  8}},
    {"int64",	{inviwo::NumericType::SignedInteger,	 64}}
};

AmiraMeshVolumeReader::AmiraMeshVolumeReader()
    :DataReaderType<Volume>()
    ,amFileName_("")
    ,dimensions_(0)
    ,format_(nullptr)
{
    addExtension(FileExtension("am", "Amira Mesh binary file"));
}

AmiraMeshVolumeReader::AmiraMeshVolumeReader(const AmiraMeshVolumeReader& rhs)
    :DataReaderType<Volume>(rhs)
    ,amFileName_(rhs.amFileName_)
    ,dimensions_(rhs.dimensions_)
    ,format_(rhs.format_) {}

AmiraMeshVolumeReader& AmiraMeshVolumeReader::operator=(const AmiraMeshVolumeReader& that)
{
    if (this != &that)
    {
        amFileName_ = that.amFileName_;
        dimensions_ = that.dimensions_;
        format_ = that.format_;
        DataReaderType<Volume>::operator=(that);
    }

    return *this;
}

AmiraMeshVolumeReader* AmiraMeshVolumeReader::clone() const
{
    return new AmiraMeshVolumeReader(*this);
}

namespace
{
/** Find a string in the given buffer and return a pointer
    to the contents directly behind the SearchString.
    If not found, return the buffer. A subsequent sscanf()
    will fail then, but at least we return a decent pointer.
*/
const char* FindAndJump(const char* buffer, const char* SearchString)
{
    const char* FoundLoc = strstr(buffer, SearchString);
    if (FoundLoc) return FoundLoc + strlen(SearchString);
    return buffer;
}
};

std::shared_ptr<Volume> AmiraMeshVolumeReader::readData(const std::string& filePath)
{
    std::string fileName = filePath;
    if (!filesystem::fileExists(filePath))
    {
        std::string newPath = filesystem::addBasePath(filePath);

        if (filesystem::fileExists(newPath))
        {
            fileName = newPath;
        }
        else
        {
            throw DataReaderException("Could not find input file: " + fileName, IvwContext);
        }
    }

    std::string fileDirectory = filesystem::getFileDirectory(fileName);
    std::string fileExtension = filesystem::getFileExtension(fileName);
    amFileName_ = fileName;

    FILE* fp = fopen(amFileName_.c_str(), "rb");
    if (!fp)
    {
        throw DataReaderException("Could not open input file: " + amFileName_, IvwContext);
    }

    //We read the first 2k bytes into memory to parse the header.
    //The fixed buffer size looks a bit like a hack, and it is one, but it gets the job done.
    char buffer[2048];
    fread(buffer, sizeof(char), 2047, fp);
    buffer[2047] = '\0'; //The following string routines prefer null-terminated strings

    if (!strstr(buffer, "# AmiraMesh BINARY-LITTLE-ENDIAN 2.1"))
    {
        fclose(fp);
        throw DataReaderException("Not a proper AmiraMesh file.", IvwContext);
    }

    //Find the Lattice definition, i.e., the dimensions of the uniform grid
    int xDim(0), yDim(0), zDim(0);
    sscanf(FindAndJump(buffer, "define Lattice"), "%d %d %d", &xDim, &yDim, &zDim);
    if (xDim <= 0 || yDim <= 0 || zDim <= 0)
    {
        fclose(fp);
        throw DataReaderException("Dimensions are invalid.", IvwContext);
    }
    dimensions_.x = xDim;
    dimensions_.y = yDim;
    dimensions_.z = zDim;

    //Find the BoundingBox
    float xmin(1.0f), ymin(1.0f), zmin(1.0f);
    float xmax(-1.0f), ymax(-1.0f), zmax(-1.0f);
    sscanf(FindAndJump(buffer, "BoundingBox"), "%g %g %g %g %g %g", &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
    if (xmin > xmax || ymin > ymax || zmin > zmax)
    {
        fclose(fp);
        throw DataReaderException("Bounding Box is invalid.", IvwContext);
    }

    //Is it a uniform grid? That is the only thing we support in this reader.
    const bool bIsUniform = (strstr(buffer, "CoordType \"uniform\"") != NULL);
    if (!bIsUniform)
    {
        fclose(fp);
        throw DataReaderException("Unsupported coordinate type. Only uniform lattices are supported!", IvwContext);
    }

    //Primitive data type, i.e., float, short, ...
    //Type of the field: scalar, vector
    const char LatticeSearchTerm[] = "Lattice { ";
    char* bufPrimType = strstr(buffer, LatticeSearchTerm);
    if (!bufPrimType)
    {
        //We did not find a lattice here. Jump out!
        fclose(fp);
        throw DataReaderException("No Lattice found in this AmiraMesh file.", IvwContext);
    }

    //Jump right infront of the primitive type
    bufPrimType += sizeof(LatticeSearchTerm) - 1;

    //I am so sorry for this
    std::string strAmiraPrimType;
    int i(0);
    for(;i<20;i++)
    {
        if (bufPrimType[i] == ' ' || bufPrimType[i] == '[') break;

        strAmiraPrimType += bufPrimType[i];
    }

    //Does it have several components?
    int NumComponents(1);
    if (bufPrimType[i] == '[')
    {
        //A field with more than one component, i.e., a vector field
        sscanf(bufPrimType + i + 1, "%d", &NumComponents);
    }
    if (NumComponents < 1)
    {
        fclose(fp);
        throw DataReaderException("Number of Components seems broken.", IvwContext);
    }

    //Get the corresponding Inviwo DataFormat
    const auto itMap = PrimTypeMap.find(strAmiraPrimType);
    if (itMap == PrimTypeMap.end())
    {
        fclose(fp);
        throw DataReaderException("Unsupported data type \'" + strAmiraPrimType + "\'.", IvwContext);
    }

    format_ = DataFormatBase::get(itMap->second.first, NumComponents, itMap->second.second);

    //READ IT!
    const auto idxStartData = strstr(buffer, "# Data section follows") - buffer;
    if (idxStartData <= 0)
    {
        fclose(fp);
        throw DataReaderException("Could not find data section.", IvwContext);
    }
    //Set the file pointer to the beginning of "# Data section follows"
    fseek(fp, (long)idxStartData, SEEK_SET);
    //Consume this line, which is "# Data section follows"
    fgets(buffer, 2047, fp);
    //Consume the next line, which is "@1"
    fgets(buffer, 2047, fp);

    //Read the data
    // - how much to read
    const std::size_t NumToRead = dimensions_.x*dimensions_.y*dimensions_.z*(format_->getSize());
    // - prepare memory
    char* pData = new char[NumToRead];
    if (!pData)
    {
        fclose(fp);
        throw DataReaderException("Could not allocate enough memory.", IvwContext);
    }
    // - do it
    const size_t ActRead = fread((char*)pData, sizeof(char), NumToRead, fp);
    // - ok?
    if (NumToRead != ActRead)
    {
        fclose(fp);
        throw DataReaderException("Could not read the expected amount of bytes (got less).", IvwContext);
    }

    //Close the file
    fclose(fp);

    //Setup Inviwo's data structures

    //Set the bounding box
    // - The basis represents the size of the bbox, i.e., the lengths of its sides
    // - Essentially, the scaling factors you need to make a [0,1]^3 box the correct size
    glm::mat3 basis(1.0f);
    basis[0][0] = xmax - xmin;
    basis[1][1] = ymax - ymin;
    basis[2][2] = zmax - zmin;
    // - The offset is the lower left corner.
    glm::vec3 offset(xmin, ymin, zmin);

    //Create volume and fill in data
    auto volume = std::make_shared<Volume>(dimensions_, format_);
    // - bbox
    volume->setBasis(basis);
    volume->setOffset(offset);
    // - representation. A RAM representation since the data is already in memory.
    auto volRAM = createVolumeRAM(dimensions_, format_, pData);
    volume->addRepresentation(volRAM);
    // - min and max
    volume->dataMap_.initWithFormat(format_);
    SetMinMaxForInviwoDataMap(pData, NumToRead, volume);

    //Tell them what we did.
    std::string size = util::formatBytesToString(NumToRead);
    LogInfo("Loaded volume: " << filePath << " size: " << size);

    return volume;
}

} // namespace kth
} // namespace

