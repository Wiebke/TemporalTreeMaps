/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Friday, December 01, 2017 - 08:55:23
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treegeneratefromtrackinggraph.h>
#include <modules/tools/performancetimer.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

#ifndef __clang__
    #include <omp.h>
#endif

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeGenerateFromTrackingGraph::processorInfo_
{
    "org.inviwo.TemporalTreeGenerateFromTrackingGraph",      // Class identifier
    "Nested Tracking Graphs",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeGenerateFromTrackingGraph::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeGenerateFromTrackingGraph::TemporalTreeGenerateFromTrackingGraph()
    :Processor()
    ,portSeries("VolumeSeries")
    ,portOutTree("OutTree")
    ,portOutSegmentationExample("OutSegmentationExample")
	,propTimeStepExample("TimeStepExample", "Time Step Example")
    ,propColormap("Colormap", "Colormap", TransferFunction({{0.0f, vec4(0.2f, 0.2f, 0.2f, 1.0f)}, {1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f)}}))
    ,propRelation("relation", "Relation")
    ,propHierarchyLevelGroup("HierarchyLevelGroup", "Hierarchy")
    ,propNumLevels("NumLevels", "Number of Levels", 0, 0, 10, 1)
    ,propGhostChildrenCreate("CreateGhostChildren", "Create Ghost Children", true)
    ,propGhostChildrenTrackLikeParents("GhostChildrenTrackLikeParents", "Ghost Children Track Like Parents", true)
    ,propParallel("Parallel", "Parallel Execution of the Tracking", true, InvalidationLevel::Valid)
    ,propAction("Action", "Start Tracking")
    ,CurrentValueRange(0, 1)
{
    //Change a few things when new data comes in:
    // - enable/disable example time step slider
    // - set right min/max for data range
    portSeries.onChange([&]()
    {
        if (portSeries.getData())
        {
            const size_t NumFiles = portSeries.getData()->GetNumFiles();
            propTimeStepExample.setMaxValue(NumFiles - 1);
            const auto Volume = portSeries.getData()->GetVolume(0);
            if (Volume)
            {
                CurrentValueRange = Volume->dataMap_.valueRange;
                UpdateMinMaxAllSliders();
            }
        }
        else
        {
            propTimeStepExample.setMaxValue(propTimeStepExample.getMinValue());
        }
    });
    addPort(portSeries);


    addPort(portOutTree);
    addPort(portOutSegmentationExample);

    addProperty(propTimeStepExample);
    addProperty(propColormap);

    propRelation.addOption("smaller", "Datavalue < Isovalue", SMALLER);
    propRelation.addOption("smallerEqual", "Datavalue <= Isovalue", SMALLEREQUAL);
    propRelation.addOption("greaterEqual", "Datavalue >= Isovalue", GREATEREQUAL);
    propRelation.addOption("greater", "Datavalue > Isovalue", GREATER);
    addProperty(propRelation);

    //Isovalues / Levels
    addProperty(propHierarchyLevelGroup);
    //propNumLevels.set(1, 0, 10, 1); //gives comilation error, since a >= operator is missing
    propHierarchyLevelGroup.addProperty(propNumLevels);
    propNumLevels.onChange([&]()
    {
        const size_t nDesiredLevels = propNumLevels.get();
        const size_t nCurrentLevels = propHierarchyLevelGroup.size() - 1;

        //Add new levels
        for(size_t i(nCurrentLevels);i<nDesiredLevels;i++)
        {
            DoubleProperty* ppropIsovalue = new DoubleProperty("Isovalue" + std::to_string(i+1), "Isovalue " + std::to_string(i+1), 0);
            UpdateMinMaxSlider(*ppropIsovalue);
            propHierarchyLevelGroup.addProperty(ppropIsovalue, true);
        }

        //Delete old levels
        for(size_t i(nCurrentLevels);i>nDesiredLevels;i--)
        {
            propHierarchyLevelGroup.removeProperty(propHierarchyLevelGroup[i]);
        }
    });
    propNumLevels.set(1); //Add one slider

    addProperty(propGhostChildrenCreate);
    propGhostChildrenCreate.setChecked(true);
    propGhostChildrenCreate.addProperty(propGhostChildrenTrackLikeParents);
    propGhostChildrenCreate.onChange([&]()
    {
        propGhostChildrenCreate.setCollapsed(!propGhostChildrenCreate);
        propGhostChildrenTrackLikeParents.setReadOnly(!propGhostChildrenCreate);
    });

    addProperty(propParallel);

    propAction.onChange([&]()
    {
        PerformanceTimer Timer;
        GenerateNestedTrackingGraphs();
        LogInfo("Nested Tracking Graph with " << pOutTree->nodes.size() << " nodes, "
                            << pOutTree->edgesHierarchy.size() << " hierarchical edge groups, and "
                            << pOutTree->edgesTime.size() << " temporal edge groups."
                            " Needed " << Timer.ElapsedTime() << " seconds.");
    });
    addProperty(propAction);
}

