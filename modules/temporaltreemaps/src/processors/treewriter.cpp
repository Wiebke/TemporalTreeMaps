/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Monday, October 09, 2017 - 16:06:40
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <inviwo/core/util/filesystem.h>
#include <modules/temporaltreemaps/processors/treewriter.h>
#include <modules/temporaltreemaps/datastructures/treeorder.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeWriter::processorInfo_
{
    "org.inviwo.TemporalTreeWriter",      // Class identifier
    "Tree Writer",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeWriter::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeWriter::TemporalTreeWriter()
    :Processor()
    , portInTree("InTree")
    , propFilename("Filename", "Filename")
    , propPrettyPrint("PrettyPrint", "Pretty Print")
    , propOverwrite("Overwrite", "Overwrite", false)
    , propNTGAddWeights("NTGAddWeights", "Add weight values to the NTG output", true)
    , propNTGAddOrder("NTGAddOrder", "Add the order to the NTG output", true)
    , propNTGScaleWeights("NTGScaleWeights", "Scaled Weights", 1.0f, 0.01f, 1.0f, 0.01f)
{
    addPort(portInTree);

    propFilename.addNameFilter(FileExtension("tree", "TemporalTree ASCII"));
    propFilename.addNameFilter(FileExtension("cbortree", "TemporalTree Binary CBOR"));
    propFilename.addNameFilter(FileExtension("msgpacktree", "TemporalTree Binary MessagePack"));
    propFilename.addNameFilter(FileExtension("ntg", "Nested Tracking Graph"));
    propFilename.setAcceptMode(AcceptMode::Save);
    addProperty(propFilename);
 
    propFilename.onChange([&]()
    {
        const bool bASCIITree = (propFilename.get().rfind(".tree") != -1);
        const bool bNTG = (propFilename.get().rfind(".ntg") != -1);
        
        propPrettyPrint.setVisible(bASCIITree || bNTG);
        propNTGAddWeights.setVisible(bNTG);
        propNTGAddOrder.setVisible(bNTG);
    });

    addProperty(propPrettyPrint);
    addProperty(propOverwrite);
    addProperty(propNTGAddWeights);
    addProperty(propNTGAddOrder);
    addProperty(propNTGScaleWeights);
}


namespace
{

void GetLayerOrder(const TemporalTree& Tree, const std::map<size_t, size_t>& LeafOrderMap,
                   const std::vector<size_t>& LevelIndices, std::vector<size_t>& LayerOrder)
{
    //Prepare memory
    const size_t NumLeaves = LeafOrderMap.size();
    if (NumLeaves == 0) return;

    //For actually sorting the parents
    std::vector<std::pair<size_t, size_t>> ParentFirstLeaf;
    ParentFirstLeaf.reserve(LevelIndices.size());

    //For each parent, get all its leaves and find the first one.
    std::set<size_t> LeavesOfParent;
    for(const size_t idxParent : LevelIndices)
    {
        //Get this parent's leaves
        LeavesOfParent.clear();

        const uint64_t tMinParent = Tree.nodes[idxParent].startTime();
        const uint64_t tMaxParent = Tree.nodes[idxParent].endTime();

        Tree.getLeaves(idxParent, tMinParent, tMaxParent, tMinParent, tMaxParent, LeavesOfParent);

        if (LeavesOfParent.size())
        {
            //Find first leaf according to the given order
            size_t MinOrderRow(NumLeaves);
            for(const size_t idxLeaf : LeavesOfParent)
            {
                if (LeafOrderMap.count(idxLeaf) == 0)
                {
                    LogInfoCustom("LayerOrder", "Leaf " << idxLeaf << " not found! Size = " << LeafOrderMap.size());
                    continue;
                }
                const size_t OrderRow = LeafOrderMap.at(idxLeaf);
                if (OrderRow < MinOrderRow) MinOrderRow = OrderRow;
            }

            //Add it to the sorting vector
            ParentFirstLeaf.push_back( std::make_pair(idxParent, MinOrderRow) );
        }
        else
        {
            //We are a child
            ParentFirstLeaf.push_back( std::make_pair(idxParent, LeafOrderMap.at(idxParent)) );
        }
    }

    //Sort parents according to their first child
    std::sort(ParentFirstLeaf.begin(), ParentFirstLeaf.end(),
              [&](const auto& a, const auto& b)
                {
                    return a.second < b.second;
                });

    //Condense to a layer order
    const size_t NumElems = ParentFirstLeaf.size();
    LayerOrder.resize(NumElems);
    for(size_t i(0);i<NumElems;i++)
    {
        LayerOrder[i] = ParentFirstLeaf[i].first;
    }
}

}

json TemporalTreeWriter::createJSON(std::shared_ptr<const TemporalTree> tree, const bool bNTG, const bool bNTGAddWeights, const bool bNTGAddOrder, double nTGScaleWeights)
{
    //Create a JSON from the tree
    json jAll;

    //Our version or Nested Tracking Graph?
    if (!bNTG)
    {
        json jNodes = json::array();
        for (auto& node : tree->nodes)
        {
            json jSingleNode;
            jSingleNode["name"] = node.name;
            jSingleNode["values"] = node.values;

            jNodes.push_back(jSingleNode);
        }

        jAll["nodes"] = jNodes;
        jAll["edgesHierarchy"] = tree->edgesHierarchy;
        jAll["edgesTime"] = tree->edgesTime;
        jAll["order"] = tree->order;
    }
    else
    {
        //Nested Tracking Graphs are stored and processed in a non-aggregated way
        //Node property w is responsible for the width of the band

        //Options like bNTGAddWeights, bNTGAddOrder, nTGScaleWeights are given as arguments to the function

        //The three big arrays of the data structure
        json jNodes;
        json jTemporalEdges;
        json jHierarchicalEdges;



        //De-aggregate
        TemporalTree NTGTree;
        tree->deaggregate(NTGTree);

        auto times = NTGTree.getTimes();

        for (auto time : times)
        {
            jHierarchicalEdges[std::to_string(time)] = json::object();
        }

        //Get a proper order
        TemporalTree::TTreeOrder Order(NTGTree.order);
        const size_t NumLeaves = Order.size();
        const bool bSaveWithOrder = (NumLeaves != 0) && bNTGAddOrder;
        TemporalTree::TTreeOrderMap OrderMap;
        treeorder::toOrderMap(OrderMap, Order);

        std::vector<size_t> LayerOrder;
        size_t Level(1);

        std::vector<size_t> LevelIndices;
        LevelIndices = NTGTree.getLevel(Level, LevelIndices, 0);
        while (!LevelIndices.empty())
        {
            //In which order will we run through the nodes of this hierarchy layer?
            // The default is the order in which they initially come
            std::vector<size_t>* pNodeIndexArray = &LevelIndices;

            //Create an order of nodes in this layer, but depending on the leaves layer
            if (bSaveWithOrder)
            {
                GetLayerOrder(NTGTree, OrderMap, LevelIndices, LayerOrder);
                //Run through nodes in our sorted order
                pNodeIndexArray = &LayerOrder;
            }

            //We count how many nodes we already written per time step. Needed for topological order.
            std::map<uint64_t, int> Stacks;

            //Order in which we run through the nodes
            const std::vector<size_t>& NodeIndexArray = *pNodeIndexArray;

            //Add to JSON nodes
            for (size_t i(0); i < NodeIndexArray.size(); i++)
            {
                //Shorthand
                const size_t& idxNode = NodeIndexArray[i];
                const TemporalTree::TNode& ThisNode = NTGTree.nodes[idxNode];
                const uint64_t NTGTime = ThisNode.values.cbegin()->first;

                //Prepare stack
                if (Stacks.count(NTGTime) == 0) Stacks[NTGTime] = 0;

                //Node
                json jSingleNode;
                jSingleNode["l"] = Level - 1;
                jSingleNode["t"] = NTGTime;
                if (bNTGAddWeights) jSingleNode["w"] = ThisNode.values.cbegin()->second * nTGScaleWeights;

                //Node Layout based on order
                // - a layout array property with x, y, w variables.
                if (bSaveWithOrder)
                {
                    json jLayout;
                    jLayout["x"] = NTGTime;
                    jLayout["y"] = Stacks[NTGTime];
                    Stacks[NTGTime]++;

                    //Get number of siblings
                    // - get parent
                    std::vector<size_t> Parents = NTGTree.getHierarchicalParents(idxNode);
                    // - get number of its children
                    const size_t NumSiblings = Parents.empty() ? 0 : NTGTree.getHierarchicalChildren(Parents[0]).size();
                    const float SiblingsFactor = (Level - 1 > 0) ? (1.0f / float(NumSiblings)) : 1.0f;

                    //Topological size
                    //jLayout["w"] = 0.3 * SiblingsFactor * float((MaxLevel+1) - Level) / float(MaxLevel);

                    //Data size
                    jLayout["w"] = ThisNode.values.cbegin()->second * nTGScaleWeights;

                    jSingleNode["layout"] = jLayout;
                }

                //Add this node to the list of all nodes
                jNodes[std::to_string(idxNode)] = jSingleNode;

                //Add temporal edges
                const auto& Succ = NTGTree.getTemporalSuccessors(idxNode);
                if (!Succ.empty())
                {
                    jTemporalEdges[std::to_string(Level - 1)][std::to_string(idxNode)] = Succ;
                }

                //Add hierarchical edges
                const auto& Children = NTGTree.getHierarchicalChildren(idxNode);
                if (!Children.empty())
                {
                    jHierarchicalEdges[std::to_string(NTGTime)][std::to_string(idxNode)] = Children;
                }
            }

            //Forward to the next level
            LevelIndices = NTGTree.getLevel(Level + 1, LevelIndices, Level);
            Level++;
        }

        //Join all arrays in one big json
        jAll["N"] = jNodes;
        jAll["ET"] = jTemporalEdges;
        jAll["EN"] = jHierarchicalEdges;
    }

    return jAll;
}


void TemporalTreeWriter::process()
{
    //Get filename and open file
    const std::string& Filename = propFilename.get();

    if (filesystem::fileExists(Filename) && !propOverwrite.get())
    {
        LogWarn("File already exists: " << Filename);
        return;
    }

    //Get the tree
    std::shared_ptr<const TemporalTree> InTree = portInTree.getData();


    std::ofstream outfile;
    outfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    try
    {
        outfile.open(Filename);
    }
    catch (const std::ofstream::failure& e)
    {  
        LogError("File could not be opened: " << Filename);
        LogError("  Error Code: " << e.code() << "    . " << e.what());
        return;
    }

    //Filetype
    const bool bASCII = propFilename.get().rfind(".tree") != -1;
    const bool bCBOR = propFilename.get().rfind(".cbortree") != -1;
    const bool bNTG = propFilename.get().rfind(".ntg") != -1;
    //const bool bMsgPack = !(bASCII || bCBOR);

    json jAll = createJSON(InTree, bNTG, propNTGAddWeights.get(), propNTGAddOrder.get(), propNTGScaleWeights.get());

    //Stream it out as ASCII or Binary
    try
    {
        if (bASCII || bNTG)
        {
            //ASCII
            if (propPrettyPrint.get()) outfile << std::setw(4);
            outfile << jAll << std::endl;
        }
        else if (bCBOR)
        {
            //Binary CBOR
            std::vector<std::uint8_t> v_cbor = json::to_cbor(jAll);
            outfile.write(reinterpret_cast<char*>(&v_cbor[0]), v_cbor.size());
        }
        else
        {
            //Binary MessagePack
            std::vector<std::uint8_t> v_pack = json::to_msgpack(jAll);
            outfile.write(reinterpret_cast<char*>(&v_pack[0]), v_pack.size());
        }
    }
    catch (const std::ofstream::failure& e)
    {  
        LogError("Error during save: " << Filename);
        LogError("  Error Code: " << e.code() << "    . " << e.what());
        return;
    }

    outfile.close();
}

} // namespace kth
} // namespace
