#ifndef _INCLUDE_IMATCHEXTL4D_H_
#define _INCLUDE_IMATCHEXTL4D_H_

#include <IShareSys.h>
#include <KeyValues.h>

#include "public/matchmaking/imatchframework.h"

#if SOURCE_ENGINE == SE_LEFT4DEAD
#include "public/matchmaking/l4d/imatchext_l4d.h"
#elif SOURCE_ENGINE == SE_LEFT4DEAD2
#include "public/matchmaking/l4d2/imatchext_l4d.h"
#else
#error "Unsupported SDK"
#endif

#define SMINTERFACE_IMATCHEXT_NAME		"IMatchExtInterface"
#define SMINTERFACE_IMATCHEXT_VERSION		10

namespace SourceMod
{
	class IMatchExtInterface : public SMInterface
	{
	public:
		virtual const char *GetInterfaceName() {
			return SMINTERFACE_IMATCHEXT_NAME;
		}
		
		virtual unsigned int GetInterfaceVersion() {
			return SMINTERFACE_IMATCHEXT_VERSION;
		}

	public:
		virtual IMatchExtL4D *GetIMatchExtL4D() = 0;
		virtual IMatchFramework *GetIMatchFrameWork() = 0;
	};
};

#endif // _INCLUDE_IMATCHEXTL4D_H_