namespace
{
template<typename TPredicate, typename TContentType>
void EvalPredicate(const VolumeRAM& Data, const double Isovalue, TPredicate Predicate,
                   const TContentType& TrueValue, TContentType* Visited)
{
    const size3_t& Dims = Data.getDimensions();

    ////Note: Visited is considered to be of the right size already!
    //#ifdef IVW_DEBUG
    //    const size_t NumVertices = Dims[0] * Dims[1] * Dims[2];
    //    Visited.size() == NumVertices;
    //#endif

    //Run over the data and see if it is larger/smaller than the isovalue
    size_t LinearIdx(0);
    size3_t Pos;
    for (Pos[2]=0; Pos[2] < Dims[2]; Pos[2]++)
    {
        for (Pos[1]=0; Pos[1] < Dims[1]; Pos[1]++)
        {
            for (Pos[0]=0; Pos[0] < Dims[0]; Pos[0]++,LinearIdx++)
            {
                if (Predicate(Data.getAsDouble(Pos), Isovalue))
                {
                    Visited[LinearIdx] = TrueValue;
                }
            }
        }
    }
}

template<typename TContentType>
void EvalPredicate(const VolumeRAM& Data, const double Isovalue,
                   const TemporalTreeGenerateFromTrackingGraph::RELATION& HowToCompare,
                   const TContentType& TrueValue, TContentType* Visited)
{
    //Mark every outside pixel as visited
    switch (HowToCompare)
    {
        case TemporalTreeGenerateFromTrackingGraph::RELATION::GREATER:
        {
            //EvalPredicate(Data, Isovalue, [](const double& a, const double& b){return a > b;}, Visited);
            EvalPredicate(Data, Isovalue, std::greater<double>(), TrueValue, Visited);
            break;
        }

        case TemporalTreeGenerateFromTrackingGraph::RELATION::GREATEREQUAL:
        {
            EvalPredicate(Data, Isovalue, std::greater_equal<double>(), TrueValue, Visited);
            break;
        }

        case TemporalTreeGenerateFromTrackingGraph::RELATION::SMALLEREQUAL:
        {
            EvalPredicate(Data, Isovalue, std::less_equal<double>(), TrueValue, Visited);
            break;
        }

        default:
        case TemporalTreeGenerateFromTrackingGraph::RELATION::SMALLER:
        {
            EvalPredicate(Data, Isovalue, std::less<double>(), TrueValue, Visited);
            break;
        }
    }
}


int64_t GetLinearIndex(const int64_t i, const int64_t j, const int64_t k, const size3_t& Dims)
{
	return ((k * Dims[1] + j) * Dims[0] + i);
}

glm::i64vec3 RecoverLinearIndex(const size3_t& Dims, size_t idx)
{
	return glm::i64vec3(idx % Dims.x, (idx / Dims.x) % Dims.y, idx / (Dims.x * Dims.y));;
}


typedef std::vector<std::vector<size_t>> TLayerComponents;
typedef std::multimap<size_t, size_t> TOverlap;

void GetComponents(const VolumeRAM& Data, const double Isovalue,
                    const TemporalTreeGenerateFromTrackingGraph::RELATION& HowToCompare, TLayerComponents& Components)
{
    //How many vertices do we have?
    const size3_t& Dims = Data.getDimensions();
    const size_t NumVertices = Dims[0] * Dims[1] * Dims[2];

    //Visitation map
    std::vector<char> Visited(NumVertices, true);
    EvalPredicate(Data, Isovalue, HowToCompare, (char)0, &Visited[0]);

    //Flood fill components
    Components.clear();
    for(size_t idxVertex(0);idxVertex<NumVertices;idxVertex++)
    {
        //Already visited?
        if (Visited[idxVertex]) continue;

        //Start a new component here
        Components.emplace_back();
        std::vector<size_t>& ThisComponent = Components.back();
        ThisComponent.push_back(idxVertex);
        // - FIFO queue
        std::queue<int64_t> Q;
        Q.push((int64_t)idxVertex);
        Visited[idxVertex] = true;

        //Flood-fill until queue is empty
        while(!Q.empty())
        {
            //Current Vertex
            const int64_t idxCurrent = Q.front();
            Q.pop();

            //Add neighbors to queue and to the component
            glm::i64vec3 Current = RecoverLinearIndex(Dims, idxCurrent);
            glm::i64vec3 Left = Current - glm::i64vec3(1,1,1);
            glm::i64vec3 Right = Current + glm::i64vec3(1,1,1);
            if (Left.x < 0) Left.x = 0;
            if (Left.y < 0) Left.y = 0;
            if (Left.z < 0) Left.z = 0;
            if (Right.x >= (int64_t)Dims[0]) Right.x = Dims[0] - 1;
            if (Right.y >= (int64_t)Dims[1]) Right.y = Dims[1] - 1;
            if (Right.z >= (int64_t)Dims[2]) Right.z = Dims[2] - 1;

            for (int64_t k = Left.z; k <= Right.z; k++)
            {
                for (int64_t j = Left.y; j <= Right.y; j++)
                {
                    for (int64_t i = Left.x; i <= Right.x; i++)
                    {
                        const int64_t NeighbourLinearIdx = GetLinearIndex(i, j, k, Dims);
                        const size_t NeighbourLinearIdx_t = (size_t)NeighbourLinearIdx;

                        if (Visited[NeighbourLinearIdx_t]) continue;

                        Q.push(NeighbourLinearIdx);
                        ThisComponent.push_back(NeighbourLinearIdx_t);
                        Visited[NeighbourLinearIdx_t] = true;
                    }
                }
            }
        }// end of Q

        //Sorting makes it easier to compute the overlap between components
        std::sort(ThisComponent.begin(), ThisComponent.end());
    }
}


void DetectOverlap(const TLayerComponents& Left, const TLayerComponents& Right,
                   TOverlap& LeftToRight, TOverlap& RightToLeft)
{
    //Run over all possible component pairs (Left, Right) and detect overlap
    for(size_t i(0);i<Left.size();i++)
    {
        for(size_t j(0);j<Right.size();j++)
        {
            const std::vector<size_t>& LComp = Left[i];
            const std::vector<size_t>& RComp = Right[j];

            // Iterate through both SORTED lists simultaneously
            auto itL = LComp.begin();
            auto itR = RComp.begin();
            while (itL != LComp.end() && itR != RComp.end())
            {
                if (*itL < *itR)
                {
                    itL++;
                }
                else if (*itR < *itL)
                {
                    itR++;
                }
                else
                {
                    //Element is in both lists: we have overlap!
                    LeftToRight.emplace(i, j);
                    RightToLeft.emplace(j, i);
                    break;
                }
            }
        }
    }
}


void DetectOverlapClusters(const TOverlap& LeftToRight, const TOverlap& RightToLeft,
                           const TLayerComponents& LeftComponents, const TLayerComponents& RightComponents,
                           std::vector<float>& LeftNewSizes)
{
    LeftNewSizes.resize(LeftComponents.size());

    std::vector<size_t> LeftComponentCluster;
    std::vector<size_t> RightComponentCluster;
    std::vector<bool> bVisitedLeft(LeftComponents.size(), false);
    std::vector<bool> bVisitedRight(RightComponents.size(), false);
    for(size_t i(0);i<LeftComponents.size();i++)
    {
        if (bVisitedLeft[i]) continue;

        //Start new component
        std::queue<std::pair<bool, size_t>> Q;
        Q.emplace(true, i);
        bVisitedLeft[i] = true;
        LeftComponentCluster.clear();
        RightComponentCluster.clear();
        LeftComponentCluster.push_back(i);

        //Visit connected neighbors
        while (!Q.empty())
        {
            //Who is first?
            const auto& Front = Q.front();

            //The grass is always greener on the other side!
            const TOverlap& ToTheGreenerSide = Front.first ? LeftToRight : RightToLeft;
            std::vector<bool>& bVisitedOnTheGreenerSide = Front.first ? bVisitedRight : bVisitedLeft;
            std::vector<size_t>& GreenerComponentCluster = Front.first ? RightComponentCluster : LeftComponentCluster;

            //Add the neighbors to the Queue
            auto Range = ToTheGreenerSide.equal_range(Front.second);
            for(auto itNeigh(Range.first);itNeigh!=Range.second;itNeigh++)
            {
                if (!bVisitedOnTheGreenerSide[itNeigh->second])
                {
                    Q.emplace(!Front.first, itNeigh->second);
                    bVisitedOnTheGreenerSide[itNeigh->second] = true;
                    GreenerComponentCluster.push_back(itNeigh->second);
                }
            }

            //Remove current point from the queue
            Q.pop();
        }

        //We now have a left and a right component cluster that track to each other in all possible forms (splits & merges in any combinations)
        // The left side will be temporally extended to the right side.
        // To do so, it needs a size on the right side that equals the right cluster.
        // This is based on the fractions of the left side and the total sum on the right sum.
        // - get right sum
        size_t RightSum(0);
        std::for_each(RightComponentCluster.begin(), RightComponentCluster.end(), [&](const size_t& id){ RightSum += RightComponents[id].size(); });
        // - get left sum
        size_t LeftSum(0);
        std::for_each(LeftComponentCluster.begin(), LeftComponentCluster.end(), [&](const size_t& id){ LeftSum += LeftComponents[id].size(); });
        // - compute new sizes
        const float LRFactor(float(RightSum) / float(LeftSum));
        std::for_each(LeftComponentCluster.begin(), LeftComponentCluster.end(), [&](const size_t& id){ LeftNewSizes[id] = LRFactor * float(LeftComponents[id].size()); });
    }
}


///Add the parent's fat as a new ghost child
void AddGhostChildren(const bool bGhostChildrenTrackLikeParents, TemporalTree& Tree)
{
    //Visit each node, find all its children:
    // the difference between the children's sum and the parent is the size of the ghost
    const size_t NumNodes = Tree.nodes.size();
    std::vector<std::map<uint64_t, float>> AllGhostValueVectors(NumNodes);
    for(size_t i(0);i<NumNodes;i++)
    {
        //The current node...
        auto& Node = Tree.nodes[i];
        //...and its children
        const auto& Children = Tree.getHierarchicalChildren(i);

        //Add ghosts only if this node has children. Otherwise, it is a leaf already and does not need a layer below itself.
        if (Children.empty()) continue;

        //Through the time steps of the parent
        std::map<uint64_t, float>& GhostValues = AllGhostValueVectors[i];
        for(auto& ParentTimeValue : Node.values)
        {
            //Sum up the children at this point in time.
            float ChildrenSum(0);
            for(const auto& idChild : Children)
            {
                // If we only sum up values, we will count merges and splits twice
                // Thus if we have a temporal sucessor, do not count the ending value
                if (Tree.nodes[idChild].endTime() == ParentTimeValue.first && 
                    !Tree.getTemporalSuccessors(idChild).empty())
                ChildrenSum += Tree.nodes[idChild].getValueAt(ParentTimeValue.first);
            }
            ivwAssert(ChildrenSum <= ParentTimeValue.second, "Children sum up to more than the parent.");

            //Add Ghost value
            GhostValues.emplace(ParentTimeValue.first, ParentTimeValue.second - ChildrenSum);
        }
    }

    //Actually add the ghost
    std::vector<size_t> GhostIDs(NumNodes);
    for(size_t i(0);i<NumNodes;i++)
    {
        const std::map<uint64_t, float>& GhostValues = AllGhostValueVectors[i];

        //Add Ghost child
        if (!GhostValues.empty())
        {
            GhostIDs[i] = Tree.addChild(i, "Ghost of " + std::to_string(i), GhostValues);
        }
        else
        {
            GhostIDs[i] = 0; //A ghost can never have ID zero, since we need to have some proper node first before that one can have a ghost
        }
    }

    //Track like the parents
    if (bGhostChildrenTrackLikeParents)
    {
        for(size_t i(0);i<NumNodes;i++)
        {
            if (GhostIDs[i] > 0)
            {
                const auto& ParentSucc = Tree.getTemporalSuccessors(i);
                for(const auto& Parent : ParentSucc)
                {
                    if (GhostIDs[Parent] > 0)
                    {
                        Tree.addTemporalEdge(GhostIDs[i], GhostIDs[Parent]);
                    }
                }

                //This should never trigger, but whatever
                const auto& ParentPred = Tree.getTemporalPredecessors(i);
                for(const auto& Parent : ParentPred)
                {
                    if (GhostIDs[Parent] > 0)
                    {
                        const auto& itEdges = Tree.edgesTime.find(GhostIDs[Parent]);
                        if (itEdges == Tree.edgesTime.end()
                            || std::find(itEdges->second.begin(), itEdges->second.end(), GhostIDs[i]) == itEdges->second.end())
                        {
                            Tree.addTemporalEdge(GhostIDs[Parent], GhostIDs[i]);
                        }
                    }
                }
            }
        }
    }
}

};



