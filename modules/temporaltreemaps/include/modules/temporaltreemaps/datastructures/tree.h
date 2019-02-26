/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Monday, October 09, 2017 - 16:31:44
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <set>
#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/datatraits.h>

namespace inviwo
{
namespace kth
{

/** \class TemporalTree
    \brief Describes a tree data structure with time-dependent values at the nodes.

    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTree
{
//Friends
//Types
public:

    typedef std::map<uint64_t, float> TValueMap;

    typedef std::map<uint64_t, std::pair<float, float> > TDrawingLimitMap;

    typedef std::map<uint64_t, std::pair<vec3, vec3> > TCushionMap;

    typedef std::map<uint64_t, vec4> TColorPerTimeMap;

    ///Single node in a time-dependent tree. Can be inner node or leaf.
    struct TNode
    {
        ///Some application-specific text
        std::string name;

        ///Value of the node at discrete time points,
        ///at least at two points in time.
        ///This value is usually 0 when the node is an inner (parent) node,
        ///so that all actual values are held at the leaf nodes.
        ///If computeAccumulated has been called inner nodes
        ///hold values accumulated from the values of children
        ///and leaves are filled with values for each timestep
        TValueMap values;

        ///Drawing information for the node for each timestep;
        TDrawingLimitMap lowerLimit;
        TDrawingLimitMap upperLimit;
        TCushionMap cushion;
        ///Map has the same number of values as upperLimit, lowerLimit and cushion 
        TColorPerTimeMap colors;

        ///In case someone computes a single color for the entire leaf
        vec3 color;

        ///Simple constructor
        TNode()
            :name("")
            ,values()
            ,lowerLimit()
            ,upperLimit()
            ,cushion()
            ,colors()
        {
        }

        ///Element constructor
        TNode(const std::string& argname, const std::map<uint64_t, float>& argvalues = {})
            :name(argname)
            ,values(argvalues)
            ,lowerLimit()
            ,upperLimit()
            ,cushion()
            ,colors()
        {
        }

        ///Returns the time of the first element of this node
        ///or the maximum uint64_t value if the node does not have any values
        uint64_t startTime() const;

        ///Returns the time of the last element of this node 
        ///or the minimum value uint64_t value if the node does not have any values
        uint64_t endTime() const;

        ///Get the value of the node for a given time 
        float getValueAt(const uint64_t time) const;

        ///Add values from one map to another one with left neighbor interpolation 
        ///(Add values to add for all time points in between the current and next one)
        void fillWithLeftNeighborInterpolation(const std::set<uint64_t>& times)
        {
            fillWithLeftNeighborInterpolation(times, values);
        }

        static void fillWithLeftNeighborInterpolation(const std::set<uint64_t>& times, std::map<uint64_t, float>& values);

        static bool isOverlappingTemporally(uint64_t startTimeA, uint64_t endTimeA, uint64_t startTimeB, uint64_t endTimeB);

    };

    ///The type of edges
    typedef std::map<size_t, std::vector<size_t>> TAdjacency;

    ///Encodes a sorting of the leaves of a TemporalTree
    using TTreeOrder = std::vector<size_t>;

    ///Maps a leaf index to its order index
    using TTreeOrderMap = std::map<size_t, size_t>;

//Construction / Deconstruction
public:
    TemporalTree()
    {
    }

    virtual ~TemporalTree() = default;

//Methods
public:
	/** @name Access to Tree Elements

		Utility functions for accessing parts of the tree.
	*/
	//@{

    ///Get the indices for all nodes of a given level, where level 0 refers to the root node
    ///Optionally give nodes of a previous level to avoid recomputation when iterating through levels
    std::vector<size_t> getLevel(const size_t level, const std::vector<size_t>& prevLevelIndices, const size_t lastLevel) const;

    ///Returns the number of levels from the given node down to the lowest leaf. If this node is a leaf, the result is 0.
    size_t getNumLevels(const size_t nodeIndex) const;

    void splitTemporaryLeaves();

