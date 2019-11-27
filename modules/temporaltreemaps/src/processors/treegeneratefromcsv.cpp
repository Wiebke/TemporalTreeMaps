/*********************************************************************
 *  Author  : <author>
 *  Init    : <datetime>
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treegeneratefromcsv.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeGenerateFromCSV::processorInfo_
{
    "org.inviwo.TemporalTreeGenerateFromCSV",      // Class identifier
    "Tree Generate From CSV",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeGenerateFromCSV::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeGenerateFromCSV::TemporalTreeGenerateFromCSV()
    :Processor() 
    , inHierarchy("inHierarchy")
    , inDataValues("inData")
    , outTree("outTree")
{
    addPort(inHierarchy);
    addPort(inDataValues);
    
    
    addPort(outTree);

    //addProperty();
}

void TemporalTreeGenerateFromCSV::process()
{
    auto hierarchy = inHierarchy.getData();
    auto dataValues = inDataValues.getData();

    size_t rows = hierarchy->getNumberOfRows();
    size_t columns = hierarchy->getNumberOfColumns();

    if (columns < 5)
    {
        LogProcessorError("Not enough columns for a tree dataset in hierarchy.");
    }
    // Column 0 is an index column that gets created on reading in the data
    std::shared_ptr<const Column> columnLevel = hierarchy->getColumn(1);
    std::shared_ptr<const Column> columnName = hierarchy->getColumn(2);
    std::shared_ptr<const Column> columnStartTime = hierarchy->getColumn(3);
    std::shared_ptr<const Column> columnEndTime = hierarchy->getColumn(4);

    std::shared_ptr<TemporalTree> tree = std::make_shared<TemporalTree>(TemporalTree());

    std::map<std::string, size_t> nameToIndexMap;

    std::vector<size_t> lastLevelIndices(1,0);

    for (size_t r(0); r < rows; r++)
    {
        std::string name = columnName->getAsString(r);
        uint64_t level = uint64_t(columnLevel->getAsDouble(r));
        uint64_t start = uint64_t(columnStartTime->getAsDouble(r));
        uint64_t end = uint64_t(columnEndTime->getAsDouble(r));
        size_t nodeIndex;
        if (level==0)
        {
            if (!tree->nodes.empty())
            {
                LogProcessorError("The first node must be the root node");
            }
            TemporalTree::TNode node = TemporalTree::TNode(name, { { start, 10.0f },{ end, 10.0f } });
            tree->addNode(node);
            lastLevelIndices[0] = tree->nodes.size() - 1;
            nodeIndex = lastLevelIndices[0];
            nameToIndexMap.insert({ name, nodeIndex });
        }
        else
        {
            auto it = nameToIndexMap.find(name);
            // If there already exists another node with the same name, only add the temporal edge
            if (it != nameToIndexMap.end())
            {
                tree->addHierarchyEdge(lastLevelIndices[level - 1], it->second);
                nodeIndex = it->second;
            }
            else
            {
                tree->addChild(lastLevelIndices[level - 1], name, { { start, 10.0f },{ end, 10.0f } });
                if (lastLevelIndices.size() <= level)
                {
                    lastLevelIndices.push_back(0);
                }
                lastLevelIndices[level] = tree->nodes.size() - 1;
                nodeIndex = lastLevelIndices[level];
                nameToIndexMap.insert({ name, nodeIndex });
            }
        }

        for (size_t col = 5; col < columns; col++)
        {
            std::string predecessor = hierarchy->getColumn(col)->getAsString(r);
            auto it = nameToIndexMap.find(predecessor);
            // Predecessor needs to exist before
            if (it != nameToIndexMap.end())
            {
                tree->addTemporalEdge(it->second, nodeIndex);
            }
            else
            {
                if (predecessor != "")
                {
                    LogProcessorError("Predecessor does not exist.");
                    return;
                }
            }
        }

    }

    // Fill with data values
    rows = dataValues->getNumberOfRows();
    columns = dataValues->getNumberOfColumns();

    if (columns < 4)
    {
        LogProcessorError("Not enough columns for a tree dataset in data values.");
    }

    for (size_t r(0); r < rows; r++)
    {
        auto name = dataValues->getColumn(1)->getAsString(r);
        auto it = nameToIndexMap.find(name);
        // If there already exists another node with the same name, only add the temporal edge
        if (it != nameToIndexMap.end())
        {
            TemporalTree::TNode& node = tree->nodes[it->second];
            for (size_t c(2); c < columns; c++)
            {
                auto header = dataValues->getHeader(c);
                if (header == "")
                {
                    continue;
                }
                uint64_t time = uint64_t(std::stoi(header));
                auto value = dataValues->getColumn(c)->getAsDouble(r);
                if (std::isnan(value))
                {
                    continue;
                }
                node.values[time] = float(value);
            }
        }
        else
        {
            LogProcessorError("Node does not occur in hierarchy.");
        }

    }

    outTree.setData(tree);


}

} // namespace
} // namespace