void TemporalTreeGenerateFromTrackingGraph::UpdateMinMaxAllSliders()
{
    //Get all sliders and set min/max
    auto IsoSliders = propHierarchyLevelGroup.getPropertiesByType<DoubleProperty>();
    for(auto Slider : IsoSliders)
    {
        UpdateMinMaxSlider(*Slider);
    }
}

void TemporalTreeGenerateFromTrackingGraph::UpdateMinMaxSlider(DoubleProperty& Slider)
{
    const double& Val = Slider.get();
    Slider.setMinValue(Val < CurrentValueRange[0] ? Val : CurrentValueRange[0]);
    Slider.setMaxValue(Val > CurrentValueRange[1] ? Val : CurrentValueRange[1]);
}


std::shared_ptr<Volume> TemporalTreeGenerateFromTrackingGraph::CreateOrReuseResultVolume(std::shared_ptr<Volume> pInVolume)
{
    if (!pOutExampleVolume)
    {
        pOutExampleVolume = std::make_shared<Volume>(pInVolume->getDimensions(),
                                 DataFormatBase::get(DataFormatId::Vec3UInt8));
        if (!pOutExampleVolume) return pOutExampleVolume;
    }

    //Dimension
    if (pOutExampleVolume->getDimensions() != pInVolume->getDimensions())
    {
        pOutExampleVolume->setDimensions(pInVolume->getDimensions());
    }

    //Other aspects
    pOutExampleVolume->setOffset(pInVolume->getOffset());
    pOutExampleVolume->setBasis(pInVolume->getBasis());
    pOutExampleVolume->setModelMatrix(pInVolume->getModelMatrix());
    pOutExampleVolume->setWorldMatrix(pInVolume->getWorldMatrix());

    return pOutExampleVolume;
}


