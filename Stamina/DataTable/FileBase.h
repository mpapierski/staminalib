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
#ifndef __DT_FILEBASE__
#define __DT_FILEBASE__


#include "DataTable.h"

#include "../ObjectImpl.h"

namespace Stamina { namespace DT {

	enum enFileMode {
		fileClosed = 0,
		fileRead = 1,
		fileWrite = 2,
		fileReadWrite = 3,
		fileAppend = 4,
	};

	enum enFileOperation {
		noOperation = 0,
		loadColumns = 1,
		saveOldCryptVersion = 2,
	};

	inline enFileOperation operator | (const enFileOperation & a, const enFileOperation & b) {
		return (enFileOperation) ((int)a | (int)b); 
	}



	class FileBase: public Object<iObject> {
	public:
		//   virtual

		STAMINA_OBJECT_CLASS(DT::FileBase, ::Stamina::iObject);

		FileBase();
		FileBase(DataTable& table);
		~FileBase();

		/**Assigns a table to this file handler*/
		virtual void assign(DataTable& table);

		/**Resets internal data.
		Should be always before save when using the same File object for loading & saving.
		*/
		virtual void reset();

		inline bool isWriteFailed() {
			return _writeFailed;
		}
		inline void setWriteFailed(bool state) {
			_writeFailed = state;
		}

		virtual Version getVersion() {
			return Version();
		}

		/**Loads data from specified file.
		Uses only column descriptor from assigned table.
		*/
		virtual enResult load (const StringRef& fn = "", enFileOperation operation = loadColumns); // wgrywa caly plik
		/**Loads all data from specified file.
		Uses merged column descriptors from assigned table and specified file.
		*/
		virtual enResult loadAll (const StringRef& fn = "") {
			return this->load(fn, loadColumns);
		}

		virtual enResult FileBase::loadPartial (const StringRef& fn, unsigned int start, unsigned int count, unsigned int* seekPtr = 0, enFileOperation operation = loadColumns);

		/**Stores all information from assigned table. The file is recreated.*/
		virtual enResult save (const StringRef& fn = "", enFileOperation operation = noOperation); // zapisuje caly plik

		/**Appends table data to specified file using only file's column descriptor.*/
		virtual enResult append (const StringRef& fn = "", enFileOperation operation = noOperation); // dopisuje elementy

		// placeHolders

		/**Opens file in specified mode*/
		virtual void open (const StringRef& fn , enFileMode mode) throw (...) = 0;
		virtual void close ()=0; // zamyka plik
		//virtual examine() {return 0;}; // Zapisuje lokalizacje wierszy
		//virtual examinePos(tRowId row) {return 0;}; // zapisuje lokalizacje wiersza
		//virtual int readRow(int row)=0; // wczytuje wiersz
		//virtual freeRow(int row)=0; // zwalnia wiersz

		/**Reads all rows from file*/
		virtual void readRows(bool skipFailed) throw (...); // wczytuje wiersze

		//virtual int next() {return 0;} // przesuwa sie na nastepny
		//virtual int prev() {return 0;} // przesuwa sie na poprzedni


		virtual void setColumns(const ColumnsDesc& cols) {
			_fcols = cols;
		}
		const ColumnsDesc& getColumns() {
			return _fcols;
		}

		void mergeLoadedColumns() {
       		this->_table->mergeColumns(_fcols);
		}

		inline bool isReadable() {
			return !_recreating && (_opened & (fileRead | fileAppend));
		}
		inline bool isCreatingNewFile() {
			return _recreating;
		}

		inline DataTable* getDT() {
			return _table;
		}

		String getFilename() {
			return _fileName;
		}

		virtual enFileMode getFileMode() {
			return _opened;
		}

		virtual bool isOpened() {
			return getFileMode() != fileClosed;
		}

		/**Checks table's password digest with file's one*/
		virtual bool isAuthenticated() {
			return true;
		}

		/**Runs whole authentication procedure. Should be called after loading descriptor*/
		virtual void authenticate() throw (...) {
			if (!this->isAuthenticated()) throw DTException(errNotAuthenticated);
		}

		virtual bool isFileFinished()=0;
		virtual enResult skipRow()=0;

	protected:

		/**Reads row from current position in file and stores it under @a row
		If there's nothing to read returns false instead of throwing an exception.
		*/
		virtual enResult readRow(tRowId row, bool readId = true) throw (...) =0;

		/**Stores data under @a row into current position in file*/
		virtual void writeRow(tRowId row) throw (...) =0;

		/**Stores column descriptor*/
		virtual void writeDescriptor() throw (...) =0;

		/**Reads column descriptor*/
		virtual void readDescriptor() throw (...) =0;


		virtual void writeHeader() throw (...) =0;
		virtual void readHeader() throw (...) =0;

		/**Reads only specified rows from current position in file and stores them under @a row.
		@param columns - null terminated list of columns
		*/
		virtual enResult readPartialRow(tRowId row , tColId* columns, bool readId = true) throw (...) =0;

		/**Goes to next row in file*/
		virtual bool findNextRow(bool validate = false)=0; // przechodzi do nast�pnej linijki (w razie gdy freadrow wywali b��d)
		

		virtual void seekToBeginning()=0;
		virtual void seekToEnd()=0;



	protected:
		DataTable * _table;
		enFileMode _opened;
		bool _recreating;
		bool _headerLoaded;
		ColumnsDesc _fcols;
		StringRef _fileName;
		bool _writeFailed;

	};

	class DTFileException: public DTException {
	public:

		STAMINA_OBJECT_CLASS(DT::DTFileException, DTException);

		DTFileException():DTException((enError)(errFileError | errno)) {
		
		}
	};

} }

#endif