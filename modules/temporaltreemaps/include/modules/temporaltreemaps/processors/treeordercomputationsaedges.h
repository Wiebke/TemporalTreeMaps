/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Köpp
 *  Init    : Wednesday, November 22, 2017 - 18:46:01
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <modules/temporaltreemaps/datastructures/treeorder.h>
#include <modules/temporaltreemaps/processors/treeordercomputationsa.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeOrderComputationSAEdges, Tree Order Computation}
    ![](org.inviwo.TemporalTreeOrderComputationSAEdges.png?classIdentifier=org.inviwo.TemporalTreeOrderComputationSAEdges)

    Optimizes the order of all leaves of the tree, 
    ideally such that :
       - split and merge constraints are respected 
       - hierarchal constraints are respected
       - the order is visually appealing (low 'wiggle')
    Initially leaves are ordered according to their id.
    
    ### Inports
      * __<inTree>__ Tree for which we compute a leaf order.
    
    ### Outports
      * __<outTree>__ Tree for which we computed the order with the 
                      the order variable set 

    ### Properties
      * __<Prop1>__ <description>.
*/


/** \class TemporalTreeOrderComputationSAEdges
    \brief Creates a sorting of the leaves in a temportal tree

    @author Tino Weinkauf and Wiebke Köpp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeOrderComputationSAEdges : public TemporalTreeSimulatedAnnealing
{ 
//Friends
//Types
public:
    enum THeuristicType
    {
        NoMergesOrSplits,
        MergesOnly, // in the leaves and no merges/splits otherwise
        SplitsOnly, // in the leaves and no merges/splits otherwise
        MergesAndSplits, // or one type higher up
    };

//Construction / Deconstruction
public:
    TemporalTreeOrderComputationSAEdges();
    virtual ~TemporalTreeOrderComputationSAEdges() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:

    ///Neighbor Solution from the current state 
    void neighborSolution() override;

    ///Initalize everything
    void initializeResources() override;

    ///Reset only statistic things and settings
    void restart() override;

    void setLastToCurrent() override;

    void setCurrentToLast() override;

    void setBest() override;

    void prepareNextStep() override;

    ///Our main computation function (does nothing)
    void process() override;

    void printStatistic(std::shared_ptr<TemporalTree> tree) const;

private:
    /// Add an edge to the optimization edges
    void addEdge(const size_t from, const size_t to);

    /// Traverse the temporal edges of the original tree from a given node with dfs
    void dfsTime(std::shared_ptr<const TemporalTree> tree, size_t nodeIndex, std::vector<bool>& visited);

    /// Traverse the hierarchical edges of the original tree from a given node with dfs
    /// Only visit nodes that have not been visisted by either hierarchy or temporal dfs
    void dfsHierarchy(std::shared_ptr<const TemporalTree> tree, size_t nodeIndex,
        std::vector<bool>& visitedHierarchy, std::vector<bool>& visitedTime);

    /// Delete edges that will not contribute to the final order
    /// e.g. subtrees that do not contain any original tree leaves
    void cleanEdges(std::shared_ptr<const TemporalTree> tree);

//Ports
public:


//Properties
public:

//Attributes
protected:

    /// Nodes at which it is possible to do a switch because
    /// there is more than one connection at those nodes
    std::vector<size_t> activeNodes;

    /// The current heuristic edges
    TemporalTree::TAdjacency currentEdges;
    /// The current best heuristic edges
    TemporalTree::TAdjacency bestEdges;
    /// The last heuristic edges
    TemporalTree::TAdjacency lastEdges;

    /// The current reverse heuristic edges (only used in initial building)
    TemporalTree::TAdjacency currentReverseEdges;

    /// Case for building heuristic edges
    THeuristicType initCase;

    /// Random distribution to choose one of the active nodes
    std::uniform_int_distribution<int> chooseNode;
};


} // namespace kth
} // namespace
