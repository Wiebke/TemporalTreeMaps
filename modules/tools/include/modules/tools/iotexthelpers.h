#pragma once

#include <string.h>
#include <modules/tools/toolsmoduledefine.h>

namespace inviwo
{
namespace kth
{


IVW_MODULE_TOOLS_API bool ContainsOnlyWhiteSpacesOrIsComment(const char* buf);
IVW_MODULE_TOOLS_API bool ReadNextLine(FILE* fp, char* buf, const int bufsize);
IVW_MODULE_TOOLS_API char* JumpToFirstValue(char* buf);
IVW_MODULE_TOOLS_API char* ReadAndJump(FILE* fp, char* buf, const int bufsize);
IVW_MODULE_TOOLS_API std::string ReadNextQuotedString(FILE* fp, char* buf, const int bufsize);

//IVW_MODULE_TOOLS_API bool ScanAndJump(FILE* fp, char* buf, const int bufsize, const char* format, ...);

} // namespace kth

};
