/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Thursday, October 12, 2017 - 10:03:53
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/filesystem.h>
#include <nlohmann/json.hpp>
#include <modules/temporaltreemaps/datastructures/treejsonreader.h>

using json = nlohmann::json;

namespace inviwo
{
namespace kth
{

namespace
{

void ReadEdges(const json& j, const std::string& Name, TemporalTree::TAdjacency& EdgeBuffer)
{
    if (j.cend() != j.find(Name))
    {
        const json& jEdges = j[Name];
        for(json::const_iterator it = jEdges.cbegin(); it != jEdges.cend(); it++)
        {
            const size_t KeyVal = (*it)[0];
            const std::vector<size_t> MappedValues = (*it)[1];
            EdgeBuffer.insert(std::make_pair(KeyVal, MappedValues));
        }
    }
}

std::shared_ptr<TemporalTree> readJSONTree(const std::string& filePath, const int Type)
{
    //Does the file exist? Why is this being checked here? Should be more central!
    std::string Filename = filePath;
    if (!filesystem::fileExists(Filename))
    {
        std::string NewPath = filesystem::addBasePath(Filename);

        if (filesystem::fileExists(NewPath))
        {
            Filename = NewPath;
        }
        else
        {
            throw DataReaderException("Could not find input file: " + Filename, IvwContextCustom("JSONReader"));
        }
    }

    //Open the file
    std::ifstream InFile;
    //InFile.exceptions(std::ifstream::failbit | std::ifstream::badbit); //From the json documentation: Please note that setting the exception bit for failbit is inappropriate for this use case. It will result in program termination due to the noexcept specifier in use.
    try
    {
        InFile.open(Filename);
    }
    catch (const std::ifstream::failure& e)
    {  
        throw DataReaderException("Could not open input file: " + Filename
                                  + "\n  Error Code: " + e.what(), IvwContextCustom("JSONReader"));
    }

    //Read the file contents
    json j;
    switch (Type)
    {
        case 0:
            {
                InFile >> j;
                break;
            }

        case 1:
            {
                j = json::from_cbor(InFile);
                break;
            }

        case 2:
            {
                j = json::from_msgpack(InFile);
                break;
            }

        default:
            break;
        }

    //Close the file
    InFile.close();

    //Parse the content and create a tree from it.
    auto pTree = std::make_shared<TemporalTree>();

    //Read the nodes
    if (j.cend() != j.find("nodes"))
    {
        const json& jNodes = j["nodes"];
        for(json::const_iterator it = jNodes.cbegin(); it != jNodes.cend(); it++)
        {
            //Add the Node with its name
            std::string Name = (*it)["name"];
            pTree->nodes.emplace_back(Name);

            //Shorthand
            TemporalTree::TNode& ThisNode = pTree->nodes.back();

            //Add its data values
            const json& jValues = (*it)["values"];
            for(json::const_iterator vit = jValues.cbegin(); vit != jValues.cend(); vit++)
            {
                const uint64_t Time = (*vit)[0];
                const float Value = (*vit)[1];
                ThisNode.values.emplace(Time, Value);
            }
        }
    }

    //Read the edges
    ReadEdges(j, "edgesHierarchy", pTree->edgesHierarchy);
    ReadEdges(j, "edgesTime", pTree->edgesTime);

    //Read the order
    if (j.cend() != j.find("order"))
    {
        const json& jOrder = j["order"];
        for(json::const_iterator it = jOrder.cbegin(); it != jOrder.cend(); it++)
        {
            pTree->order.push_back((size_t)*it);
        }
    }

    //Check for consistency
    if (!pTree->checkConsistency())
    {
        LogErrorCustom("Tree Loader (json)", "Loaded Tree does not pass the consistency test.");
    }

    /*auto pDeAggregatedTree = std::make_shared<TemporalTree>();
    pTree->deaggregate(*pDeAggregatedTree);

    return pDeAggregatedTree;*/

    return pTree;
}

}

TemporalTreeJSONReader::TemporalTreeJSONReader()
    :DataReaderType<TemporalTree>()
{
    addExtension(FileExtension("tree", "TemporalTree JSON ASCII"));
}

TemporalTreeJSONReader::TemporalTreeJSONReader(const TemporalTreeJSONReader& rhs)
    :DataReaderType<TemporalTree>(rhs)
    {}

TemporalTreeJSONReader& TemporalTreeJSONReader::operator=(const TemporalTreeJSONReader& that)
{
    if (this != &that)
    {
        DataReaderType<TemporalTree>::operator=(that);
    }

    return *this;
}

TemporalTreeJSONReader* TemporalTreeJSONReader::clone() const
{
    return new TemporalTreeJSONReader(*this);
}

std::shared_ptr<TemporalTree> TemporalTreeJSONReader::readData(const std::string& filePath)
{
    return readJSONTree(filePath, 0);
}

std::shared_ptr<TemporalTree> TemporalTreeJSONReaderCBOR::readData(const std::string& filePath)
{
    return readJSONTree(filePath, 1);
}

std::shared_ptr<TemporalTree> TemporalTreeJSONReaderMsgPack::readData(const std::string& filePath)
{
    return readJSONTree(filePath, 2);
}



TemporalTreeJSONReaderCBOR::TemporalTreeJSONReaderCBOR()
    :DataReaderType<TemporalTree>()
{
    addExtension(FileExtension("cbortree", "TemporalTree JSON Binary CBOR"));
}

TemporalTreeJSONReaderCBOR::TemporalTreeJSONReaderCBOR(const TemporalTreeJSONReaderCBOR& rhs)
    :DataReaderType<TemporalTree>(rhs)
    {}

TemporalTreeJSONReaderCBOR& TemporalTreeJSONReaderCBOR::operator=(const TemporalTreeJSONReaderCBOR& that)
{
    if (this != &that)
    {
        DataReaderType<TemporalTree>::operator=(that);
    }

    return *this;
}

TemporalTreeJSONReaderCBOR* TemporalTreeJSONReaderCBOR::clone() const
{
    return new TemporalTreeJSONReaderCBOR(*this);
}



TemporalTreeJSONReaderMsgPack::TemporalTreeJSONReaderMsgPack()
    :DataReaderType<TemporalTree>()
{
    addExtension(FileExtension("msgpacktree", "TemporalTree JSON Binary MessagePack"));
}

TemporalTreeJSONReaderMsgPack::TemporalTreeJSONReaderMsgPack(const TemporalTreeJSONReaderMsgPack& rhs)
    :DataReaderType<TemporalTree>(rhs)
    {}

TemporalTreeJSONReaderMsgPack& TemporalTreeJSONReaderMsgPack::operator=(const TemporalTreeJSONReaderMsgPack& that)
{
    if (this != &that)
    {
        DataReaderType<TemporalTree>::operator=(that);
    }

    return *this;
}

TemporalTreeJSONReaderMsgPack* TemporalTreeJSONReaderMsgPack::clone() const
{
    return new TemporalTreeJSONReaderMsgPack(*this);
}


TemporalTreeJSONReaderNTG::TemporalTreeJSONReaderNTG()
    :DataReaderType<TemporalTree>()
{
    addExtension(FileExtension("ntg", "Nested Tracking Graph"));
}

TemporalTreeJSONReaderNTG::TemporalTreeJSONReaderNTG(const TemporalTreeJSONReaderNTG& rhs)
    :DataReaderType<TemporalTree>(rhs)
    {}

TemporalTreeJSONReaderNTG& TemporalTreeJSONReaderNTG::operator=(const TemporalTreeJSONReaderNTG& that)
{
    if (this != &that)
    {
        DataReaderType<TemporalTree>::operator=(that);
    }

    return *this;
}

TemporalTreeJSONReaderNTG* TemporalTreeJSONReaderNTG::clone() const
{
    return new TemporalTreeJSONReaderNTG(*this);
}


namespace
{
bool ReadEdgesNTG(const json& j, const std::string& Name,
                                          const std::map<std::string, size_t>& NodeNameToIndex,
                                          TemporalTree::TAdjacency& EdgeBuffer)
{
    if (j.cend() != j.find(Name))
    {
        bool bError(false);

        const json& jEdges = j[Name];
        for(json::const_iterator ignore = jEdges.cbegin(); ignore != jEdges.cend() && !bError; ignore++)
        {
            for(json::const_iterator it = ignore.value().cbegin(); it != ignore.value().cend() && !bError; it++)
            {
                const std::string FromNode = it.key();
                std::vector<std::string> ToNodes;
                if (!it.value().empty())
                {
                    if (it.value().front().type() == json::value_t::string)
                    {
						ToNodes = it.value().get<std::vector<std::string>>();
                    }
                    else
                    {
                        for(size_t num : it.value()) ToNodes.push_back(std::to_string(num));
                    }
                }

                //Convert the node names to indices
                if (NodeNameToIndex.count(FromNode))
                {
                    const size_t& idxFromNode = NodeNameToIndex.at(FromNode);

                    std::vector<size_t> ToNodeIndices;
                    ToNodeIndices.reserve(ToNodes.size());
                    for(const std::string& ToNodeName : ToNodes)
                    {
                        if (NodeNameToIndex.count(ToNodeName))
                        {
                            ToNodeIndices.push_back(NodeNameToIndex.at(ToNodeName));
                        }
                        else
                        {
                            bError = true;
                            break;
                        }
                    }

                    if (!bError)
                    {
                        //EdgeBuffer[idxFromNode].insert(EdgeBuffer[idxFromNode].end(), ToNodeIndices.begin(), ToNodeIndices.end());
                        EdgeBuffer.insert(std::make_pair(idxFromNode, ToNodeIndices));
                    }
                }
            }
        }

        return !bError;
    }

    return false;
}
}


std::shared_ptr<TemporalTree> TemporalTreeJSONReaderNTG::readData(const std::string& filePath)
{
    //Does the file exist? Why is this being checked here? Should be more central!
    std::string Filename = filePath;
    if (!filesystem::fileExists(Filename))
    {
        std::string NewPath = filesystem::addBasePath(Filename);

        if (filesystem::fileExists(NewPath))
        {
            Filename = NewPath;
        }
        else
        {
            throw DataReaderException("Could not find input file: " + Filename, IvwContextCustom("TemporalTreeJSONReaderNTG"));
        }
    }

    //Open the file
    std::ifstream InFile;
    //InFile.exceptions(std::ifstream::failbit | std::ifstream::badbit); //From the json documentation: Please note that setting the exception bit for failbit is inappropriate for this use case. It will result in program termination due to the noexcept specifier in use.
    try
    {
        InFile.open(Filename);
    }
    catch (const std::ifstream::failure& e)
    {  
        throw DataReaderException("Could not open input file: " + Filename
                                  + "\n  Error Code: " + e.what(), IvwContextCustom("TemporalTreeJSONReaderNTG"));
    }

    //Read the file contents
    json j;
    InFile >> j;

    //Close the file
    InFile.close();

    //Parse the content and create a tree from it.
    auto pTree = std::make_shared<TemporalTree>();
    // - global root at index = 0; we need to add its time & data values later
    pTree->nodes.emplace_back("Root");

    //Nodes have unique names in the NTG and edges refer to them. We need to resolve that.
    std::map<std::string, size_t> NodeNameToIndex;

    //Find minimal level. Yes, they start at 25 (!) in the Viscous_Fingers example.
    int MinLevel(INT_MAX);
    if (j.cend() != j.find("ET"))
    {
        const json& jEdges = j["ET"];
        for(json::const_iterator it = jEdges.cbegin(); it != jEdges.cend(); it++)
        {
            const auto& TheKey = it.key();
            int TheKeyInt = atoi(TheKey.c_str());
            MinLevel = std::min(MinLevel, TheKeyInt);
        }
    }
    else return pTree;

    //Read the nodes
    if (j.cend() != j.find("N"))
    {
        const json& jNodes = j["N"];
        uint64_t MinTime(std::numeric_limits<uint64_t>::max()), MaxTime(0);
        for(json::const_iterator it = jNodes.cbegin(); it != jNodes.cend(); it++)
        {
            //Get the name, which is the key in the json
            const std::string NodeName = it.key();
            // - save name for easy lookup when reading edges later
            NodeNameToIndex[NodeName] = pTree->nodes.size();
            pTree->nodes.emplace_back(NodeName);

            //Shorthand
            TemporalTree::TNode& ThisNode = pTree->nodes.back();

            //Get the value, which is a json object containing time step (t), data (w), and other unused info
            const json& jValue = it.value();
            const uint64_t Time = (jValue.find("t") != jValue.cend()) ? jValue["t"] : json(0);
            const float DataValue = (jValue.find("w") != jValue.cend()) ? jValue["w"] : 0;
            ThisNode.values.emplace(Time, DataValue);

            //Min and Max time for the root
            MinTime = std::min(MinTime, Time);
            MaxTime = std::max(MaxTime, Time+1);

            //CAREFUL: THIS DOES NOT WORK WHEN TIME STEPS ARE NOT JUST INCREMENTS.
            ThisNode.values.emplace(Time+1, DataValue); //To make it consistent with our data structure

            //Connect to the global root, if we are on the zeroth level
            if (jValue["l"] == MinLevel)
            {
                pTree->edgesHierarchy[0].push_back(NodeNameToIndex[NodeName]);
            }
        }

        //Root time
        pTree->nodes[0].values.emplace(MinTime, 0.f);
        pTree->nodes[0].values.emplace(MaxTime, 0.f);
    }

    //Read the edges
    bool bOKWhileReadingEdges(true);
    bOKWhileReadingEdges = bOKWhileReadingEdges && ReadEdgesNTG(j, "EN", NodeNameToIndex, pTree->edgesHierarchy);
    bOKWhileReadingEdges = bOKWhileReadingEdges && ReadEdgesNTG(j, "ET", NodeNameToIndex, pTree->edgesTime);

    if (!bOKWhileReadingEdges)
    {
        throw DataReaderException("Could not read edges!", IvwContextCustom("TemporalTreeJSONReaderNTG"));
    }

    //Check for consistency
    if (!pTree->checkConsistency())
    {
        LogError("Loaded Tree does not pass the consistency test.");
    }

    //return pTree;

    auto pAggregatedTree = std::make_shared<TemporalTree>(pTree->aggregate());

    if (!pAggregatedTree->checkConsistency())
    {
        LogError("Aggregated Tree does not pass the consistency test.");
    }

    return pAggregatedTree;
}

} // namespace kth
} // namespace

