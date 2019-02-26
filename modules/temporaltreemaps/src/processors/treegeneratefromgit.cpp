/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, November 22, 2017 - 16:12:28
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <inviwo/core/util/utilities.h>
#include <modules/temporaltreemaps/processors/treegeneratefromgit.h>
#include <modules/tools/performancetimer.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo TemporalTreeGenerateFromGit::processorInfo_
{
    "org.inviwo.TemporalTreeGenerateFromGit",      // Class identifier
    "Tree Generate From Git",                // Display name
    "Temporal Tree",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo TemporalTreeGenerateFromGit::getProcessorInfo() const
{
    return processorInfo_;
}


TemporalTreeGenerateFromGit::TemporalTreeGenerateFromGit()
    :Processor()
    ,portOutTree("OutTree")
    ,propGitRepoDir("GitRepoDir", "Git Repository")
    ,propCommitsSince("CommitsSince", "Commits Since")
    ,propCommitsUntil("CommitsUntil", "Commits Until")
    ,propBranch("Branch", "Branch")
	,propCommitsChoice("CommitsChoice", "Choosing Commits")
    ,propAggregatedCommits("AggregatedCommits", "Aggregated Commits", 1, 1)
    ,propAction("Action", "Start Scan")
{
    addPort(portOutTree);
    addProperty(propGitRepoDir);
    addProperty(propCommitsSince);
    addProperty(propCommitsUntil);
    addProperty(propBranch);
	addProperty(propCommitsChoice);

	propCommitsChoice.addOption("commitsAsIs", "As is", 0);
	propCommitsChoice.setSelectedIndex(0);
	propCommitsChoice.setCurrentStateAsDefault();

	propCommitsChoice.addOption("commitsByAggregation", "By Subsampling", 1);
	util::hide(propAggregatedCommits);
	addProperty(propAggregatedCommits);

	propCommitsChoice.addOption("commitsByTags", "By Tags", 2);

	propCommitsChoice.onChange([&]() {
		if (propCommitsChoice.get() == 1) {
			util::show(propAggregatedCommits);
		}
		else {
			util::hide(propAggregatedCommits);
		}
	});

    propAction.onChange([&]()
    {
        ScanGitRepository();
    });
    addProperty(propAction);
}


void TemporalTreeGenerateFromGit::RunGitLog(const fs::path& GitRepo, TGitCommitLog& Log)
{
	//Checkout most recent version of the repository (In case of previous errors)
	std::string GitFetch = "git fetch --all";
	const int sysretPull = system(GitFetch.c_str());
	if (sysretPull != 0)
	{
		LogInfo("Initial pull failed with " << sysretPull << ".");
	}

    //Create the git command
    //std::string GitCommand = "cd " + propGitRepoDir.get() + "; git log --pretty=format:commit:%H%ntimestamp:%ct --reverse --no-renames --raw";
    std::string GitCommand = "git log --pretty=format:commit:%H%ntimestamp:%ct --reverse --no-renames --raw";
    if (!propCommitsSince.get().empty()) GitCommand += " --since \"" + propCommitsSince.get() + "\"";
    if (!propCommitsUntil.get().empty()) GitCommand += " --until \"" + propCommitsUntil.get() + "\"";
    if (!propBranch.get().empty()) GitCommand += " " + propBranch.get();

    //Temp file
    const std::string TempFileName = std::tmpnam(NULL);
    std::string TotalCmd = GitCommand + " 1> " + TempFileName + " 2>&1";

    //Run
    LogInfo("Running: " << TotalCmd);
    const int sysret = system(TotalCmd.c_str());
    LogInfo("Command exited with " << sysret);

    //Parse
    std::ifstream TextLog(TempFileName, std::ifstream::in);
    std::string CurrentLine;
    TGitCommit CurrentCommit;
    bool bFirst(true);
    while (std::getline(TextLog, CurrentLine))
    {
        //New commit!
        const size_t idxCommit = CurrentLine.find("commit:");
        if (idxCommit != std::string::npos)
        {
            //Flush the old commit
            if (!bFirst)
            {
                Log.push_back(CurrentCommit);
                CurrentCommit.Files.clear();
            }

            CurrentCommit.Sha1 = CurrentLine.substr(idxCommit + 7 /*std::string("commit:").length()*/);

            //Next time, we will add it.
            bFirst = false;
            continue;
        }

        //Timestamp
        const size_t idxTimestamp = CurrentLine.find("timestamp:");
        if (idxTimestamp != std::string::npos)
        {
            const std::string strTimestamp = CurrentLine.substr(idxTimestamp + 10);
            CurrentCommit.Timestamp = strtoull(strTimestamp.c_str(), NULL, 10);
            continue;
        }

        //Affected Files
        const size_t idxLastTab = CurrentLine.rfind('\t');
        if (idxLastTab != std::string::npos)
        {
            const size_t idxStartChange = CurrentLine.rfind(' ', idxLastTab);
            if (idxStartChange != std::string::npos)
            {
                //Type of change - it is not super crucial to get this 100% right.
                // We record actual changes how we observe them on the filesystem.
                EGitFileChange WhatHappened(EGitFileChange::Undefined);
                switch (CurrentLine[idxStartChange + 1])
                {
                    case 'T':
                    case 'C':
                    case 'M': WhatHappened = EGitFileChange::Modified; break;

                    case 'A': WhatHappened = EGitFileChange::Created; break;

                    case 'D': WhatHappened = EGitFileChange::Deleted; break;

                    default:  WhatHappened = EGitFileChange::Undefined; break;
                }

                //Filename
                std::string strFileName = CurrentLine.substr(idxLastTab + 1);
                strFileName.erase(strFileName.begin(), std::find_if(strFileName.begin(), strFileName.end(),
                    [](int ch) {return !std::isspace(ch); }));
                fs::path Filename(strFileName);
                Filename = fs::absolute(Filename, GitRepo);

                //Add
                CurrentCommit.Files.push_back(std::make_pair(Filename, WhatHappened));

                continue;
            }
        }
    }
    TextLog.close();

    //Push the last commit
    if (!bFirst) Log.push_back(CurrentCommit);

    //Remove temporary file
    fs::remove(TempFileName);
}

namespace
{
    float GetFileSizeKBAsFloat(const fs::path& Filename)
    {
        double FileSizeBytes = (double)fs::file_size(Filename); //original in uintmax_t
        // - convert in KB
        return (float)(FileSizeBytes / 1024.);
    }


    void ScanRecursively(const fs::path& ScanDir, const size_t idParent, const uint64_t Timestamp,
                         std::map<fs::path, size_t>& PathToNode, TemporalTree& Tree)
    {
        //Scan the current hierarchy and record creation times and file sizes
        for(const fs::directory_entry& entry : fs::directory_iterator(ScanDir))
        {
            //Empty file or directory? We do not record them. Empty directories make trouble in particular.
            if (fs::is_empty(entry)) continue;

            //Is it the .git directory? Skip that one.
            if (entry.path().filename() == ".git") continue;

            //Add the item to the tree
            const size_t id = Tree.addChild(idParent, entry.path().filename().string(), {});
            PathToNode.emplace(entry.path(), id);

            //If this is a file, record the file size and set the timestamp
            if (fs::is_regular_file(entry))
            {
                const float FileSizeKB = GetFileSizeKBAsFloat(entry);
                // - add with timestamp
                Tree.nodes[id].values.emplace(Timestamp, FileSizeKB);
            }
            else if (fs::is_directory(entry))
            {
                //Scan subdirectories
                ScanRecursively(entry, id, Timestamp, PathToNode, Tree);
            }
            //else ignore
        }
    }

    bool RunSystemCmd(const std::string& Cmd)
    {
        const int sysret = system(Cmd.c_str());
        if (sysret != 0)
        {
            LogErrorCustom("TemporalTreeGenerateFromGit", "Error when running \'" << Cmd << "\'. Process exited with " << sysret << ".");
            return false;
        }

        return true;
    }

    bool GitCheckout(const std::string& Sha1)
    {
        std::string GitCommand = "git checkout " + Sha1 + " -f";
        return RunSystemCmd(GitCommand);
    }
};


void TemporalTreeGenerateFromGit::GitLogToTree(const TGitCommitLog& Log, const fs::path& GitRepoBase, TemporalTree& Tree)
{
    std::map<fs::path, size_t> PathToNode;

    //Create a root
    const size_t idRoot = Tree.addNode(GitRepoBase.filename().string(), {});
    PathToNode.emplace(GitRepoBase, idRoot);

    //Any commits?
    if (Log.empty()) return;

    //Checkout first commit
    if (!GitCheckout(Log[0].Sha1)) return;

    //Scan the file system fully for the first commit.
    //The changes of the first commit are not of any interest.
    ScanRecursively(GitRepoBase, idRoot, Log[0].Timestamp, PathToNode, Tree);

    //Check out every further commit, record the changes to the hierarchy and file size values.
    for(size_t i(1);i<Log.size();i++)
    {
        //Shorthand
        const TGitCommit& ThisCommit = Log[i];

        if (!GitCheckout(ThisCommit.Sha1)) return;

        //For each file: record status on disk and compare to knowledge from commit
        for(const auto& FileAndWhat : ThisCommit.Files)
        {
            //Disk Status
            // - exists?
            const bool bFoundOnDisk(fs::exists(FileAndWhat.first));
            // - regular file? Empty directories may appear due to submodules.
            const bool bIsFile(fs::is_regular_file(FileAndWhat.first));
            // - size
            const float FileSizeKB(bFoundOnDisk && bIsFile ? GetFileSizeKBAsFloat(FileAndWhat.first) : 0.f);

            //Tree Status
            // - find it
            const auto itNode = PathToNode.find(FileAndWhat.first);\
            const bool bFoundInTree(itNode != PathToNode.end());

            //Compare to commit status - this is only informative. We ultimately record the disk status below.
            if (FileAndWhat.second == EGitFileChange::Created && (!bFoundOnDisk || bFoundInTree))
            {
                LogWarnCustom("TemporalTreeGenerateFromGit",
                              "File should have been newly created, but OnDisk = " << bFoundOnDisk
                              << " and InTree = " << bFoundInTree
                              << " for file " << FileAndWhat.first
                              << " at commit " << ThisCommit.Sha1);
            }
            if (FileAndWhat.second == EGitFileChange::Modified && (!bFoundOnDisk || !bFoundInTree))
            {
                LogWarnCustom("TemporalTreeGenerateFromGit",
                              "File should have been modified, but OnDisk = " << bFoundOnDisk
                              << " and InTree = " << bFoundInTree
                              << " for file " << FileAndWhat.first
                              << " at commit " << ThisCommit.Sha1);
            }
            if (FileAndWhat.second == EGitFileChange::Deleted && (bFoundOnDisk || !bFoundInTree))
            {
                LogWarnCustom("TemporalTreeGenerateFromGit",
                              "File should have been deleted, but OnDisk = " << bFoundOnDisk
                              << " and InTree = " << bFoundInTree
                              << " for file " << FileAndWhat.first
                              << " at commit " << ThisCommit.Sha1);
            }

            //Now we actually act on the disk status and record it in the tree
            if (bFoundOnDisk && bIsFile)
            {
                if (bFoundInTree)
                {
                    //Modified - update file size
                    const size_t idNode = itNode->second;
                    Tree.nodes[idNode].values.emplace(ThisCommit.Timestamp, FileSizeKB);
                }
                else
                {
                    //New file - add to tree and record first file size

                    //Get the last parent that we already have in the tree

                    // - remove the common base path. Could be done with relative paths, but I want to avoid ".." and rather run into an error.
                    fs::path::iterator itCurrent(FileAndWhat.first.begin());
                    fs::path::iterator itBase(GitRepoBase.begin());
                    for(;itBase!=GitRepoBase.end();itBase++,itCurrent++)
                    {
                        ivwAssert(itCurrent != FileAndWhat.first.end(), "File path outside of repository base path.");
                        ivwAssert(*itBase == *itCurrent, "File path outside of repository base path.");
                    }

                    // - iterate over existing parents
                    fs::path Current(GitRepoBase);
                    size_t idParent = idRoot;
                    while (itCurrent != FileAndWhat.first.end())
                    {
                        Current /= *itCurrent;
                        const auto itFound = PathToNode.find(Current);
                        if (itFound == PathToNode.end()) break;
                        idParent = itFound->second;
                        itCurrent++;
                    }

                    // - create parents and the file itself
                    do
                    {
                        const size_t idCurrent = Tree.addChild(idParent, Current.filename().string(), {});
                        PathToNode.emplace(Current, idCurrent);
                        idParent = idCurrent;
                        itCurrent++;
                        Current /= *itCurrent;
                    }
                    while (itCurrent != FileAndWhat.first.end());

                    // - add the file size of the new file
                    Tree.nodes[idParent].values.emplace(ThisCommit.Timestamp, FileSizeKB);
                }
            }
            else //(!bFoundOnDisk || !bIsFile)
            {
                if (bFoundInTree)
                {
                    //Deleted - finalize file size
                    const size_t idNode = itNode->second;
                    const float LastFileSize = Tree.nodes[idNode].values.rbegin()->second;
                    Tree.nodes[idNode].values.emplace(ThisCommit.Timestamp, LastFileSize);
                }
                else
                {
                    //Why are we talking about this file in the first place?
                }
            }
        }
    }

    //Finalize!
    //For each node, see if its file still exist. If so, extend values array to last time stamp.
    const uint64_t LastTimeStamp = Log.back().Timestamp;
    for(auto& entry : PathToNode)
    {
        //Shorthands
        const fs::path& ThisFileName = entry.first;
        const size_t& ThisID = entry.second;

        if (fs::exists(ThisFileName))
        {
            TemporalTree::TNode& Node = Tree.nodes[ThisID];
            if (Node.values.size() > 0)
            {
                //Get the last data value
                const float LastData = Node.values.rbegin()->second;

                //Finialize the node with this value at the given time.
                Node.values.emplace(LastTimeStamp, LastData);
            }
        }
    }

    //Finally, make sure that all parents have times dictated by their children
    Tree.addDefaultTimesForParents(idRoot);
}

void TemporalTreeGenerateFromGit::AggregateGitLog(const TGitCommitLog& Log, TGitCommitLog& AggregatedLog)
{
    const size_t numCommits = Log.size();
    if (propAggregatedCommits < 2)
    {
        AggregatedLog = Log;
        return;
    }

	// 0th commit, every given one and the last one 
	// (might be only +1 if number of commits is divisible by propAggregatedCommits)
    AggregatedLog.reserve(numCommits/propAggregatedCommits + 2);
	// For the first commit
	AggregatedLog.emplace_back();

    for(size_t i(0); i < numCommits; i++)
    {
        AggregatedLog.back().Sha1 = Log[i].Sha1;
        AggregatedLog.back().Timestamp = Log[i].Timestamp;

        for (const auto& FileAndWhat : Log[i].Files)
        {
            // Check if the file already exists in this Log
            auto itFile = std::find_if(AggregatedLog.back().Files.begin(), AggregatedLog.back().Files.end(), [&](const auto& s) { return s.first == FileAndWhat.first; });
            
            if (itFile == AggregatedLog.back().Files.end())
            {
                AggregatedLog.back().Files.push_back(FileAndWhat);
            }
        }
		// Skip propAggregatedCommits - 1 commits 
		// (Nothing comes after the last iteration, so no need to start a new log)
		if (i % propAggregatedCommits == 0 && i != numCommits - 1)
		{
			// Start a new log
			AggregatedLog.emplace_back();
		}

    }

	LogInfo(Log.back().Timestamp);
	LogInfo(AggregatedLog.back().Timestamp);
}

void TemporalTreeGenerateFromGit::AggregateGitLogTags(const TGitCommitLog & Log, TGitCommitLog & AggregatedLog, const std::vector<std::string>& TagSha1s)
{
	AggregatedLog.reserve(TagSha1s.size() + 2);
	// For the first commit
	AggregatedLog.emplace_back();

	for (size_t i(0); i < Log.size(); i++)
	{
		AggregatedLog.back().Sha1 = Log[i].Sha1;
		AggregatedLog.back().Timestamp = Log[i].Timestamp;
		for (const auto& FileAndWhat : Log[i].Files)
		{
			// Check if the file already exists in this Log
			auto itFile = std::find_if(AggregatedLog.back().Files.begin(), AggregatedLog.back().Files.end(), [&](const auto& s) { return s.first == FileAndWhat.first; });

			if (itFile == AggregatedLog.back().Files.end())
			{
				AggregatedLog.back().Files.push_back(FileAndWhat);
			}
		}

		// Check if this is a tag commit
		auto itTag = std::find(TagSha1s.begin(), TagSha1s.end(), Log[i].Sha1);
		// Start a new log entry if: the commit is a tag, the commit is first (but not if it is the last one)
		if ((itTag != TagSha1s.end() || i == 0) && i != Log.size() -1) {
			AggregatedLog.emplace_back();
		}
		
	}
}

void TemporalTreeGenerateFromGit::ScanGitRepository()
{
    //Get the Git repo
    const std::string GitRepoString = propGitRepoDir.get();
    const fs::path GitRepo(GitRepoString);
    if (GitRepoString.empty() || !fs::is_directory(GitRepo) || !fs::is_directory(GitRepo / ".git"))
    {
        LogError("Please provide the path to a proper git repository. The \'.git\' folder needs to be a subdirectory directly under the given one.");
        return;
    }

    //Change dir to git repo, come back later
    const fs::path OldCWD = fs::current_path();
    fs::current_path(GitRepo);

    //Get the history of the repo
    TGitCommitLog Log;
    PerformanceTimer Timer;
    RunGitLog(GitRepo, Log);
    LogInfo("Read " << Log.size() << " commits from the repository. Needed " << Timer.ElapsedTime() << " seconds.");

    //Aggregation
    TGitCommitLog AggregatedLog;

	if (propCommitsChoice.get() == 1) {
		AggregateGitLog(Log, AggregatedLog);
	}
	else if (propCommitsChoice.get() == 2) {
		std::string GitCommand = "git show-ref --dereference --tag";
		
		const std::string TempFileName = std::tmpnam(NULL);
		std::string TotalCmd = GitCommand + " 1> " + TempFileName + " 2>&1";

		const int sysret = system(TotalCmd.c_str());
		if (sysret != 0)
		{
			LogErrorCustom("TemporalTreeGenerateFromGit", "No Tags, aggregating instead");
			AggregateGitLog(Log, AggregatedLog);
		}
		else {
			std::ifstream TextLog(TempFileName, std::ifstream::in);
			std::string CurrentLine;
			std::vector<std::string> TagSha1s;
			while (std::getline(TextLog, CurrentLine))
			{
				// Take all tags, dereferences and not (annotated tags will occur with both commit and tag)
				TagSha1s.push_back(CurrentLine.substr(0, 40));
			}
			AggregateGitLogTags(Log, AggregatedLog, TagSha1s);
		}
	}
	else {
		AggregatedLog = Log;
	}

    LogInfo("Aggregated to " << AggregatedLog.size() << " commits.");

    //Get time span for information purposes
    std::stringstream stMin, stMax;
    uint64_t tMin(0), tMax(0);
    if (!AggregatedLog.empty())
    {
        tMin = AggregatedLog.front().Timestamp;
        tMax = AggregatedLog.back().Timestamp;
        time_t ttMin = tMin;
        time_t ttMax = tMax;
        stMin << std::put_time(std::localtime(&ttMin), "%Y-%m-%d %X"); //If you call them in the same stream, they will print the first time only. STL bug?
        stMax << std::put_time(std::localtime(&ttMax), "%Y-%m-%d %X");
    }

    //Create a tree
    auto pOutTree = std::make_shared<TemporalTree>();
    // - shorthand
    TemporalTree& OutTree = *pOutTree;

    //Convert the history to a temporal tree
    Timer.Reset();
    GitLogToTree(AggregatedLog, GitRepo, OutTree);
    LogInfo("Git temporal tree with " << OutTree.nodes.size() << " nodes in range: [" << tMin << ", " << tMax << "], which is "
            << stMin.str() << " through " << stMax.str()
            << ". Needed " << Timer.ElapsedTime() << " seconds.");

    //Restore git repo
    GitCheckout(propBranch.get().empty() ? "master" : propBranch.get());

    //Return to original path
    fs::current_path(OldCWD);

    //Push it out!
    portOutTree.setData(pOutTree);
}


void TemporalTreeGenerateFromGit::process()
{
    //Nothing to do here.
}

} // namespace kth
} // namespace

