/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, November 22, 2017 - 13:13:38
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <modules/temporaltreemaps/processors/treegeneratefromfilesystem.h>
#include <modules/tools/performancetimer.h>

#include <sstream>
#include <filesystem>

namespace fs = std::experimental::filesystem;

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeGenerateFromFileSystem::processorInfo_
{
    "org.inviwo.TemporalTreeGenerateFromFileSystem",      // Class identifier
    "Tree Generate From File System",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeGenerateFromFileSystem::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeGenerateFromFileSystem::TemporalTreeGenerateFromFileSystem()
    :Processor()
    ,portOutTree("OutTree")
	,propStartDir("StartDir", "Start Directory")
    ,propMaxDepth("MaxDepth", "Maximal Depth", -1, -1, 10, 1)
    ,propAction("Action", "Start Scan")
{
    addPort(portOutTree);
    addProperty(propMaxDepth);
    addProperty(propStartDir);

    propAction.onChange([&]()
    {
        ScanFileSystem();
    });
    addProperty(propAction);
}

namespace
{
    void RecordHistoricalSizes(const fs::path& ScanDir, std::map<uint64_t, float>& HistoricalSizes)
    {
        //Record individual sizes
        for(const fs::directory_entry& entry : fs::recursive_directory_iterator(ScanDir))
        {
            //If this is a file, record the file size and creation time
            if (fs::is_regular_file(entry.status()))
            {
                double FileSizeBytes = (double)fs::file_size(entry); //original in uintmax_t
                // - convert in KB
                float FileSizeKB = (float)(FileSizeBytes / 1024.);
                //...and the creation time
                uint64_t FileModTime = (uint64_t)std::chrono::system_clock::to_time_t(fs::last_write_time(entry));
                // - add them
                HistoricalSizes.emplace(FileModTime, FileSizeKB);
            }
            //else ignore, since we only tally up the files
        }

        //Tally up
        float Sum(0);
        for(auto& item : HistoricalSizes)
        {
            item.second = Sum += item.second;
        }
    }


    void ScanRecursively(const fs::path& ScanDir, const size_t idParent, const int Level, const int MaxLevel, TemporalTree& OutTree)
    {
        //Did we reach maximum level?
        if (Level == MaxLevel)
        {
            //The only way to determine the size of this directory,
            //is to scan its content recursively.
            //But we do this without creating a tree below this item.
            //Hence, this item (idParent) becomes a leaf.
            RecordHistoricalSizes(ScanDir, OutTree.nodes[idParent].values);

            //It may happen that the entire subtree is empty, i.e., no file with > 0 Bytes.
            //if (OutTree.nodes[idParent].values.empty())
            //{
            //}

            return;
        }

        //Scan the current hierarchy and record creation times and file sizes
        // This is not a full history, but somewhat
        for(const fs::directory_entry& entry : fs::directory_iterator(ScanDir))
        {
            //Empty file or directory? We do not record them. Empty directories make trouble in particular.
            if (fs::is_empty(entry)) continue;

            //Add the item to the tree
            const size_t id = OutTree.addChild(idParent, entry.path().filename().string(), {});

            //If this is a file, record the file size and creation time
            if (fs::is_regular_file(entry))
            {
                double FileSizeBytes = (double)fs::file_size(entry); //original in uintmax_t
                // - convert in KB
                float FileSizeKB = (float)(FileSizeBytes / 1024.);
                //...and the creation time
                uint64_t FileModTime = (uint64_t)std::chrono::system_clock::to_time_t(fs::last_write_time(entry));
                // - add them
                OutTree.nodes[id].values.emplace(FileModTime, FileSizeKB);
            }
            else if (fs::is_directory(entry))
            {
                //Scan subdirectories
                ScanRecursively(entry, id, Level + 1, MaxLevel, OutTree);
            }
            //else ignore
        }
    }


    void GenerateTimeSpans(const size_t idNode, const uint64_t tMax, uint64_t& tMin, TemporalTree& OutTree)
    {
        //Recursively over the entire tree

        //Get the children of this node
        const auto itHierarchyEdges = OutTree.edgesHierarchy.find(idNode);

        //Is it a leaf or a parent?
        if (itHierarchyEdges == OutTree.edgesHierarchy.end())
        {
            //Leaves: extend from the last time to now, duplicate last file size.

            //Get the data values
            auto& Values = OutTree.nodes[idNode].values;
            if(!Values.empty())
            {
                tMin = Values.begin()->first;
                const uint64_t LastTime = Values.rbegin()->first;
                const float LastFileSize = Values.rbegin()->second;

                //Extend
                assert(LastTime <= tMax);
                if (LastTime != tMax) Values.emplace(tMax, LastFileSize);
            }
        }
        else
        {
            //Parents: span from minimum time to maximum time (now) of children, file size zero.

            //Shorthand for the children
            const std::vector<size_t>& Children = itHierarchyEdges->second;

            //Call recursively, get minimum time
            tMin = tMax;
            for(const size_t idChild : Children)
            {
                uint64_t tMinChild(tMax);
                GenerateTimeSpans(idChild, tMax, tMinChild, OutTree);

                //Earlier?
                if (tMinChild < tMin) tMin = tMinChild;
            }

            //Span it
            auto& Values = OutTree.nodes[idNode].values;
            assert(Values.size() == 0);
            Values.emplace(tMin, 0.f);
            Values.emplace(tMax, 0.f);
        }
    }
};

void TemporalTreeGenerateFromFileSystem::ScanFileSystem()
{
    //We are using std::filesystem from the C++17 standard,
    //which is reasonably well available in Visual Studio 2015 Update 3.

    //Get the start directory
    const fs::path StartDir = propStartDir.get();

    //Maximal Depth
    const int MaxDepth = propMaxDepth.get();

    //Safety
    if (!(fs::exists(StartDir) && fs::is_directory(StartDir)))
    {
        LogError(StartDir << " does not exist or is not a directory.");
        return;
    }

    //Create a tree
    auto pOutTree = std::make_shared<TemporalTree>();
    // - shorthand
    TemporalTree& OutTree = *pOutTree;

    //Create a root
    const size_t idRoot = OutTree.addNode(StartDir.filename().string(), {});

    //Scan!
    PerformanceTimer Timer;
    ScanRecursively(StartDir, idRoot, 0, MaxDepth, OutTree);

    //Augment parents with time spans until the oldest file.
    uint64_t tMin;
    uint64_t tMax;
    OutTree.getMinMaxTime(idRoot, tMin, tMax);
    //// - what is now?
    //uint64_t Now = (uint64_t)std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    //tMax = Now;
    uint64_t tMinInternal;
    GenerateTimeSpans(idRoot, tMax, tMinInternal, OutTree);
    assert(tMinInternal == tMin);

    //Print the oldest file date
    time_t ttMin = tMin;
    time_t ttMax = tMax;
    std::stringstream stMin, stMax;
    stMin << std::put_time(std::localtime(&ttMin), "%Y-%m-%d %X"); //If you call them in the same stream, they will print the first time only. STL bug?
    stMax << std::put_time(std::localtime(&ttMax), "%Y-%m-%d %X");
    LogInfo("File System Tree with "
            << OutTree.nodes.size() <<" nodes in range: [" << tMin << ", " << tMax << "], which is "
            << stMin.str() << " through " << stMax.str()
            << ". Needed " << Timer.ElapsedTime() << " seconds.");

    //Push it out!
    portOutTree.setData(pOutTree);
}


void TemporalTreeGenerateFromFileSystem::process()
{
    //Nothing to be done by default.
}

} // namespace kth
} // namespace