    void leafDepthStatistic(const size_t nodeIndex, std::vector<bool>& visited, const TAdjacency& edges,
        std::vector<size_t>& leafDepthCounters, size_t depth) const;

    ///Get the leave nodes of the tree. Leaves are nodes that do not have hierarchical children.
    std::vector<size_t> getLeaves() const;

    ///Get the leave nodes of the given subtree.
    ///Needs to be called with the starting and end time of the initial node.
    ///Leaves are nodes that do not have hierarchical children.
    void getLeaves(const size_t nodeIndex, const uint64_t initialStartTime, const uint64_t initalEndTime, uint64_t startTime, uint64_t endTime, std::set<size_t>& Leaves) const;

    ///Returns whether a node is a leaf or not.
    bool isLeaf(const size_t nodeIndex) const
    {return (edgesHierarchy.find(nodeIndex) == edgesHierarchy.end());}

    ///Returns the depth of a node
    size_t depth(const size_t nodeIndex) const;

    ///Returns the depth of a node
    size_t depthWithReverse(const size_t nodeIndex) const;

    ///Get the hierarchical children of a node given by its index.
    std::vector<size_t> getHierarchicalChildren(const size_t nodeIndex) const
    {return getEdgesFrom(nodeIndex, edgesHierarchy);}

    ///Get the hierarchical parents of a node given by its index. Parentship might change over time.
    std::vector<size_t> getHierarchicalParents(const size_t nodeIndex) const
    {return getEdgesTo(nodeIndex, edgesHierarchy);}

    std::vector<size_t> getHierarchicalParentsWithReverse(const size_t nodeIndex) const
    {return getEdgesFrom(nodeIndex, reverseEdgesHierachy);}

    ///Get the immediate temporal successors of a node given its index.
    std::vector<size_t> getTemporalSuccessors(const size_t nodeIndex) const
    {return getEdgesFrom(nodeIndex, edgesTime);}

    ///Get the immediate temporal predecessors of a node given its index.
    std::vector<size_t> getTemporalPredecessors(const size_t nodeIndex) const
    {return getEdgesTo(nodeIndex, edgesTime);}

    std::vector<size_t> getTemporalPredecessorsWithReverse(const size_t nodeIndex) const
    {return getEdgesFrom(nodeIndex, reverseEdgesTime);}

    // Sort vector of indices by time (first smaller startTime, then smaller endTime)
    void sortByTime(std::vector<size_t>& nodesIndices) const;

    // Sort vector of indices by time backwards (first larger endTime, then larger startTime)
    void sortByTimeBackwards(std::vector<size_t>& nodesIndices) const;

    ///Get the reverse map for edges (incoming temporal edges)
    std::map<size_t, std::vector<size_t> > getReverseEdges(const TAdjacency& Edges) const;

    void computeReverseEdges()
    {
        reverseEdgesTime = getReverseEdges(edgesTime);
        reverseEdgesHierachy = getReverseEdges(edgesHierarchy);
    }

    ///Returns number of edges in the given set of edges
    size_t getNumEdges(const TAdjacency& Edges) const;

    ///Returns number of hierarchy edges
    size_t getNumHierarchyEdges() const
    {return getNumEdges(edgesHierarchy);}

    ///Returns number of time edges
    size_t getNumTimeEdges() const
    {return getNumEdges(edgesTime);}

protected:
    ///Get the edges pointing away from node.
    std::vector<size_t> getEdgesFrom(const size_t nodeIndex, const TAdjacency& Edges) const;

    ///Get the edges pointing to a node.
    std::vector<size_t> getEdgesTo(const size_t nodeIndex, const TAdjacency& Edges) const;

    ///Compute components
    std::vector<unsigned int> computeComponents(const TAdjacency& Edges) const;

    //@}


	/** @name Extending the Tree

		Utility functions for extending the tree and adding data to it.
	*/
	//@{

public:

    /** Add a node to the tree.
    
        @returns the index of the newly created node.
    */
    size_t addNode(const TNode& node)
    {
        nodes.push_back(node);
        return nodes.size() - 1;
    }

