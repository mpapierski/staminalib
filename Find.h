#pragma once
/*
 *  Stamina.LIB
 *  
 *  Please READ /License.txt FIRST! 
 * 
 *  Copyright (C)2003,2004,2005 Rafa� Lindemann, Stamina
 *
 *  $Id: DT.h 93 2005-08-22 00:23:52Z hao $
 */

/**@file All basic definitions needed to work with DataTable

*/

#include "DT.h"
#include "Value.h"

namespace Stamina { namespace DT {

	class Find {
	public:

		enum Operation {
			eq, neq, gt, gteq, lt, lteq
		};

		inline Find(Operation operation, tColId col, const oValue& value):operation(operation), col(col), value(value) {}

		static inline Find EqStr(tColId col, const StringRef& str) {
			return Find(eq, col, new Value_string(str));
		}
		static inline Find EqInt(tColId col, int value) {
			return Find(eq, col, new Value_int(value));
		}

		Operation operation;
		tColId col;
		oValue value;
	};


} }