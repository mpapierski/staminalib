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

#ifndef __STAMINA_STRINGBUFFER__
#define __STAMINA_STRINGBUFFER__

#pragma once

#include "Version.h"
#include "Assert.h"
#include "Memory.h"

namespace Stamina {

	/** Buffer template.
	Implements basic buffer operations.

	Supports:
	 - cheap referencing - operations on external buffer of unknown size
	 - data discard - ability to discard buffer's data without freeing the memory
	 - up to 0x0FFFFFFF (268.435.455) items of length
	 - always makes room for one (nulled on most types) item at the end for convenience (using data buffer as char*, passing it to winapi functions etc.). All buffer and data sizes don't count the terminating item and neither You should do that!
	 - all size parameters are about item count, not real memory occupation!

	*/
	template <typename TYPE>
	class Buffer {

	protected: // na pocz�tku, �eby by�y dobrze widoczne w debuggerze

		TYPE* _buffer;
		unsigned int _size; ///< Size of the buffer (how many items it can accomodate without reallocating memory)
		unsigned int _length; ///< Number of allocated & valid items

	public:

		const static ModuleVersion version;


//		friend class String;

		const static unsigned int pooledBufferSize = 64;
		const static unsigned int maxBufferSize = 0xFFFFFFFF;
		const static unsigned int lengthUnknown = 0xFFFFFFFF;
		const static unsigned int lengthDiscarded = 0xFFFFFFFE;
		const static unsigned int wholeData = 0xFFFFFFFF;

		class PassBuffer: public Buffer<TYPE> {
		public:
			PassBuffer(const Buffer<TYPE>& b):Buffer<TYPE>() {
				this->swap(const_cast<Buffer<TYPE>&>(b));
			}
		};

		class BufferRef: public Buffer<TYPE> {
		public:
			BufferRef(const Buffer<TYPE>& b):Buffer<TYPE>() {
				this->assignCheapReference(b);
			}
		};


	public:
		inline Buffer(): _size(0), _buffer(0), _length(0) {
		}

		inline Buffer(const Buffer& b): _size(0), _buffer(0), _length(0) {
			this->assign(b);
		}

		inline Buffer(const PassBuffer& pass): _size(0), _buffer(0), _length(0) {
			this->swap(const_cast<PassBuffer&>(pass));
		}


		inline Buffer(unsigned int initialSize): _size(0), _buffer(0), _length(0) {
			resize(initialSize, 0);
		}

		inline Buffer(const TYPE* data, unsigned int dataSize = lengthUnknown): _size(0), _buffer(0), _length(0) {
			assignCheapReference(data, dataSize);
		}


		inline ~Buffer() {
			freeBuffer();
		}

		inline void swap(Buffer<TYPE>& b) {
			TYPE* buffer = this->_buffer;
			int length = this->_length;
			int size = this->_size;
			this->_buffer = b._buffer;
			this->_length = b._length;
			this->_size = b._size;

			b._buffer = buffer;
			b._length = length;
			b._size = size;
			
		}

		/** Creates "cheap reference" - provided buffer will replace the one currently in use, until modification occurs.
		*/
		inline void assignCheapReference(const TYPE* data, unsigned int length = lengthUnknown) {
			//S_ASSERT(data);
			this->reset();
			this->_buffer = (TYPE*)data;
			this->_length = length;
		}

		inline void assignCheapReference(const Buffer<TYPE>& b) {
			this->assignCheapReference( b.getBuffer(), b.getLength() );
		}

		/** Makes a copy of data */
		inline void assign(const TYPE* data, unsigned int size) {
//			S_ASSERT(data);
			S_ASSERT(size <= maxBufferSize);
			this->makeRoom(size, 0);
			if (data) {
				copy(_buffer, data, size);
			}
			markValid(size);
		}

		inline void assign(const Buffer<TYPE>& b) {
			this->assign( b.getBuffer(), b.getLength() );
		}

		int compareBytes(const Buffer<TYPE>& b) const {
			int r;
			int l = min((this->getLength()), (b.getLength()));

			r = memcmp(this->getString(), b.getString(), l);
			if (r == 0) {
				if ((this->getLength()) > (b.getLength())) 
					return 1;
				else if ((this->getLength()) < (b.getLength())) 
					return -1;
			}
			return r;
		}