std::shared_ptr<TemporalTree> TemporalTreeGenerateFromTrackingGraph::CreateOrReuseResultTree()
{
    if (!pOutTree) pOutTree = std::make_shared<TemporalTree>();

    if (pOutTree)
    {
        pOutTree->nodes.clear();
        pOutTree->edgesHierarchy.clear();
        pOutTree->edgesTime.clear();
    }

    return pOutTree;
}


void TemporalTreeGenerateFromTrackingGraph::GetIsovaluesSorted(std::vector<double>& Isovalues)
{
    Isovalues.clear();

    //Get isovalues
    if (propHierarchyLevelGroup.size() < 1) return;
    const size_t NumIsovalues = propHierarchyLevelGroup.size() - 1;
    if (NumIsovalues < 1) return;
    Isovalues.resize(NumIsovalues);
    for(int i(0);i<NumIsovalues;i++)
    {
        Isovalues[i] = ((DoubleProperty*)(propHierarchyLevelGroup[i+1]))->get();
    }

    //Sort isovalues to have the order of the levels correctly
    const RELATION HowToCompare = propRelation.get();
    if (HowToCompare == RELATION::SMALLER || HowToCompare == RELATION::SMALLEREQUAL)
    {
        std::sort(Isovalues.begin(), Isovalues.end(), std::greater<double>());
    }
    else
    {
        std::sort(Isovalues.begin(), Isovalues.end(), std::less<double>());
    }
}


