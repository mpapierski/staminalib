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
#ifndef __DT_FILEBIN__
#define __DT_FILEBIN__


#include <io.h>
#include "FileBase.h"
#include "..\Helpers.h"
#include "..\FindFileFiltered.h"

namespace Stamina { namespace DT {



	const char binRowSeparator = '\n';
	const char binColumnSeparator = '\n';
	const char binVersionMaj = '3';
	const char binVersionMin = '6';

	const __int64 minimumBackupPeriod = 60 * 60; // an hour

	class FileBin : public FileBase {

	public:

		STAMINA_OBJECT_CLASS(DT::FileBin, DT::FileBase);

		friend class TestFileBin;

		enum enFileFlags {
			fflagFragmented = 0x10000,
			/**Uses crypt procedures on all fields*/
			fflagCryptAll = 0x20000,

			fflagNone = 0,
		};

		enum enDataFlags {
			dflagNone = 0,

			dflagLastId = 1,
			dflagPasswordDigest = 2,
			dflagDBId = 4,

			dflagParams = 8, /*TODO: */
			dflagCreated = 0x10,
			dflagModified = 0x20,
			dflagLastBackup = 0x40,
			dflagPassSalt = 0x80,
			dflagXorSalt = 0x100,

			basicDataFlags = dflagLastId | dflagPasswordDigest |  dflagCreated | dflagModified | dflagLastBackup | dflagPassSalt | dflagXorSalt,

			deprecatedDataFlags = dflagDBId,

			requiredDataFlags = dflagLastId | dflagPasswordDigest | dflagPassSalt | dflagXorSalt,

			newDataFlags = dflagParams | dflagCreated | dflagModified | dflagLastBackup | dflagPassSalt | dflagXorSalt,
		};

		enum enRowDataFlags {
			rdflagRowId = 1,
			rdflagNone = 0,
		};

		enum enPositionOrigin {
			fromCurrent = SEEK_CUR,
			fromEnd = SEEK_END,
			fromBeginning = SEEK_SET
		};

	public:

		FileBin();
		FileBin(DataTable& table);
		~FileBin();

		void init();

		virtual void reset();


		void open (const StringRef& fn , enFileMode mode) throw (...);
		void close ();

		enFileFlags getFileFlags() {
			return _fileFlag;
		}
		/**Sets file flags.
		@warning You are responsible to set them wisely. Flags are always stored in the file when closing (together with row count). Extreme caution should be taken when appending data to the file.
		*/
		void setFileFlag(enFileFlags flag, bool setting) {
			if (setting)
				_fileFlag = (enFileFlags) (_fileFlag | flag);
			else
				_fileFlag = (enFileFlags) (_fileFlag & (~flag));
		}
		bool hasFileFlag(enFileFlags flag) {
			return (_fileFlag & flag) != 0;
		}

		String getOpenedFileName() {
			if (! _temp_fileName.empty())
				return _temp_fileName;
			else
				return _fileName;
		}
		String getFileName() {
            return _fileName;
		}
		bool isUsingTemp() {
			return _temp_fileName.empty() == false && _temp_enabled == true;
		}

		/**Returns true if file is in the version that supports new crypt functions*/
		inline bool versionNewCrypt() {
			if (_verMaj > '3') return true;
			if (_verMaj < '3') return false;
			return _verMin >= '5';
		}

		inline bool versionNewString() {
			if (_verMaj > '3') return true;
			if (_verMaj < '3') return false;
			return _verMin >= '6';
		}

		inline void setOldCryptVersion() {
			_verMaj = '3';
			_verMin = '4';
		}

		virtual Version getVersion() {
			return Version(_verMaj, _verMin, 0, 0);
		}


		public:


		enResult readRow(tRowId row, bool readId = true) throw (...) {
			return readPartialRow(row, 0, readId);
		}

		void writeRow(tRowId row) throw (...);

		virtual void writeHeader() throw (...);

		virtual void readHeader() throw (...);

		void writeDescriptor() throw (...);

		void readDescriptor() throw (...);

		enResult readPartialRow(tRowId row , tColId* columns, bool readId = true) throw (...);

		bool findNextRow(bool validate = false);

		bool isFileFinished() {
			return feof(_file);
		}

		enResult skipRow();


