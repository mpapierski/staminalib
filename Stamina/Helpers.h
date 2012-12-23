/*

The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License from
/LICENSE.HTML in this package or at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is "Stamina.lib" library code, released Feb 1, 2006.

The Initial Developer of the Original Code is "STAMINA" - Rafa� Lindemann.
Portions created by STAMINA are 
Copyright (C) 2003-2006 "STAMINA" - Rafa� Lindemann. All Rights Reserved.

Contributor(s): 

--

$Id$

*/
#ifndef __STAMINA_HELPERS_H__
#define __STAMINA_HELPERS_H__
#pragma once

#include "Stamina.h"
#include <cmath>

namespace Stamina {

	inline int round(double val) {
		return (int)std::floor(val + 0.5);
	}

	inline void randomSeed() {
#ifdef _WINDOWS_
		srand(GetTickCount());
#else
		srand(time(0));
#endif
	}

	unsigned int random(unsigned int min = 0, unsigned int max = RAND_MAX);

	const char * inttoch(int v , char * s , int radix=10 , int max=-1 , bool upper=true);

	char * _vsaprintf(const char *format, va_list ap);
	char * _saprintf(const char *format, ...);

	std::string inttostr(int v , int radix=10 , int max=-1 , bool upper=true);

	inline std::string stringf(const char *format, ...) {
		va_list ap;

		va_start(ap, format);
		char * msg = _vsaprintf(format, ap);
			std::string _msg = msg;
			free(msg);
			va_end(ap);
			return _msg;
	}

	int chtoint(const std::string& str , unsigned char base=0xFF);



	inline unsigned char chval(unsigned char ch) { // zamienia znak na liczbe i odwrotnie
		/* Zamieniamy liczbe na znak */
		if (ch < 10) return '0'+ch; // 0 - 10
		if (ch < '0') return 'A'+(ch-10); // A - ...
		/* Zamieniamy znak na liczbe */
		if (ch < 'A') return ch-'0'; // 0-10
		if (ch < 'a') return ch-'A'+10; // A-...
		return ch-'a'+10; // a-...
	}

	char * str_tr(char * str , const char * chIn , const char * chOut);


	/** Looks for an argument in programs arguments list.
	If find or def are UNICODE, then the UNICODE arg list is looked up.

	@param find Name to find with special character (ie. "/find" "-help" "-?" etc.)
	@param getValue Returns the value of an argument (ie. "/find=VALUE")
	@param def Default value to return if nothing is found...
	@return Returns found argument or it's value
	*/
	std::string getArgV(const std::string& find , bool getValue = false, const std::string& def = "");

	const char * getArgV(const char * const * argList , int argCount , const char * find , bool getValue=false , const char * def=0);
	const wchar_t * getArgV(const wchar_t * const * argList , int argCount , const wchar_t * find , bool getValue=false , const wchar_t * def=0);

	const char * searchArray(const char * find , const char ** ar  , size_t count , bool getValue=false);
	const wchar_t * searchArray(const wchar_t * find , const wchar_t ** ar  , size_t count , bool getValue=false);

	inline bool argVExists(const std::string& find) {
		return getArgV(find, false).empty() == false;
	}


#ifdef _STRING_

	inline size_t stringLength(const char* str) {
		return strlen(str);
	}
	inline size_t stringLength(const wchar_t* str) {
		return wcslen(str);
	}
	template <typename CHAR>
	inline size_t stringLength(const std::basic_string<CHAR>& str) {
		return str.size();
	}

/*	inline std::string::iterator stringBegin(char* str) {
		return std::string::iterator(str);
	}
	inline std::wstring::iterator stringBegin(wchar_t* str) {
		return std::wstring::iterator(str);
	}
	template <typename CHAR>
	inline std::basic_string<CHAR>::iterator stringBegin(std::basic_string<CHAR>& str) {
		return str.begin();
	}
*/
	inline char charToLower(char ch) {
		return (char)tolower(ch);
	}
	inline wchar_t charToLower(wchar_t ch) {
		return (wchar_t)towlower(ch);
	}

	template <class STR>
		inline typename STR::value_type* stringBuffer(STR& str, typename STR::size_type size = STR::npos) 
	{
		if (size != STR::npos) {
			str.resize(size+1);
		}
		STR::value_type* ch = (STR::value_type*) str.c_str();
		ch[str.length()-1] = 0;
		return ch;
	}

	template <class STR>
		inline void stringRelease(STR& str, typename STR::size_type size = STR::npos) 
	{
		str.resize(size == STR::npos ? stringLength(str.c_str()) : size);
	}


	template <typename CHAR>
	inline size_t find_noCase(const CHAR* haystack, const CHAR* needle) {
		const CHAR* found = needle;
		const CHAR* current = haystack;
		while (*current) {
			if (charToLower(*current) == charToLower(*found)) {
				found++;
				if (*found == 0) return current - haystack - (found - 1 - needle);
			} else if (found != needle) {
				found = needle;
				continue;
			}
			current ++;
		};
		return -1;
	}
    

	std::string urlEncode(const std::string& str , char special = '%' , char * noChange = 0);
	std::string urlDecode(const std::string& str , char special = '%');

	std::string addSlashes(const std::string& str , char* escape = "\"'\\", char escapeChar = '\\');

#ifdef _DEQUE_

	typedef std::deque<String> tStringVector;

	void split(const StringRef & txt, const StringRef & splitter, tStringVector & list, bool all = true);

	void splitCommand(const StringRef& txt , char splitter ,  tStringVector & list);

#endif

#endif

	char * stripSlashes(char * str);

	/**Reverses buffer contents*/
	void * memrev(void * buff , int count);

	int ipToLong(const char * ip);
#ifdef _STRING_
	std::string longToIp(long adr);
#endif

// Directories

#ifdef _STRING_
	/**Removes directory with all contents.
	@return number of deleted files/dirs
	*/
	int removeDirTree(const std::string& path);

	/**Creates all missing directories on provided path.
	@return number of created directories
	*/
	int createDirectories(const std::string& path);

	/** Returns the name of last path part 

	ie. "bar.html" from "/foor/bar.html"
	*/
	inline std::string getFileName(const std::string& path) {
		return path.substr(path.find_last_of("\\/") + 1);
	}
	/** Returns the name of directory containing the file */
	inline std::string getFileDirectory(const std::string& path, bool returnDot = false) {
		size_t pos = path.find_last_of("\\/");
		if (pos == std::string::npos) {
			return returnDot ? "." : "";
		} else {
			return path.substr(0, pos);
		}
	}

	/** Unifies the path by replacing all '\' and '/' to @a delimiter and by adding/removing the ending slash */
	std::string unifyPath(const std::string& path, bool slashSuffix = false, char delimiter = '\\');

	std::string getCurrentDirectory();

#ifdef _WINDOWS_
	template<class FILE> bool deleteFile(const FILE& file);

	__if_exists(StringRef) {
		template<> inline bool deleteFile<StringRef>(const StringRef& file) {
			return ::DeleteFile(file.a_str()) != 0;
		}
	}

	template<class LIST> inline int deleteFiles(const LIST& files) {
		int c = 0;
		for (LIST::const_iterator it = files.begin(); it != files.end(); ++it) {
			if (deleteFile(*it)) {
				c++;
			}
		}
		return c;
	}
#endif

#endif


	bool fileExists(const std::string& file);

	bool isDirectory(const std::string& path);

};
#endif	// __STAMINA_HELPERS_H__