void TemporalTreeGenerateFromTrackingGraph::GenerateNestedTrackingGraphs()
{
    //Create a tree
    std::shared_ptr<TemporalTree> pOutputTree = CreateOrReuseResultTree();
    // - shorthand
    TemporalTree& OutTree = *pOutputTree;
    // - root
    const size_t idRoot = OutTree.addNode("Root", {});

    //Get Input
    auto pSeries = portSeries.getData();
    if (!pSeries) return;
    const size_t NumFiles = pSeries->GetNumFiles();

    //Get isovalues
    std::vector<double> Isovalues;
    GetIsovaluesSorted(Isovalues);
    const size_t NumIsovalues = Isovalues.size();
    if (NumIsovalues < 1) return;

    //Get other params
    const RELATION HowToCompare = propRelation.get();

    //Memory to store components
    std::vector<TLayerComponents> AllLayerComponents[2];
    AllLayerComponents[0].resize(NumIsovalues);
    AllLayerComponents[1].resize(NumIsovalues);

    //Mapping between components and tree
    typedef std::map<std::pair<size_t, size_t>, size_t> TLayerComponentToTreeID;
    TLayerComponentToTreeID AllLayerComponentToTreeID[2];

    //Track over all files
    for(int i(0);i<NumFiles;i++)
    {
        //Swap the buffers
        std::vector<TLayerComponents>& NowAllLayerComponents = AllLayerComponents[i%2];
        std::vector<TLayerComponents>& PreviousAllLayerComponents = AllLayerComponents[(i+1)%2];
        TLayerComponentToTreeID& NowLayerComponentToTreeID = AllLayerComponentToTreeID[i%2];
        TLayerComponentToTreeID& PreviousLayerComponentToTreeID = AllLayerComponentToTreeID[(i+1)%2];

        //Get input data
        std::shared_ptr<Volume> pVol = pSeries->GetVolume(i);
        if (!pVol) return;

        //Get Representation
        const VolumeRAM* pData = pVol->getRepresentation<VolumeRAM>();
        if (!pData) return;

        //Temporal overlap maps
        std::vector<TOverlap> PreviousToNowAllLayer(NumIsovalues);
        std::vector<TOverlap> NowToPreviousAllLayer(NumIsovalues);

        //Segment the layers and compute overlap
        #ifndef __clang__
            omp_set_num_threads(std::min(std::thread::hardware_concurrency(), (unsigned int)NumIsovalues));
        #endif    
        #pragma omp parallel for schedule(static, 1) if (propParallel.get())
        for(int j(0);j<NumIsovalues;j++)
        {
            //Shorthand
            TLayerComponents& NowLayerComponents = NowAllLayerComponents[j];

            //This loop computes components and finds their overlap.
            //Timing on the SquareCylinder data set (> 500 scalar fields): 
            //    Time for components: 129.16 seconds.
            //    Time for overlap: 0.183867 seconds.
            //    Total time needed 130.888 seconds.
            //In other words, the components take the longest time. The jury is out on whether this can be reduced.

            //Get the components of this layer
            NowLayerComponents.clear();
            GetComponents(*pData, Isovalues[j], HowToCompare, NowLayerComponents);
            //LogInfo("Time: " << i << " Iso: " << Isovalues[j] << " Number of Components: " << NowLayerComponents.size());

            //Find overlap between previous and this time step, i.e., the set of all edges.
            if (i > 0)
            {
                TLayerComponents& PreviousLayerComponents = PreviousAllLayerComponents[j];
                TOverlap& PreviousToNow = PreviousToNowAllLayer[j];
                TOverlap& NowToPrevious = NowToPreviousAllLayer[j];
                DetectOverlap(PreviousLayerComponents, NowLayerComponents, PreviousToNow, NowToPrevious);
            }
        }

        //Hierarchical overlap maps
        std::vector<TOverlap> ParentToChildAllLayer(NumIsovalues-1);
        std::vector<TOverlap> ChildToParentAllLayer(NumIsovalues-1);
        //Find overlap between hierarchical layers
        for(size_t j(0);j<NumIsovalues-1;j++)
        {
            //Shorthand
            TLayerComponents& ParentLayerComponents = NowAllLayerComponents[j];
            TLayerComponents& ChildLayerComponents = NowAllLayerComponents[j+1];

            TOverlap& ParentToChild = ParentToChildAllLayer[j];
            TOverlap& ChildToParent = ChildToParentAllLayer[j];
            DetectOverlap(ParentLayerComponents, ChildLayerComponents, ParentToChild, ChildToParent);
        }


        //Clear the mapping between components and tree nodes, since we're just about to create this.
        NowLayerComponentToTreeID.clear();

        //Tracking
        for(size_t j(0);j<NumIsovalues;j++)
        {
            //Shorthands Time
            TOverlap& PreviousToNow = PreviousToNowAllLayer[j];
            TOverlap& NowToPrevious = NowToPreviousAllLayer[j];
            //Shorthands Hierarchy
            TLayerComponents& PreviousChildLayerComponents = PreviousAllLayerComponents[j];
            TLayerComponents& NowChildLayerComponents = NowAllLayerComponents[j];

            std::vector<float> PreviousNewSizes;
            DetectOverlapClusters(PreviousToNow, NowToPrevious, PreviousChildLayerComponents, NowChildLayerComponents, PreviousNewSizes);

            //Whether a component has been handled by the tracking code in Stage I; to be used in Stage II.
            std::vector<bool> bTrackedToNow(NowChildLayerComponents.size(), false);

            /*  Tracking Stage I: Identify exclusively tracked components.
                    
                A component A is exclusively tracked, if it has exactly one connection to the previous time step
                and that so-identified predecessor B has exactly one connection to the current time step, namely to A.

                In this case, add time stamp and value to the values member of the predecessor's tree node.
            */
            if (i > 0)
            {
                for(size_t c(0);c<NowChildLayerComponents.size();c++)
                {
                    const size_t NumMatchesNowToPrevious = NowToPrevious.count(c);
                    if (NumMatchesNowToPrevious == 1)
                    {
                        //Possibly a continuation, i.e., exclusive track. Check from the other side.
                        const size_t idPreviousMatchComponent = NowToPrevious.find(c)->second;
                        // - how many does it connect to?
                        const size_t NumMatchesPreviousToNow = PreviousToNow.count(idPreviousMatchComponent);

                        if (NumMatchesPreviousToNow == 1)
                        {
                            ivwAssert(PreviousToNow.find(idPreviousMatchComponent)->second == c, "Inconsistent overlap maps.");

                            //Continuation!
                            // - extend the value vector
                            const size_t idNode = PreviousLayerComponentToTreeID.at(std::make_pair(j, idPreviousMatchComponent));
                            const size_t NumOfVoxels = NowChildLayerComponents[c].size();
                            OutTree.nodes[idNode].values.emplace(i, float(NumOfVoxels));
                            NowLayerComponentToTreeID.emplace(std::make_pair(j, c), idNode);
                            // - record tracking
                            bTrackedToNow[c] = true;
                        }
                    }
                }
            }

            /*  Tracking Stage II: Start, end and connect components.
                    
                All non-exclusively tracked components are handled by this scheme:
                Start new tree nodes for the components of the current time step.
                Connect these nodes according to overlap with the previous time step.
                End the tree nodes of the components of the previous time step.
            */
            //Stage IIa: Start new components and connect them to previous ones.
            for(size_t c(0);c<NowChildLayerComponents.size();c++)
            {
                if (bTrackedToNow[c]) continue;

                //Get the parent
                size_t idParentTreeID(idRoot);
                if (j > 0)
                {
                    TOverlap& NowChildToParent = ChildToParentAllLayer[j-1];
                    ivwAssert(NowChildToParent.count(c) == 1, "Child should have one and only one parent in a single time step.");
                    const size_t idParentComponent = NowChildToParent.find(c)->second;
                    ivwAssert(NowLayerComponentToTreeID.find(std::make_pair(j-1, idParentComponent)) != NowLayerComponentToTreeID.end(), "Parent not found.");
                    idParentTreeID = NowLayerComponentToTreeID.at(std::make_pair(j-1, idParentComponent));
                }

                //Add the child. While it is a new node, it may have previously existing children. Taken care of below. 
                const size_t NumOfVoxels = NowChildLayerComponents[c].size();
                const size_t idNew = OutTree.addChild(idParentTreeID, "", {{i, float(NumOfVoxels)}});
                OutTree.nodes[idNew].name = std::to_string(idNew);
                NowLayerComponentToTreeID.emplace(std::make_pair(j, c), idNew);

                //Add temporal connections and copy their children
                auto Connections = NowToPrevious.equal_range(c);
                for(auto itConn=Connections.first;itConn!=Connections.second;itConn++)
                {
                    const size_t idPredecessorTreeID = PreviousLayerComponentToTreeID.at(std::make_pair(j, itConn->second));
                    OutTree.addTemporalEdge(idPredecessorTreeID, idNew);
                    // - expand the previous item to the current time step
                    OutTree.nodes[idPredecessorTreeID].values.emplace(i, PreviousNewSizes[itConn->second]);
                }
            }
            //Stage IIb: End old components that have no overlap.
            //Nothing to do here, really. They end by just not being continued in any form to now.
        }

        //Add new hierarchical information
        // The hierarchy changes if new components appear, i.e., merge, split, and birth.
        // In case of merges and splits, a node may inherit a child from the previous time step.
        // We need to take care of this here; it does not fit into the above algo.
        for(const auto& LCTID : NowLayerComponentToTreeID)
        {
            //Shorthands to the node/component
            const size_t& idLayer = LCTID.first.first;
            const size_t& idComponent = LCTID.first.second;
            const size_t& idNode = LCTID.second;

            //The last layer cannot have children
            if (idLayer >= NumIsovalues - 1) continue;

            //Shorthands Hierarchy
            TOverlap& ParentToChild = ParentToChildAllLayer[idLayer];

            //The children recorded in the tree.
            const std::vector<size_t> ExistingEdgesToChildren = OutTree.getHierarchicalChildren(idNode);

            //Get all its overlapping components and check whether a corresponding edge exists. If not, add this edge.
            const auto& ChildrenRange = ParentToChild.equal_range(idComponent);
            for(auto itChild(ChildrenRange.first);itChild!=ChildrenRange.second;itChild++)
            {
                const size_t idChildTreeID = NowLayerComponentToTreeID.at(std::make_pair(idLayer+1, itChild->second));
                if (std::find(ExistingEdgesToChildren.begin(), ExistingEdgesToChildren.end(), idChildTreeID) == ExistingEdgesToChildren.end())
                {
                    OutTree.addHierarchyEdge(idNode, idChildTreeID);
                }
            }
        }
    }


    //Add ghost children
    if (propGhostChildrenCreate) AddGhostChildren(propGhostChildrenTrackLikeParents, OutTree);

    uint64_t tMin, tMax;
    OutTree.getMinMaxTimeShallow(idRoot, tMin, tMax);
    OutTree.nodes[idRoot].values.emplace(tMin, 0.0f);
    OutTree.nodes[idRoot].values.emplace(tMax, 0.0f);

    //Push it out!
    portOutTree.setData(pOutputTree);
}