		bool equalBytes(const Buffer<TYPE>& b, unsigned int size) const {
			if (size > b.getLength() && this->getLength() != b.getLength()) {
				return false;
			}
			return memcmp(this->getString(), b.getString(), min(this->getLength(), size)) == 0;
		}

		int compare(const Buffer<TYPE>& b) const;

		bool equal(const Buffer<TYPE>& b, unsigned int size = wholeData) const;

		bool equalBuffers(const Buffer<TYPE>& b) const {
			return  (_buffer != 0 && _buffer == b._buffer);
		}

		bool operator == (const Buffer<TYPE>& b) const {
			return equal(b);
		}

		/** Calculates the number of bytes needed to store @a newSize of data. It only expands current buffer size. */
		inline unsigned int calculateRoom(unsigned int newSize) const {
			S_ASSERT(newSize <= maxBufferSize);
			unsigned int allocSize = getBufferSize();
			if (newSize > allocSize) {
				if (allocSize < (maxBufferSize/2)) 
					allocSize *= 2;
			}
			return allocSize > newSize ? allocSize : newSize;
		}

		/** Ensures that there's enough space in the buffer, resizing the buffer if necessary. See resize() for more information.

		@param keepData - the same as in resize()
		@return Returns true if buffor was (re)allocated
		*/
		inline bool makeRoom(unsigned int newSize, unsigned int keepData = wholeData) {
			S_ASSERT(newSize < maxBufferSize);
			if (hasOwnBuffer()) {
				if (newSize > getBufferSize()) {
					this->resize(calculateRoom(newSize), keepData);
					return true;
				}
			} else { // jak nie ma bufora, zawsze alokujemy nowy
				resize(newSize, keepData);
				return true;
			}
			return false;
		}

		/** If the buffer is a reference - make a copy of it 
		@param keepData - the same as in resize()
		*/
		inline void makeUnique(unsigned int keepData = wholeData) {
			if (isReference()) {
				if (keepData > getLength()) keepData = getLength();
				resize(keepData, keepData);
			}
		}

		/** Returns number of used bytes in the buffer */
		inline unsigned int getLength() const {
			if (_length == lengthDiscarded) {
				return 0;
			}
			if (_length == lengthUnknown) {
				const_cast<Buffer<TYPE> * >(this)->_length = 0;
				if (!isEmpty() && isValid()) {
					const_cast<Buffer<TYPE>* >(this)->_length = len(_buffer);
					//TYPE* ch = _buffer;
					//TYPE* end = isReference() ? (TYPE*)-1 : _buffer + _size;
					//while (ch < end && *(ch++)) const_cast<Buffer<TYPE>* >(this)->_length++;
				}
			}
			return _length;
		}
		inline int getKnownLength() const {
			return (signed)_length;
		}

		/** Appends data to the end of the buffer
		*/
		inline void append(const TYPE* data, unsigned int dataSize) {
			if (dataSize == 0) return;
			S_ASSERT(dataSize <= maxBufferSize);
			S_ASSERT(dataSize + getLength() <= maxBufferSize);
			const TYPE* oldBuffer = hasOwnBuffer() ? _buffer : 0;
			makeRoom(getLength() + dataSize);
			S_ASSERT(_buffer != 0);
			S_ASSERT(data != 0);
			S_ASSERT(getBufferSize() >= dataSize + getLength());
			if (data == oldBuffer) {
				data = _buffer;
			}
			copy(_buffer + getLength(), data, dataSize);
			markValid(getLength() + dataSize);
		}

		inline void append(const Buffer& b) {
			this->append(b.getBuffer(), b.getLength());
		}

