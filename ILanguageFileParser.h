#ifndef _INCLUDE_LANGPARSER_H_
#define _INCLUDE_LANGPARSER_H_

#include <IShareSys.h>
using namespace SourceMod;

#define SMINTERFACE_ILANGPARSER_NAME		"IValveLanguageParser"
#define SMINTERFACE_ILANGPARSER_VERSION		10

#define MAX_KEY_LENGTH	128
#define MAX_PHRASE_LENGTH	4096

#ifdef _MSC_VER
using ucs2_t = wchar_t;
#else
using ucs2_t = char16_t;
#endif

enum ParseAction_t
{
	Parse_Continue,
	Parse_Halt,
	Parse_HaltFail,
};

enum ParseError_t
{
	ParseError_None,
	ParseError_StreamOpen,
	ParseError_StreamRead,
	ParseError_Overflow,
	ParseError_StreamEnd,
	ParseError_InvalidToken,
	ParseError_InvalidChar,
	ParseError_SectionBegin,
	ParseError_SectionEnd,
};

class ILanguageFileParserListener
{
public:
	virtual ParseAction_t State_Started()
	{
		return Parse_Continue;
	}

	virtual ParseAction_t State_EnteredSection(const char* pszKey)
	{
		return Parse_Continue;
	}

	virtual ParseAction_t State_KeyValue(const char* pszKey, const char* pszValue)
	{
		return Parse_Continue;
	}

	virtual ParseAction_t State_LeftSection()
	{
		return Parse_Continue;
	}

	virtual void State_Ended(bool halted, bool failed)
	{
	}
};

class ILanguageFileParser :
	public SMInterface
{
public:
	const char* GetInterfaceName() override
	{
		return SMINTERFACE_ILANGPARSER_NAME;
	}

	unsigned int GetInterfaceVersion() override
	{
		return SMINTERFACE_ILANGPARSER_VERSION;
	}

public:
	virtual ParseError_t ParseFile(const char* pszRelativePath, ILanguageFileParserListener* pListener, char* error, size_t maxlength) = 0;
	virtual ParseError_t ParseBuffer(const ucs2_t* pDataIn, ILanguageFileParserListener* pListener, char* error, size_t maxlength) = 0;
};

#endif // _INCLUDE_LANGPARSER_H_
