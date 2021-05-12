#ifndef _INCLUDE_IMATCHEXT_EXTENSION_PROPER_H_
#define _INCLUDE_IMATCHEXT_EXTENSION_PROPER_H_

#include "smsdk_ext.h"

#include "../IMatchExtInterface.h"

extern IMatchFramework *g_pMatchFramework;
extern IMatchExtL4D *g_pMatchExtL4D;

class CMatchExtInterface :
	public SDKExtension,
	public IMatchExtInterface
{
public:
	bool SDK_OnLoad(char *error, size_t maxlength, bool late) override;

public:
	IMatchExtL4D *GetIMatchExtL4D() override;
	IMatchFramework *GetIMatchFrameWork() override;
};

#endif // _INCLUDE_IMATCHEXT_EXTENSION_PROPER_H_
