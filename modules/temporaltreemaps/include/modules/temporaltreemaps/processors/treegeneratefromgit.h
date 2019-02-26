/*********************************************************************
 *  Author  : Tino Weinkauf and Wiebke Koepp
 *  Init    : Wednesday, November 22, 2017 - 16:12:28
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <filesystem>
namespace fs = std::experimental::filesystem;

#include <modules/temporaltreemaps/temporaltreemapsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/stringproperty.h>
//#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/meshport.h>
//#include <inviwo/core/properties/boolcompositeproperty.h>
//#include <inviwo/core/properties/boolproperty.h>
//#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
//#include <inviwo/core/properties/transferfunctionproperty.h>

#include <modules/temporaltreemaps/datastructures/treeport.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.TemporalTreeGenerateFromGit, Generate Tree From Git}
    ![](org.inviwo.TemporalTreeGenerateFromGit.png?classIdentifier=org.inviwo.TemporalTreeGenerateFromGit)

    Generates a temporal tree from a git repository.
    
    We call git on the command line with this:
    
    <pre>
    git log --pretty=format:commit:%H%ntimestamp:%ct --reverse --no-renames --raw
    </pre>

    and including "--since" and "--until" for a temporal selection.
    A branch name could be given as well. It is last item in the command.

    Example output:
    <pre>
    commit:3f683e1a6e72a008208af5d160c26196fa27a53c
    timestamp:1456221727
    :100644 100644 4530c9a... f2c3781... M  kxlines/linedrawer.cpp
    :100644 100644 0b9e738... 8d41f1a... M  kxlines/linedrawer.h
    :100644 100644 182c830... cc0759a... M  kxlines/linerasterizer.cpp
    </pre>

    We parse the the sha1, the time stamp, and the file names
    and put them into a temporary data structure.

    We will call git to check out a commit.
    Then we will record the filesizes of the affected files
    and record all this information in a tree.

    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class TemporalTreeGenerateFromGit
    \brief Generates a temporal tree from a git repository.
    
    Uses calls to 'git log' and other commands
    to record the file size history of a git repository.

    @author Tino Weinkauf and Wiebke Koepp
*/
class IVW_MODULE_TEMPORALTREEMAPS_API TemporalTreeGenerateFromGit : public Processor
{ 
//Friends
//Types
public:
    /** Types of changes to a file in a git repository

        From the git-diff manual page, possible status letters from the git-log are:
        - A: addition of a file
        - C: copy of a file into a new one
        - D: deletion of a file
        - M: modification of the contents or mode of a file
        - R: renaming of a file
        - T: change in the type of the file
        - U: file is unmerged (you must complete the merge before it can be committed)
        - X: "unknown" change type (most probably a bug, please report it)

        We ask git-log to not report renaming. They will then be converted to additions and deletions.
        We record any changes as Modified unless we have a A or a D.
    */
    enum EGitFileChange
    {
        Undefined,
        Created,
        Modified,
        Deleted
    };

    ///Holds a git commit with absolute file names.
    struct TGitCommit
    {
        std::string Sha1;
        uint64_t Timestamp;
        std::vector<std::pair<fs::path, EGitFileChange>> Files;
    };

    typedef std::vector<TGitCommit> TGitCommitLog;

//Construction / Deconstruction
public:
    TemporalTreeGenerateFromGit();
    virtual ~TemporalTreeGenerateFromGit() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    void ScanGitRepository();

protected:
    ///Our main computation function
    virtual void process() override;

    ///Runs git log to obtain the history of the repository.
    void RunGitLog(const fs::path& GitRepo, TGitCommitLog& Log);

    void AggregateGitLog(const TGitCommitLog& Log, TGitCommitLog& AggregatedLog);

	void AggregateGitLogTags(const TGitCommitLog& Log, TGitCommitLog& AggregatedLog, const std::vector<std::string>& TagSha1s);

    ///Scans the history and records it as a temporal tree.
    void GitLogToTree(const TGitCommitLog& Log, const fs::path& GitRepoBase, TemporalTree& Tree);

//Ports
public:
    ///The output tree
    TemporalTreeOutport portOutTree;

//Properties
public:
    ///Where to start the search. A git repository is assumed here.
    DirectoryProperty propGitRepoDir;

    ///Only commits after this date
    StringProperty propCommitsSince;

    ///Only commits before this date
    StringProperty propCommitsUntil;

    ///Only commits on this branch
    StringProperty propBranch;

    ///Way of choosing the commits to consider
    OptionPropertyInt propCommitsChoice;
	
	///Number of commits to be aggreagted and to be treated as a single one
    IntSizeTProperty propAggregatedCommits;

    ///Since scanning is expensive and depends on the unknown filesystem / git,
    ///we scan only on explicit user demand, namely when pressing this button.
    ButtonProperty propAction;



//Attributes
private:

};

} // namespace kth
} // namespace
