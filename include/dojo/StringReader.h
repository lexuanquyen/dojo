#pragma once

#include "dojo_common_header.h"

#include "dojostring.h"

namespace Dojo {
	///StringReader wraps a utf::string to help parsing
	class StringReader {
	public:

		StringReader(utf::string string);

		///returns a new unicode character or 0 if the stream ended
		utf::character get();

		bool startsWith(utf::string_view str) const;

		void back();

		//TODO move all to Utils
		static bool isNumber(uint32_t c);

		static bool isLowerCaseLetter(uint32_t c);

		static bool isUpperCaseLetter(uint32_t c);

		static bool isLetter(uint32_t c);

		///returns if the given char is ok for a name, 0-9A-Za-z
		static bool isNameCharacter(uint32_t c);

		static bool isHex(uint32_t c);

		static bool isWhiteSpace(uint32_t c);

		void skipWhiteSpace();

		uint8_t getHexValue(uint32_t c);

		utf::string::const_iterator getCurrentIndex() const;

		///reads a formatted hex
		unsigned int readHex();

		float readFloat();

		utf::string_view readString();

		///reads n raw bytes from the file
		void readBytes(void* dest, int sizeBytes);

		utf::string_view getString() const {
			return mString;
		}

	private:
		const utf::string mString;

		utf::string::const_iterator mIdx;
	};
}