		/** Prepends data to the buffer 
		*/
		inline void prepend(const TYPE* data, unsigned int dataSize) {
			if (dataSize == 0) return;
			unsigned currentLength = getLength();
			S_ASSERT(dataSize <= maxBufferSize);
			S_ASSERT(dataSize + currentLength <= maxBufferSize);
			const TYPE* oldBuffer = hasOwnBuffer() ? _buffer : 0;
			if (isValid()) {
				moveRight(0, dataSize); // wywoluje makeroom
			} else {
				makeRoom(dataSize, 0);
			}
			S_ASSERT(_buffer != 0);
			S_ASSERT(data != 0);
			S_ASSERT(getBufferSize() >= dataSize + currentLength);
			if (data == oldBuffer) {
				data = _buffer + dataSize;
			}
			copy(_buffer, data, dataSize, this->getBufferEnd(currentLength)); // copy za�atwia niszczenie nadpisywanych, bufferEnd zapewnia, �e ew. dodane nowe elementy nie b�d� "niszczone"
			markValid(currentLength + dataSize);
		}

		inline void prepend(const Buffer& b) {
			this->prepend(b.getBuffer(), b.getLength());
		}

		/** Inserts data into any position in the buffer.
		@param pos The position where to insert the data. Can be beyond currently allocated data!

		@warning This function allows to insert data at virtually any location. It automatically expands the buffer, leaving completely random data between insert position and the end of previous data. Use with @b extreme @b caution!
		@warning You CANNOT insert data that comes from the same buffer!
		*/
		inline void insert(unsigned int pos, const TYPE* data, unsigned int dataSize) {
			if (dataSize == 0) return;
			unsigned currentLength = getLength();
			if (pos > maxBufferSize) pos = currentLength;
			S_ASSERT(dataSize <= maxBufferSize);
			S_ASSERT(dataSize + currentLength <= maxBufferSize);
			S_ASSERT(pos + dataSize <= maxBufferSize);
			if (isValid()) {
				if (pos >= getLength()) {
					makeRoom(pos + dataSize);
				} else {
					moveRight(pos, dataSize); // wywoluje makeroom
				}
			} else {
				makeRoom(dataSize, 0);
				pos = 0;
			}
			S_ASSERT(_buffer != 0);
			S_ASSERT(data != 0);
			S_ASSERT(getBufferSize() >= pos + dataSize);

			copy(_buffer + pos, data, dataSize, this->getBufferEnd(currentLength)); // copy za�atwia niszczenie nadpisywanych
			markValid(std::max(currentLength + dataSize, pos + dataSize));
		}

		inline unsigned int insertInRange(unsigned int pos, const TYPE* data, unsigned int dataSize) {
			if (pos > getLength()) pos = getLength();
			insert(pos, data, dataSize);
			return pos;
		}

		inline void insert(unsigned int pos, const Buffer& b) {
			this->insert(pos, b.getBuffer(), b.getLength());
		}


		/**
		@warning You CANNOT substitute with data that comes from the same buffer!
		*/
		inline void replace(unsigned int pos, unsigned int count, const TYPE* data, unsigned int dataSize) {
			if (dataSize == 0) {
				this->erase(pos, count);
				return;
			}
			if (pos > getLength()) {
				this->append(data, dataSize);
				return;
			}
			if (count == 0) {
				this->insert(pos, data, dataSize);
				return;
			}
			S_ASSERT(data);
			unsigned int currentLength = getLength();
			unsigned int newLength = currentLength;
			if (isReference()) {
				TYPE* from = _buffer;
				if (count > currentLength || pos + count > currentLength) count = currentLength - pos;
				makeRoom(currentLength - count + dataSize, pos);
				S_ASSERT(_buffer);
				if (pos + count < currentLength) { // je�eli zostaje co� po...
					S_ASSERT(getBufferSize() >= dataSize + currentLength - count);
					// kopiujemy dane po zmienianym...
					copy(_buffer + pos + dataSize, from + pos + count, currentLength - pos - count);
				}
				newLength = currentLength - count + dataSize;
			} else if (count < currentLength && pos + count < currentLength) {
				if (dataSize > count) {
					moveRight(pos + count, dataSize - count);
				} else if (dataSize < count) {
					moveLeft(pos + count, count - dataSize);
				}
				newLength = lengthUnknown; // moveLeft/Right za�atwia ju� markValid
			} else {
				makeRoom(pos + dataSize, pos);
				newLength = pos + dataSize;
			}
			S_ASSERT(getBufferSize() >= pos + dataSize);

			copy(_buffer + pos, data, dataSize, this->getBufferEnd(currentLength)); // copy za�atwia niszczenie nadpisywanych
			if (newLength != lengthUnknown) {
				markValid(newLength);
			}
		}