void TemporalTreeGenerateFromTrackingGraph::process()
{
    if (!portSeries.isConnected()) return;

    //Get Input
    auto pSeries = portSeries.getData();
    if (!pSeries) return;
    const size_t NumFiles = pSeries->GetNumFiles();

    //Get isovalues
    std::vector<double> Isovalues;
    GetIsovaluesSorted(Isovalues);
    const size_t NumIsovalues = Isovalues.size();
    if (NumIsovalues < 1) return;
    const double IsovalueRange = fabs(Isovalues.front() - Isovalues.back());

    //Get other params
    const RELATION HowToCompare = propRelation.get();

    //Get input data
    std::shared_ptr<Volume> pVol = pSeries->GetVolume(propTimeStepExample.get());
    if (!pVol) return;
    //Get Representation
    const VolumeRAM* pInData = pVol->getRepresentation< VolumeRAM >();
    if (!pInData) return;

    //Reuse or get output data
    std::shared_ptr<Volume> pOutVolume = CreateOrReuseResultVolume(pVol);
    VolumeRAM* pVolRAM = pOutVolume->getEditableRepresentation<VolumeRAM>();
    glm::tvec3<uint8_t>* pOutData = (glm::tvec3<uint8_t>*)pVolRAM->getData();
    // - empty it
    memset(pOutData, 0, pVolRAM->getNumberOfBytes());

    //Draw colors
    const TransferFunction& Colormap = propColormap.get();
    for(int i(0);i<NumIsovalues;i++)
    {
        //const double NormalizedIsovalue = fabs(Isovalues[i] - Isovalues.front())  / IsovalueRange;
        const double NormalizedIsovalue = double(i+1) / double(NumIsovalues - 1 + 1); //avoid zero

        vec4 fColor = Colormap.sample(NormalizedIsovalue);
        //LogInfo("NormalizedIsovalue = " << NormalizedIsovalue << " Color = " << fColor);

        glm::tvec3<uint8_t> Color;
        Color[0] = uint8_t(255 * fColor[0]);
        Color[1] = uint8_t(255 * fColor[1]);
        Color[2] = uint8_t(255 * fColor[2]);

        EvalPredicate(*pInData, Isovalues[i], HowToCompare, Color, pOutData);
    }

    //Push it!
    portOutSegmentationExample.setData(pOutVolume);
}

} // namespace kth
} // namespace