		void seekToBeginning() {
			this->setFilePosition(0, fromBeginning);
		}
		void seekToEnd() {
			this->setFilePosition(0, fromEnd);
		}

		void writeState() throw (...);
		//void readCount() throw (...);

		void setErasedRow(bool overwrite=true , int testIndex=0) throw (...);


		void setFilePosition(int pos , enPositionOrigin origin) throw(...) {
			if (fseek(_file , pos , origin))
				throw DTFileException();
		}

		unsigned int getFilePosition() {
			return ftell(_file);
		}

		/**Checks table's password digest with file's one*/
		virtual bool isAuthenticated();

		unsigned int getStoredRowsCount() {
			return _storedRowsCount;
		}


		/**Creates a backup of specified file*/
		static void backupFile(const StringRef& filename, bool move);
		/**Creates a backup of the file we are currently operating on...
		This function automatically determines if backup is really needed, and sets lastBackupTime property.
		*/
		void backupFile(bool move = true);

		static String makeBackupFilename(const StringRef& filename, const Time64& time = Time64(true)) {
			return filename + time.strftime(".%Y-%m-%d %H-%M-%S.bak");
		}

		/** Removes old backups leaving only few of them.
		@param filename - If it points to .dtb file, only backups of these file are cleaned. Otherwise it cleans up whole directory.
		*/
		static void cleanupBackups(const StringRef& filename);

		/** Restores specified backup file to it's original filename */
		static void restoreBackup(const StringRef& filename);

		/** Restores latest backup of specified filename */
		bool restoreLastBackup(const StringRef& filename = "");

		Date64 findLastBackupDate(const StringRef& filename = "");
		String findLastBackupFile(const StringRef& filename = "", Date64* date = 0);


	protected:

		/**Generates new password digest.
		Should be used only when creating new file.
		@warning It's already being called inside writeHeader!
		*/
		void generatePasswordDigest(bool newSalt);

		/**Generates xor mask digest.
		Should be called after loading (with newSalt==false),
		and before saving to the new file (with newSalt==true).
		@warning It's already being called inside readHeader && writeHeader!
		*/
		void generateXorDigest(bool newSalt);


		inline void readData(void* buffer, int size, unsigned int* decrement = 0) throw(...) {
			if (fread(buffer, size, 1, _file) < 1) {
				throw DTFileException();
			}
			if (decrement) {
				if (*decrement < size) {
					throw DTException(errBadFormat);
				}
				*decrement -= size;
			}
		}

		inline void writeData(const void* buffer, int size, unsigned int* increment = 0) throw(...) {
			if (fwrite(buffer, size, 1, _file) < 1) {
				throw DTFileException();
			}
			if (increment)
				*increment += size;
		}

		PassStringRef readString(const Column* col = 0, unsigned int* decrement = 0) throw(...);

		void writeString(const StringRef& s, const Column* col = 0, unsigned int* increment = 0) throw(...);

		PassStringRef readString(unsigned int* decrement = 0) throw(...) {
			return readString(0, decrement);
		}

		void writeString(const StringRef& s, unsigned int* increment = 0) throw(...) {
			writeString(s, 0, increment);
		}


		void readCryptedData(const Column* col, void* buffer, int size, unsigned int* decrement = 0) throw(...);

		void writeCryptedData(const Column* col, const void* buffer, int size, unsigned int* increment = 0) throw(...);


		inline void updateFileSize() {
			_fileSize = _filelength(_file->_file);
		}



	public:

		bool useTempFile;
		bool warningDialogs;
		bool makeBackups;

	protected:

		FILE * _file;
		int _pos_data;
		int _pos_rows;
		int _pos_cols;
		int _pos_dataLastId;
		int _pos_state;
		int _storedRowsCount;
		//int mode;
		char _verMaj , _verMin;
		enFileFlags _fileFlag;
		unsigned int _dataSize;
		enDataFlags _dataFlag;


		/**File's password digest used for authentication.*/
		MD5Digest _passwordDigest;
		unsigned int _passwordSalt;

		/**File's xor mask digest.*/
		MD5Digest _xorDigest;
		unsigned int _xorSalt;

		bool _temp_enabled;
		String _temp_fileName; // Podczas uzywania tempa plik z ... tempem :)

		unsigned int _fileSize;


	};

} }

#endif