		inline void replace(unsigned int pos, unsigned int count, const Buffer& b) {
			this->replace(pos, count, b.getBuffer(), b.getLength());
		}

		inline void erase(unsigned int pos, unsigned int count = wholeData) {
			if (!isValid() || count == 0) return;
			if (count > getLength() || pos + count > getLength()) count = getLength() - pos;
			if (pos + count >= getLength()) {
				truncate(pos);
				return;
			}
			moveLeft(pos + count, count);
		}


		/** Truncates buffer data at the specified position. */
		inline void truncate(unsigned int pos) {
			if (!isValid()) return;
			if (pos > getLength()) pos = getLength();
			if (isReference()) {
				makeUnique(pos);
			}
			this->markValid(pos);
		}


		/** Moves contents of the buffer to the left (optimized). Fails if buffer is not valid.
		@param start Position of the first character to move
		@param offset Offset of movement
		@param length The length of data to move
		@param truncate Truncates buffer after moved data
		*/
		void moveLeft(unsigned int start, unsigned int offset, unsigned int length = wholeData, bool truncate = true) {
			if (!isValid() || offset == 0) return;
			TYPE* from = _buffer;
			unsigned int dataLength = getLength();
			unsigned int newLength = dataLength;
			if (length > dataLength) length = dataLength;
			if (start < offset) {
				if (length > (offset - start)) {
					length -= (offset - start); // musimy skr�ci� d�ugo�� kopiowania o tyle, o ile p�niej je zaczynamy
				} else {
					length = 0;
				}
				start = offset;
			}
			if (start >= dataLength) {// nie ma sk�d ich przesuwa�
				if (truncate) 
					this->truncate(0);
				return;
			} 
			if (length + start > dataLength) {
				length = dataLength - start;
			}
			if (length == 0) {
				if (truncate) 
					this->truncate(start - offset);
				return;
			}
			if (isReference()) {
				// makeUnique bylby z�y bo nie rezerwowa�by bufora na przenoszenie
				resize(truncate ? start - offset + length : dataLength, start - offset); // kopiujemy tylko to co zostanie na pocz�tku
			}
			TYPE* to = _buffer;

			if (truncate/* || (length + start >= this->_length)*/) {
				newLength = start - offset + length;
			} else {
				// skoro nic nie ucinamy - d�ugo�� pozostaje bez zmian. Przy zmianie z reference mog�a si� jednak zmieni�, wi�c przywracamy star�.
				newLength = dataLength;
				if (getBuffer() != from) { // kopiujemy pozosta�o�ci
					S_ASSERT(_size >= (start - offset + length) + dataLength - (start + length - offset));

					copy(to + start - offset + length, from + start - offset + length, dataLength - (start + length - offset));
				}
			}
			S_ASSERT(from);
			S_ASSERT(to);
			S_ASSERT(_size >= start + length);
			from += start;
			to += start - offset;

			move(to, from, length); // move za�atwia niszczenie nadpisywanych
			markValid(newLength);
		}

		
		/** Moves contents of the buffer to the right (optimized). Fails if buffer is not valid.
		@param start Position of the first character to move
		@param offset Offset of movement
		@param length Length of data to move
		@param truncate Truncates data beyond moved data

		@warning Use with EXTREME caution! Too big offset will leave invalid items!
		*/
		void moveRight(unsigned int start, unsigned int offset, unsigned int length = wholeData, bool truncate = true) {
			if (!isValid() || offset == 0) return;
			unsigned int dataLength = getLength();
			if (length > dataLength) length = dataLength;
			if (start > dataLength) return;
			if (length > dataLength - start)
				length = dataLength - start;

			TYPE* from = _buffer;
			if (isReference() && truncate) {
				makeRoom(start + offset + length, start + offset); // potrzebujmey tylko co ma by� na pocz�tku...
			} else {
				makeRoom(std::max(truncate ? 0 : dataLength, start + offset + length)); // potrzebujemy wszystko
				from = _buffer;
			}
			TYPE* to = _buffer;

			S_ASSERT(from != 0);
			S_ASSERT(to != 0);
			S_ASSERT(_size > start + length);

            from += start;
			to += start + offset;

			move(to, from, length); // move za�atwia niszczenie nadpisywanych

			markValid(std::max(truncate ? 0 : dataLength, start + offset + length));

		}

