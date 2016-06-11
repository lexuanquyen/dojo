#pragma once

#include "dojo_common_header.h"

#include "Stream.h"

namespace Dojo {
	///MemoryInputStream is a wrapper to read and stream from a preallocated memory area
	class MemoryInputStream : public Stream {
	public:

		MemoryInputStream(uint8_t* mem, int size);

		///reads up to "number" bytes from the stream into buf, returns the number of bytes read
		virtual int64_t read(uint8_t* buf, int64_t number);

		///returns the total bytes in the stream, -1 if this stream has no end
		virtual int64_t getSize();

		///returns the kind of access this stream provides, ie. read-only
		virtual Access getAccess();

		///returns the current reading/writing position
		virtual int64_t getCurrentPosition();

		///goes to the given position
		virtual int seek(int64_t offset, int64_t fromWhere = SEEK_SET);

	protected:

		uint8_t* pMem;
		int64_t mSize;

		int64_t mPosition;

	private:
	};
}
