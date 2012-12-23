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
#pragma once

#if !defined(STAMINA_LIB_VERSION_H_INCLUDED_)
#define STAMINA_LIB_VERSION_H_INCLUDED_


#include "Stamina.h"
#include <boost/operators.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <cstring>

namespace Stamina {

	class Version: public boost::totally_ordered<Version> {
	public:

		inline Version(int version = 0) {
			this->major = (short)( ((version)>>28)&0xF );
			this->minor = (short)( ((version)>>20)&0xFF );
			this->release = (short)( ((version)>>12)&0xFF );
			this->build = (short)( ((version))&0xFFF );
		}
		inline Version(int high, int low) {
			this->major = (short)( high >> 16 );
			this->minor = (short)( high & 0xFFFF );
			this->release = (short)( low >> 16 );
			this->build = (short)( low & 0xFFFF );
		}
		inline Version(short major, short minor, short release, short build) {
			this->major = major;
			this->minor = minor;
			this->release = release;
			this->build = build;
		}
		inline Version(const Version& b) {
			*this = b;
		}
		inline Version(const char* str) {
			minor = release = build = 0;
			char* v = (char*)str;
			this->major = (short)std::strtoul(v, &v, 10);
			if (*v != '.') return;
			this->minor = (short)std::strtoul(++v, &v, 10);
			if (*v != '.') return;
			this->release = (short)std::strtoul(++v, &v, 10);
			if (*v != '.') return;
			this->build = (short)std::strtoul(++v, &v, 10);
			if (*v != '.') return;
		}
		inline Version& operator = (const Version& b) {
			major = b.major;
			minor = b.minor;
			release = b.release;
			build = b.build;
			return *this;
		}
		inline bool operator == (const Version& b) const {
			return major == b.major && minor == b.minor && release == b.release && build == b.build;
		}
		inline bool operator > (const Version& b) const {
			return major > b.major && minor > b.minor && release > b.release && build > b.build;
		}
		inline bool empty() const {
			return !major && !minor && !release && !build;
		}

		inline unsigned int getInt() const {
			return ((((major)&0xF)<<28) | (((minor)&0xFF)<<20) | (((release)&0xFF)<<12) | ((build)&0xFFF));
		}
#ifdef _STRING_
#define __STAMINA_VERSION__WITH_STRING
		/**Returns version string (x.x.x.x).
		@param Minimum number of version parts to return
		*/
		std::string getString(char elements = 2) const {
			char buff [10];
			std::string s;
			if (elements > 0 || major || minor || release || build)
				s += _itoa(major, buff, 10);
			if (elements > 1 || minor || release || build) {
				s += ".";
				s += _itoa(minor, buff, 10);
			}
			if (elements > 2 || release || build) {
				s += ".";
				s += _itoa(release, buff, 10);
			}
			if (elements > 3 || build) {
				s += ".";
				s += _itoa(build, buff, 10);
			}
			return s;
		}
#endif

	public:
		short major, minor, release, build;
	};


	enum enVersionCategory {
		versionUnknown,
		versionClass,
		versionModule,
		versionAPI
	};


	/** Defines version information about class/module/API. */
	class ModuleVersion {
	public:
		/**
		@warn name @b must be a static string! Don't use std::string, String, or anything like that in here!
		*/
		ModuleVersion(enVersionCategory category, const char* name, const Version& version):_category(category), _name(name), _version(version) {
		}

		bool operator == (const ModuleVersion&b) {
			return _category == b._category && boost::iequals(_name, b._name) && _version == b._version;
		}

		enVersionCategory getCategory() const {
			return _category;
		}

		const char* getName() const {
			return _name;
		}

		const Version& getVersion() const {
			return _version;
		}

	private:
		enVersionCategory _category;
		const char* _name;
		const Version _version;
	};

};

#ifndef __STAMINA_VERSIONCONTROL__

#define STAMINA_REGISTER_VERSION(NAME, MODULE)

#define STAMINA_REGISTER_CLASS_VERSION(CLASS)

#endif

#endif /* STAMINA_LIB_VERSION_H_INCLUDED_ */