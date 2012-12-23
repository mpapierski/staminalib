CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (Stamina)

SET (Stamina_SOURCES
	CriticalSection.cpp
	Debug.cpp
	FileResource.cpp
	FindFile.cpp
	Helpers.cpp
	Internet.cpp
	InvokeQueue.cpp
	Logger.cpp
	MD5.cpp
	Object.cpp
	RegEx.cpp
	SimXML.cpp
	String.cpp
	Task.cpp
	Thread.cpp
	ThreadInvoke.cpp
	Time64.cpp
	Unique.cpp
	WinHelper.cpp)

SET (Stamina_HEADERS
	Array.h
	Assert.h
	Buffer.h
	CRC32.h
	ComHelper.h
	Compat_vs2k5.h
	Console.h
	CriticalSection.h
	DataTable.h
	DataTableImpl.h
	Debug.h
	Event.h
	Exception.h
	FileResource.h
	FindFile.h
	FindFileFiltered.h
	Helpers.h
	Internet.h
	InvokeQueue.h
	Lib.h
	ListWndImpl.h
	Lock.h
	LockSelector.h
	Logger.h
	LoggerImpl.h
	MD5.h
	Memory.h
	Mutex.h
	Object.h
	ObjectClassInfo.h
	ObjectImpl.h
	ObjectPtr.h
	Rect.h
	RegEx.h
	Semaphore.h
	SimXML.h
	Stamina.h
	TLS.h
	Task.h
	Thread.h
	ThreadInvoke.h
	ThreadRun.h
	Time64.h
	Timer.h
	Unique.h
	UniqueImpl.h
	Version.h
	VersionControl.h
	WideChar.h
	WinHelper.h
	iArray.h)

SOURCE_GROUP ("Header Files"
	FILES ${Stamina_HEADERS})

ADD_LIBRARY (Stamina
	${Stamina_SOURCES}
	${Stamina_HEADERS})
TARGET_LINK_LIBRARIES (Stamina
	${LIBRARIES})