    /** Add a node to the tree given by its parameters.

        @see addDataValue()
        @see finalizeTree()
    
        @returns the index of the newly created node.
    */
    size_t addNode(const std::string& name, const std::map<uint64_t, float>& values)
    {
        nodes.emplace_back(name, values);
        //if (values.size() < 1)
        //{
        //    nodes.back().values[0] = 0;
        //}
        //if (values.size() < 2)
        //{
            //nodes.back().values[1] = 0;
        //}
        return nodes.size() - 1;
    }

    ///Add a hierarchical edge to the tree
    void addHierarchyEdge(const size_t from, const size_t to)
    {
        //Add edge to the map (create either a new entry or update the entry for the from-key)
        auto itToAdd = edgesHierarchy.find(from);
        if (itToAdd == edgesHierarchy.end())
        {
            TAdjacency::mapped_type edgesToAdd = { to };
            edgesHierarchy.emplace(from, edgesToAdd);
        }
        else
        {
            (itToAdd->second).push_back(to);
        }
    }

    ///Add a node to the tree given by its parameters as a child of a node with given index,
    ///where the edge has a given life time, returns the index of the newly created node.
    size_t addChild(const size_t parent, const std::string& name, const std::map<uint64_t, float>& values)
    {
        const size_t child = this->addNode(name, values);
        this->addHierarchyEdge(parent, child);
        return child;
    }

    ///Add a temporal edge to the tree
    void addTemporalEdge(const size_t from, const size_t to)
    {
        // Add edge to the map (create either a new entry or update the entry for the from-key)
        std::map<size_t, std::vector<size_t> >::iterator itToAdd = edgesTime.find(from);
        if (itToAdd == edgesTime.end())
        {
            std::vector<size_t> edgesToAdd = { to };
            edgesTime.emplace(from, edgesToAdd);
        }
        else
        {
            (itToAdd->second).push_back(to);
        }
    }

    ///Adds a data value to a node.
    void addDataValue(const uint64_t Time, const size_t nodeIndex, const float dataValue)
    {
        TNode& Node = nodes[nodeIndex];
        Node.values.emplace(Time, dataValue);
    }

    ///Adds default parent timings with zero as data value in the given subtree.
    void addDefaultTimesForParents(const size_t nodeIndex)
    {
        uint64_t tMin, tMax;
        addDefaultTimesForParents(nodeIndex, tMin, tMax);
    }

    ///Adds default parent timings with zero as data value in the given subtree. @returns the time range for the given node.
    void addDefaultTimesForParents(const size_t nodeIndex, uint64_t& tMin, uint64_t& tMax);

    ///Adds one additional data value to each node, unless it has already one at the given time.
    ///Useful at the end of construction.
    ///We can specify a set of nodes that are not updated (usually ones that are not active anymore)
    void finalizeTree(const uint64_t Time, std::set<size_t> excludeNodes = {})
    {
        for(size_t nodeIndex(0);nodeIndex<nodes.size();nodeIndex++)
        {
            auto& node = nodes[nodeIndex];
            // Make sure we actually want to process this node
            if (excludeNodes.find(nodeIndex) != excludeNodes.end())
            {
                continue;
            }
            //Any value here?
            if (node.values.size() < 1)
            {
                //Nope, let's get outa here. This is a programming error.
                throw Exception("Node (\"" + node.name + "\") with no data values.", IvwContext);
            }
            if (node.values.find(Time) == node.values.end()) //if not found at this time
            {
                //Get the last data value
                const float LastData = node.values.rbegin()->second;

                //Finialize the node with this value at the given time.
                node.values.emplace(Time, LastData);
            }
        }
    }

    //@}

	/** @name Computation on the Tree

		Compute inner values.
	*/
	//@{

    ///Compute accumulated values for the tree, initialization has to happen prior to this 
    void computeAccumulated(const bool justLeaves = true)
    {
        std::vector<bool> processed(nodes.size(), false);
        computeAccumulated(0, getTimes(), processed, justLeaves);
    }

