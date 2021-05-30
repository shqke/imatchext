#ifndef _INCLUDE_IMATCHEXTL4D_H_
#define _INCLUDE_IMATCHEXTL4D_H_

#include <IShareSys.h>

#if SOURCE_ENGINE == SE_LEFT4DEAD
#include <matchmaking/imatchframework.h>
#include <matchmaking/l4d/imatchext_l4d.h>
#elif SOURCE_ENGINE == SE_LEFT4DEAD2
#include <matchmaking/imatchframework.h>
#include <matchmaking/l4d2/imatchext_l4d.h>
#endif

class IMatchExtL4D;
class IMatchFramework;

#define SMINTERFACE_IMATCHEXT_NAME		"IMatchExtInterface"
#define SMINTERFACE_IMATCHEXT_VERSION		11

namespace SourceMod
{
	class IMatchExtListener
	{
	public:
		virtual ~IMatchExtListener() = 0;

		virtual void NotifyMissionReload() = 0;
	};

	class IMatchExtInterface :
		public SMInterface
	{
	public:
		const char *GetInterfaceName() override
		{
			return SMINTERFACE_IMATCHEXT_NAME;
		}
		
		unsigned int GetInterfaceVersion() override
		{
			return SMINTERFACE_IMATCHEXT_VERSION;
		}

	public:
		virtual IMatchExtL4D *GetIMatchExtL4D() = 0;
		virtual IMatchFramework *GetIMatchFrameWork() = 0;

		virtual void AddListener(IMatchExtListener* pListener) = 0;
		virtual void RemoveListener(IMatchExtListener* pListener) = 0;
	};
};

#endif // _INCLUDE_IMATCHEXTL4D_H_
