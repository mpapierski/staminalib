/*
 *  Stamina.LIB
 *  
 *  Please READ /License.txt FIRST! 
 * 
 *  Copyright (C)2003,2004,2005 Rafa� Lindemann, Stamina
 *
 *  $Id$
 */

#pragma once

#include <string>
#include <Stamina\Object.h>
#include <Stamina\ObjectPtr.h>
#include <Stamina\String.h>


namespace Stamina { namespace Unique {

	enum enDomainId {
		domainOther,
		domainDomain,
		domainRange,
		domainTable,
		domainIMessage,
		domainMessageType,
		domainAction,
		domainNet,
		domainIcon,
		domainNotFound = -1
	};
	enum enRangeId {
		rangeMain = 0,
		rangeStatic,
		rangeUnique,
		rangeUniqueLow,
		rangeDefault = -1,
		rangeNotFound = -1
	};

	typedef enDomainId tDomainId;
	typedef enRangeId tRangeId;
	typedef unsigned int tId;

	const tId idNotFound = -1;
	const tId commonUniqueStart = 0x00800000;

	class iRange: public iSharedObject {
	public:

		STAMINA_OBJECT_CLASS(Stamina::Unique::iRange, iSharedObject);

		/** Rodzaj zakresu		
		*/
		enum enType {
			typeBoth = 3,
			// Zakres tylko dla identyfikator�w statycznych
			typeStatic = 1,
			// Zakres tylko dla identyfikator�w unikalnych
			typeUnique = 2
		};

		virtual tId __stdcall createUniqueId() =0;
		virtual bool __stdcall canRegisterId(tId id) const =0;
		virtual bool __stdcall canRegisterName() const =0;
		virtual bool __stdcall containsId(tId id) const =0;

		virtual tRangeId __stdcall getRangeId() const =0;
		virtual unsigned int __stdcall getPriority() const =0;
		virtual enType __stdcall getType() const =0;
		virtual unsigned int __stdcall getMinimum() const =0;
		virtual unsigned int __stdcall getMaximum() const =0;
		virtual unsigned int __stdcall getNextUnique() const =0;

	protected:
	};

	typedef SharedPtr<iRange> oRange;

	/** Tworzy podstawowy zakres identyfikator�w
	*
	* \param id Unikalny identyfikator zakresu
	* \param priority Priorytet zakresu okre�laj�cy jego szczeg�owo��. Bardziej og�lne zakresy powinny mie� ni�szy priorytet. Zakres domy�lny powinien mie� priorytet 0.
	* \param type Rodzaj zakresu
	* \param minimum Minimalny identyfikator
	* \param maximum Maxymalny identyfikator
	* \param uniqueStart Pierwszy identyfikator do automatycznego nadawania. -1 ustawia ten parametr na warto�� @a minimum.
	* \return 
	*/
	oRange basicRange(tRangeId rangeId, iRange::enType type, unsigned int priority,  unsigned int minimum = 1, unsigned int maximum = 0x7FFFFFFF, unsigned int uniqueStart=-1);


	class iDomain: public iSharedObject {
	public:

		STAMINA_OBJECT_CLASS(Stamina::Unique::iDomain, iSharedObject);


		virtual tDomainId __stdcall getDomainId() const =0;
		virtual String __stdcall getDomainName() const =0;

		virtual tId __stdcall getId(const StringRef& name) const =0;
		virtual String __stdcall getName(tId id) const =0;
		virtual oRange __stdcall inRange(tId id, iRange::enType check = iRange::typeBoth) const =0;
		bool idExists(tId id) const {
			return getName(id).empty() == false;
		}
		bool nameExists(const StringRef& name) const {
			return getId(name) != idNotFound;
		}

		virtual bool __stdcall registerId(tId id, const StringRef& name) =0;
		virtual tId __stdcall registerName(const StringRef& name, const oRange& range) =0;
		tId registerName(const StringRef& name, tRangeId range = rangeDefault) {
			return registerName(name, getRange(range));
		}
		virtual bool __stdcall unregister(tId id) =0;
		virtual bool __stdcall unregister(const StringRef& name) =0;

		virtual bool __stdcall addRange(const oRange& range, bool setAsDefault = false) =0;
		bool rangeExists(tRangeId rangeId) {
			return getRange(rangeId) == true;
		}
		bool rangeExists(const oRange& range) {
			return rangeExists(range->getRangeId());
		}
		virtual oRange __stdcall getRange(tRangeId rangeId) =0;
        virtual bool __stdcall removeRange(const oRange& range) =0;

		bool idInRange(tRangeId rangeId, tId id) {
			oRange range = getRange(rangeId);
			if (!range) return false;
			return range->containsId(id);
		}

	};


	typedef SharedPtr<iDomain> oDomain;

	oDomain basicDomain(tDomainId id, const StringRef& name);


	class iDomainList: public iSharedObject {
	public:

		STAMINA_OBJECT_CLASS(Stamina::Unique::iDomainList, iSharedObject);


		virtual oDomain __stdcall getDomain(tDomainId id) const =0;
		virtual oDomain __stdcall getDomain(const StringRef& name) const =0;
		virtual void __stdcall registerDomain(const oDomain& domain) =0;
		virtual void __stdcall unregisterDomain(const oDomain& domain) =0;

		bool domainExists(tDomainId id) const {
			return getDomain(id) == true;
		}

	};

	typedef SharedPtr<iDomainList> oDomainList;

	oDomainList basicDomainList();

	iDomainList* instance();
	void setInstance(const oDomainList& list);


	inline tId getId(tDomainId domainId, const StringRef& name) {
		oDomain domain = instance()->getDomain(domainId);
		if (!domain) return idNotFound;
		return domain->getId(name);
	}
	inline String getName(tDomainId domainId, tId id) {
		oDomain domain = instance()->getDomain(domainId);
		if (!domain) return "";
		return domain->getName(id);
	}

	inline bool registerId(tDomainId domainId, tId id, const StringRef& name) {
		oDomain domain = instance()->getDomain(domainId);
		if (!domain) return idNotFound;
		return domain->registerId(id, name);
	}
	inline tId registerName(tDomainId domainId, const StringRef& name, tRangeId range = rangeDefault) {
		oDomain domain = instance()->getDomain(domainId);
		if (!domain) return idNotFound;
		return domain->registerName(name, range);
	}

	inline oDomain getDomain(tDomainId domainId) {
		return instance()->getDomain(domainId);
	}


};};