		// -- more internal

		/** Resets the buffer completely (frees all allocated memory). Leaves active and major flags intact */
		inline void reset() {
			freeBuffer();
			_buffer = 0;
			_size = 0;
			_length = lengthDiscarded;
		}

		void resize(unsigned int newSize, unsigned int keepData = wholeData) {
			S_ASSERT(newSize <= maxBufferSize);
			if (keepData && newSize > 0 && this->isValid()) {
				if (keepData > getLength()) keepData = getLength();
				if (keepData > newSize) keepData = newSize;
				unsigned int size = newSize;
				TYPE* buffer = _alloc(size);
				S_ASSERT(buffer != 0);
				S_ASSERT(size >= keepData && size > 0);
				copy(buffer, _buffer, keepData);
				freeBuffer();
				this->_buffer = buffer;
				this->_size = size;
				markValid(keepData);
			} else {
				freeBuffer();
				unsigned int size = newSize;
				if (size > 0) {
					this->_buffer = _alloc(size);
					S_ASSERT(this->_buffer != 0);
					S_ASSERT(size > 0);
				}
				this->_size = size;
				this->_length = lengthDiscarded;
			}
		}

		/** Discards data in buffer, leaving the buffer allocated in memory so we can use it later. */
		inline void discard() {
			if (! this->isReference()) {
				this->_length = lengthDiscarded;
			} else {
				reset();
			}
		}


		// -- helpers

		inline void copy(TYPE* to, const TYPE* from, unsigned int count, const TYPE* end = (TYPE*)-1);

		inline void move(TYPE* to, const TYPE* from, unsigned int count, const TYPE* end = (TYPE*)-1);

		inline unsigned int len(const TYPE* str) const;

		inline void nullify(TYPE* where);


		// getters

		/** Returns the size of an allocated buffer. If nothing is allocated (ie. data is being referenced) - returns 0 

		@warning Space for \0 character is automatically allocated and not reflected in the buffer's size. So getBuffer()[getBufferSize()] = 0 is perfectly legal.
		*/
		inline unsigned int getBufferSize() const {
			return _size;
		}

		inline TYPE* getBuffer() const {
			return _buffer;
		}

		inline TYPE* getBufferEnd(unsigned size = lengthUnknown) const {
			if (size > getLength()) size = getLength();
			return _buffer + size;
		}


		inline const TYPE* getString() const {
			return isValid() ? _buffer : (TYPE*)L"";
		}

		//

		/** Returns true if data is valid (not discarded) */
		inline bool isValid() const {
			return _buffer != 0 && _length != lengthDiscarded;
		}

		inline bool isReference() const {
			return _size == 0 && _buffer != 0;
		}

		inline bool isEmpty() const {
			return _buffer == 0;
		}

		inline bool hasOwnBuffer() const {
			return _size > 0;
		}

		/** Marks data as valid (if it's not referenced) */
		inline void markValid(unsigned int length) {
			if ( this->hasOwnBuffer() ) {
				if (_length < lengthDiscarded && length < _length) {
					// usuwamy nadmiarowe elementy, musimy je oczy�ci�...
					this->destruct(length);
				} else if (length == lengthUnknown || length == lengthDiscarded) {
					this->destruct((unsigned int)0);
				}
				_length = length;
				if (this->_length != lengthUnknown) {
					S_ASSERT(_size >= _length && _size > 0);
					S_ASSERT(_buffer != 0);
					nullify(this->_buffer + this->_length);
				}
			}
		}

		inline void markValid() {
			markValid( this->_length == lengthDiscarded ? lengthUnknown : this->_length );
		}

		inline void setLength(unsigned int length) {
			_length = length;
		}


	private:

		/** Calls desctructors for object within valid range. */
		inline void destruct(unsigned int pos, unsigned int count = wholeData);

