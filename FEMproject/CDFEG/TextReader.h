// SPDX-License-Identifier: GPL-3.0
// This file is part of CDFEG.
//
// CDFEG is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDFEG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDFEG.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TEXTREADER_H
#define TEXTREADER_H

#include <string>
#include <fstream>
#include <regex>
#include <map>
#include <vector>
namespace CDFEG {
	class TextReader
	{
	public:
		TextReader();
		TextReader(const std::string& filePath);
		~TextReader();
		void setFilePath(const std::string& filePath) { _filePath = filePath; };
		bool open();
		void close();
		bool isOpen() const;
		bool readNextLine();
		 std::string& getCurrentLine()  { return _currentLine; };
		void preLine();
		bool eof() const;
	public:
		static std::string toLowerCase(const std::string& str);
		static bool string2Int(const std::string& str ,int& value);
		static bool string2Double(const std::string& str, double& value);
		static std::string simplifyLine(const std::string& line);
		static std::map<std::string, std::string> parseInfoLine(const std::string& line, bool isToLower=false);
		static std::vector<std::string> split(const std::string& str, const std::string& delimiters);
		static std::vector<double> splitDoubles(const std::string& str, const std::string& delimiters);
		static std::vector<int> splitInts(const std::string& str, const std::string& delimiters);

	private:
		std::ifstream _fileStream;
		std::string _filePath;
		std::string _currentLine;
		std::streampos _lastPos;
		std::string _lineSimple;
	};
}
#endif
