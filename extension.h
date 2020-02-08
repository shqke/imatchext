#ifndef _INCLUDE_IMATCHEXT_EXTENSION_PROPER_H_
#define _INCLUDE_IMATCHEXT_EXTENSION_PROPER_H_

#include "smsdk_ext.h"

#include "IMatchExtInterface.h"

extern IMatchFramework *g_pMatchFramework;
extern IMatchExtL4D *g_pMatchExtL4D;

class CMatchExtInterface :
	public SDKExtension,
	public IMatchExtInterface
{
public:
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);

public:
	virtual IMatchExtL4D *GetIMatchExtL4D();
	virtual IMatchFramework *GetIMatchFrameWork();
};

#endif // _INCLUDE_IMATCHEXT_EXTENSION_PROPER_H_