		inline void destruct(TYPE* item);

		inline void freeBuffer() {
			if (hasOwnBuffer()) {
				destruct((unsigned int)0);
				_free(_buffer, getBufferSize());
			}
			_length = lengthDiscarded;
			_buffer = 0;
			_size = 0;
		}

		/** Returned memory block is always @a size + 1 of size */
		static TYPE* _alloc(unsigned int &size) {
			return Memory::allocBuffer<TYPE>(size);
		}
		static void _free(TYPE* buff, unsigned int size) {
			Memory::freeBuffer<TYPE>(buff, size);
		}

	};

	template<> inline void Buffer<char>::copy(char* to, const char* from, unsigned int count, const char* end) {
		::memcpy(to, from, count);
	}
	template<> inline void Buffer<wchar_t>::copy(wchar_t* to, const wchar_t* from, unsigned int count, const wchar_t* end) {
		::wmemcpy(to, from, count);
	}
	template<> inline void Buffer<unsigned char>::copy(unsigned char* to, const unsigned char* from, unsigned int count, const unsigned char* end) {
		::memcpy(to, from, count);
	}
	template<> inline void Buffer<int>::copy(int* to, const int* from, unsigned int count, const int* end) {
		::memcpy(to, from, count * sizeof(int));
	}
	template<> inline void Buffer<unsigned int>::copy(unsigned int* to, const unsigned int* from, unsigned int count, const unsigned int* end) {
		::memcpy(to, from, count * sizeof(unsigned int));
	}
	/*template <typename TYPE> // wska�niki...
	inline void Buffer<TYPE*>::copy(TYPE* to, const TYPE* from, unsigned int count) {
		::memcpy(to, from, count * sizeof(void*));
	}*/
	template <typename TYPE> // pozosta�e...
	inline void Buffer<TYPE>::copy(TYPE* to, const TYPE* from, unsigned int count, const TYPE* end) {
		if (end == (TYPE*)-1) {
			end = this->getBufferEnd();
		}
		while (count--) {
			if (to >= this->getBuffer() && to < end) 
				this->destruct(to); // niszczy nadpisywany element
			new (to++) TYPE(*(from++));  // inicjalizuje kolejne obiekty w pami�ci w adresach to danymi z from
		}
	}



	template<> inline void Buffer<char>::move(char* to, const char* from, unsigned int count, const char* end) {
		::memmove(to, from, count);
	}
	template<> inline void Buffer<wchar_t>::move(wchar_t* to, const wchar_t* from, unsigned int count, const wchar_t* end) {
		::wmemmove(to, from, count);
	}
	template<> inline void Buffer<unsigned char>::move(unsigned char* to, const unsigned char* from, unsigned int count, const unsigned char* end) {
		::memmove(to, from, count);
	}
	template<> inline void Buffer<int>::move(int* to, const int* from, unsigned int count, const int* end) {
		::memmove(to, from, count * sizeof(int));
	}
	template<> inline void Buffer<unsigned int>::move(unsigned int* to, const unsigned int* from, unsigned int count, const unsigned int* end) {
		::memmove(to, from, count * sizeof(unsigned int));
	}
	/*template <typename TYPE> // wska�niki...
	inline void Buffer<TYPE*>::move(TYPE** to, const TYPE** from, unsigned int count) {
		::memmove(to, from, count * sizeof(void*));
	}*/
	template <typename TYPE> // pozosta�e...
	inline void Buffer<TYPE>::move(TYPE* to, const TYPE* from, unsigned int count, const TYPE* end) {
		if (to > from && to <= from + count) {
			to += count - 1;
			from += count - 1;
			if (end == (TYPE*)-1) {
				end = this->getBufferEnd();
			}
			while (count--) {
				if (to >= this->getBuffer() && to < end) 
					this->destruct(to); // niszczy nadpisywany element
				new (to--) TYPE(*(from--));  // inicjalizuje kolejne obiekty w pami�ci w adresach to danymi z from
			}
		} else {
			copy(to, from, count, end);
		}
	}


