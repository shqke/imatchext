#ifndef _INCLUDE_WRAPPERS_H_
#define _INCLUDE_WRAPPERS_H_

#include <iserver.h>

class CBaseServer :
	public IServer
{
public:
	static int offset_m_nReservationCookie;

	static CBaseServer* FromIServer(IServer* pServer)
	{
		return reinterpret_cast<CBaseServer*>(pServer);
	}

	uint64_t GetReservationCookie() const
	{
		return *reinterpret_cast<const uint64*>(reinterpret_cast<const byte*>(this) + offset_m_nReservationCookie);
	}
};

#endif // _INCLUDE_WRAPPERS_H_
