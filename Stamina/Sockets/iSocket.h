#ifndef __STAMINA_ISOCKET_H__
#define __STAMINA_ISOCKET_H__

#include <Stamina/Object.h>
#include <Stamina/CriticalSection.h>

namespace Stamina {
	class iSocket:
		public iObject {
	public:
		STAMINA_OBJECT_CLASS_VERSION(Stamina::iSocket, iObject, Version(0,1,0,0));
	public:
		/** Sends data on a connected socket.
		*/
		virtual void send(char* data, unsigned size) = 0;
	};
};

#endif	// __STAMINA_ISOCKET_H__