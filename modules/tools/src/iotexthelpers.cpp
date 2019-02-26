#include <inviwo/core/common/inviwo.h>
#include <modules/tools/iotexthelpers.h>

namespace inviwo
{
namespace kth
{

bool ContainsOnlyWhiteSpacesOrIsComment(const char* buf)
{
	int i = 0;
	while (buf[i] != '\0')
	{
		if (
			(buf[i] != '\t')
		&&	(buf[i] != '\n')
		&&	(buf[i] != ' ')
		&&	(buf[i] != '\r')
			)
		{
			if ( (buf[i] == '#') || ((buf[i] == '/') && (buf[i+1] == '/')) ) return true;

			return false;
		}

		i++;
	}

	return true;
}


bool ReadNextLine(FILE* fp, char* buf, const int bufsize)
{
	do
	{
		char* ret = fgets(buf, bufsize, fp);
		if (!ret) return false;
	}
	while (ContainsOnlyWhiteSpacesOrIsComment(buf));

	return true;
}


char* JumpToFirstValue(char* buf)
{
	int i=0;
	while (buf[i] != '\0')
	{
		//Search for the next number or Minus-Sign or Point
		if ( ((buf[i] >= '0') && (buf[i] <= '9'))
			|| (buf[i] == '-') || (buf[i] == '.') )
			return &buf[i];

		i++;
	}

	//assert(false);
	LogErrorCustom("IO Text Helpers", "Format Error: Line containing no value.");
	return NULL;
}

char* ReadAndJump(FILE* fp, char* buf, const int bufsize)
{
	if (!ReadNextLine(fp, buf, bufsize))
	{
		LogErrorCustom("IO Text Helpers", "Unexpected EOF!");
		throw("Unexpected EOF! Please check the number of elements.");
	}

	char* jumped = JumpToFirstValue(buf);

	if (jumped)
		return jumped;
	else
		return ReadAndJump(fp, buf, bufsize);
}

std::string ReadNextQuotedString(FILE* fp, char* buf, const int bufsize)
{
	if (!ReadNextLine(fp, buf, bufsize))
	{
		LogErrorCustom("IO Text Helpers", "Unexpected EOF!");
		throw("Unexpected EOF! Please check the number of elements.");
	}

	char* FirstQuote = strchr(buf, '"');
	if (!FirstQuote)
	{
		LogErrorCustom("IO Text Helpers", "Unexpected Format (quoted String expected)!");
		throw("Unexpected Format (quoted String expected)! Please check quotations.");
	}

	char* SecondQuote = strchr(FirstQuote+1, '"');
	if (!SecondQuote)
	{
		LogErrorCustom("IO Text Helpers", "Unexpected Format (quoted String expected)!");
		throw("Unexpected Format (quoted String expected)! Please check quotations.");
	}

	assert(SecondQuote - FirstQuote - 1 >= 0);

	return std::string(FirstQuote, SecondQuote - FirstQuote - 1);
}

/* Does not work, because we don't have vsscanf on Windows and its not ANSI either */
//bool ScanAndJump(FILE* fp, char* buf, const int bufsize, const char* format, ...)
//{
//	//We want to assure, that the right number of fields (vars) is
//	//converted. Thus, we need to know, how many fields shall be converted.
//	//This is the number of '%' except "%%" inside format
//	//or the number of args in the variable argument list.
//	//The best thing would be to count the latter, but this is tricky.
//	//So we just count the single '%'.
//
//	assert(format);
//	int nFields(0);
//	for(const char* c=format;*c!='\0';c++)
//	{
//		if (*c == '%')
//		{
//			//Look ahead
//			const char* L = c;
//			L++;
//			if (*L != '%')
//				nFields++;
//			else
//				c++; //Jump over the second '%' in "%%"
//		}
//	}
//
//	char* rbuf = ReadAndJump(fp, buf, bufsize);
//
//    va_list arg;
//    va_start(arg, format);
//	int nAssignedFields = sscanf(rbuf, format, *arg);
//    va_end(arg);
//
//	if (nFields != nAssignedFields)
//	{
//		LogErrorCustom("IO Text Helpers", "Unexpected Number of Fields. Needed %d, got %d.", nFields, nAssignedFields);
//		throw("Unexpected Number of Fields! Please check the number of elements in one line.");
//	}
//
//    return true;
//}

} // namespace kth

};
