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
