/*
 *  Stamina.LIB
 *  
 *  Please READ /License.txt FIRST! 
 * 
 *  Copyright (C)2003,2004,2005 Rafa� Lindemann, Stamina
 *
 *  $Id$
 */

#ifndef __STAMINA_STRING__
#define __STAMINA_STRING__

#pragma once

#include <string>

namespace Stamina {

	typedef std::string String;

	class StringRef {
	public:
		StringRef(const char* ch):_ch(ch) {}
#ifdef _STRING_
		StringRef(const std::string& ch):_ch(ch.c_str()) {}
#endif
#ifdef STDSTRING_H
		StringRef(const CStdString& ch):_ch(ch.c_str()) {}
#endif

		operator const char*() const {
			return _ch;
		}

		const char* c_str() const {
			return _ch;
		}

		operator const String() const {
			return String(_ch);
		}

	private:
		const char* _ch;
	};


	inline bool operator == (const String& a, const StringRef& b) {
		return stricmp(a.c_str(), b) == 0;
	}

}

#endif