    // Shorthand for times
    std::set<uint64_t> getTimes() const
    {
        std::set<uint64_t> times;
        getTimes(0, times);
        return times;
    }

    std::map<uint64_t, float> computeAccumulatedRootOnly(const std::set<uint64_t>& times) const;

    /** Computes all nodes taking part in splits/merges.
    
        @returns Number of clusters and fills the @c Cluster variable:
        for each node, it contains whether it takes part in a split/merge cluster (>= 0)
        and if so, which cluster id it is.
        The first part of the pair identifies this for the left part of the node,
        the second part for the end of the node.
    */
    size_t getConstraintClusters(std::vector<std::pair<int, int>>& Clusters) const;

    ///Computes the temporal extent of a subtree in a recursive manner going to the leaves.
    void getMinMaxTime(const size_t nodeIndex, uint64_t& tMin, uint64_t& tMax) const;

    ///Computes the temporal extent of a subtree from the values of the immediate children.
    void getMinMaxTimeShallow(const size_t nodeIndex, uint64_t& tMin, uint64_t& tMax) const;

    ///Computes all time points at which events in the subtree occur
    void getTimes(const size_t subtreeIndex, std::set<uint64_t>& times) const;

    //@}

	/** @name Consistency, Aggregation, and Slicing of the Tree
	*/
	//@{

    /** Consistency check of the entire tree.

        - Make sure all inner nodes (parents) exist as long or longer than their children.
        - Make sure only siblings split and merge, i.e., have the same parent and start/end at the same time).

        @returns True, if the tree is consistent. False, otherwise.
    */
    bool checkConsistency() const;

    ///Deaggregate the tree
    void deaggregate(TemporalTree& tree) const;

    ///Returns an aggregated version of this tree 
    ///We aggregate nodes if they directly correspond to each other in time
    ///thus are not part of a split or merge
    TemporalTree aggregate() const;

    /* LATER, WHEN WE ACTUALLY NEED IT
    ///Compute the tree at a given time
    ///The tree is empty if no nodes exist at that time
    TemporalTree getHierarchyAt(const uint64_t time, const bool accumulate, const uint64_t deltaTime) const;

    */

    //@}

private:

    ///Compute accumulated values from values at the leafs for a subtree given its root
    ///Initialize each map with the time point in the set
    void computeAccumulated(const size_t subtreeIndex, const std::set<uint64_t>& times,
            std::vector<bool>& processed, const bool justLeaves);
  

//Attributes
public:
    ///All nodes of the tree. The first one is the root.
    std::vector<TNode> nodes;

    ///Hierarchical edges, edges are valid for a specific time and thus 
    ///come with a start and end time
    TAdjacency edgesHierarchy;
    
    ///Cache for reverse hierarchy edges
    TAdjacency reverseEdgesHierachy;

    ///Edges in time encoding merges and splits (essentially active at discrete time points)
    ///A node can either have a single successor in time, 
    ///then is merges with a sibling, or it has multiple successors
    ///then it has split into multiple siblings
    ///For non-aggregated version a single successor is also possible for a usual node
    TAdjacency edgesTime;

    ///Cache for reverse temporal edges
    TAdjacency reverseEdgesTime;

    ///Order on the leaves of this tree
    TTreeOrder order;
};

} // namespace kth

template <>
struct DataTraits<kth::TemporalTree>
{
    static std::string classIdentifier() { return "org.inviwo.TemporalTree"; }
    static std::string dataName() { return "TemporalTree"; }
    static uvec3 colorCode() { return uvec3(34, 139, 34); }
    static Document info(const kth::TemporalTree& data)
    {
        std::ostringstream oss;
        oss << "Tree with " << data.nodes.size() << " nodes, "
                            << data.edgesHierarchy.size() << " hierarchical edge groups, "
                            << data.edgesTime.size() << " temporal edge groups, and lifetime ["
                            << data.nodes[0].startTime() << ", " << data.nodes[0].endTime() << "].";
        Document doc;
        doc.append("p", oss.str());
        return doc;
    }
};

} // namespace