	template<> inline unsigned int Buffer<char>::len(const char* str) const {
		return std::strlen(str);
	}
	template<> inline unsigned int Buffer<wchar_t>::len(const wchar_t* str) const {
		return wcslen(str);
	}
	template <typename TYPE>
	inline unsigned int Buffer<TYPE>::len(const TYPE* str) const {
		return 0;
	}


	template<> inline void Buffer<char>::nullify(char* where) {
		*where = 0;
	}
	template<> inline void Buffer<wchar_t>::nullify(wchar_t* where) {
		*where = 0;
	}
	template<> inline void Buffer<unsigned char>::nullify(unsigned char* where) {
		*where = 0;
	}
	/*template<typename TYPE>
	inline void Buffer<TYPE*>::nullify(TYPE** where) {
		*where = 0;
	}*/
	template<typename TYPE>
	inline void Buffer<TYPE>::nullify(TYPE* where) {
		if (sizeof(TYPE) == 4) {
			*((int*)where) = 0;
		}
	}


	template <> 
	inline void Buffer<char>::destruct(unsigned int pos, unsigned int count) {
	}
	template <> 
	inline void Buffer<wchar_t>::destruct(unsigned int pos, unsigned int count) {
	}
	template <> 
	inline void Buffer<unsigned char>::destruct(unsigned int pos, unsigned int count) {
	}
	template <> 
	inline void Buffer<int>::destruct(unsigned int pos, unsigned int count) {
	}
	template <> 
	inline void Buffer<unsigned int>::destruct(unsigned int pos, unsigned int count) {
	}

	template<typename TYPE>
	inline void Buffer<TYPE>::destruct(unsigned int pos, unsigned int count) {
		if (this->isReference() == true || this->isValid() == false) return;
		while (count-- && pos < this->_length) {
			this->destruct(&_buffer[pos]);
			//delete (this->_buffer + pos, this->_buffer + pos);
			pos++;
		}
	}

	template<typename TYPE> // pisanie innych w zasadzie jest zb�dne...
	inline void Buffer<TYPE>::destruct(TYPE* item) {
		item->~TYPE();
	}


	template<> inline int Buffer<char>::compare(const Buffer<char>& b) const {
		return this->compareBytes(b);
	}
	template<> inline int Buffer<wchar_t>::compare(const Buffer<wchar_t>& b) const {
		return this->compareBytes(b);
	}
	template<> inline int Buffer<unsigned char>::compare(const Buffer<unsigned char>& b) const {
		return this->compareBytes(b);
	}
	template<> inline int Buffer<int>::compare(const Buffer<int>& b) const {
		return this->compareBytes(b);
	}
	template <typename TYPE>
	inline int Buffer<TYPE>::compare(const Buffer<TYPE>& b) const {
		return this->equal(b) ? 0 : 1;
	}

	template<> inline bool Buffer<char>::equal(const Buffer<char>& b, unsigned int size)  const {
		return this->equalBytes(b, size);
	}
	template<> inline bool Buffer<wchar_t>::equal(const Buffer<wchar_t>& b, unsigned int size) const {
		return this->equalBytes(b, size);
	}
	template<> inline bool Buffer<unsigned char>::equal(const Buffer<unsigned char>& b, unsigned int size) const {
		return this->equalBytes(b, size);
	}
	template<> inline bool Buffer<int>::equal(const Buffer<int>& b, unsigned int size) const {
		return this->equalBytes(b, size);
	}
	template <typename TYPE>
	inline bool Buffer<TYPE>::equal(const Buffer<TYPE>& b, unsigned int size) const {
		if (size > b.getLength() && this->getLength() != b.getLength()) {
			return false;
		}
		TYPE* x = this->getBuffer();
		TYPE* y = b.getBuffer();
		TYPE* end = this->getBufferEnd(size);
		while (x != end) {
			if (*(x++) != *(y++)) {
				return false;
			}
		}
		return true;
	}




	template<typename TYPE>
	const ModuleVersion Buffer<TYPE>::version = ModuleVersion(versionClass, "Stamina::Buffer", Version(1, 0, 0, 0));


	template Buffer<char>;
	template Buffer<wchar_t>;

	typedef Buffer<unsigned char> ByteBuffer;

};

#endif