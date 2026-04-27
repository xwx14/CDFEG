#include "TextReader.h"
#include <cctype>
#include <algorithm>

#include <sstream>
namespace CDFEG {

	TextReader::TextReader(const std::string& filePath)
		: _filePath(filePath)
	{
		// 构造函数仅保存文件路径，不自动打开文件
	}

	TextReader::TextReader()
	{

	}

	TextReader::~TextReader()
	{
		// 析构函数自动关闭文件
		close();
	}

	bool TextReader::open()
	{
		// 如果文件已经打开，先关闭
		if (_fileStream.is_open()) {
			_fileStream.close();
		}
		
		// 打开文件
		_fileStream.open(_filePath.c_str());
		
		// 清空当前行
		_currentLine.clear();
		
		// 返回是否打开成功
		return _fileStream.is_open();
	}

	void TextReader::close()
	{
		if (_fileStream.is_open()) {
			_fileStream.close();
		}
		_currentLine.clear();
	}

	bool TextReader::isOpen() const
	{
		return _fileStream.is_open();
	}

	bool TextReader::readNextLine()
	{
		if (!_fileStream.is_open()) {
			_currentLine.clear();
			return false;
		}
		_lastPos = _fileStream.tellg();
		// 读取下一行
		if (std::getline(_fileStream, _currentLine)) {
			return true;
		}
		
		// 如果读取失败（到达文件末尾或出错），清空当前行
		_currentLine.clear();
		return false;
	}

	void TextReader::preLine()
	{
		if (!_fileStream.is_open())return;
		// 回到上一次读取行之前的位置
		//_fileStream.clear(); // 清除EOF标志
		_fileStream.seekg(_lastPos);
	}

	bool TextReader::eof() const
	{
		return _fileStream.eof();
	}

	std::string TextReader::toLowerCase(const std::string& str)
	{
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return result;
	}

	bool TextReader::string2Int(const std::string& str, int& value)
	{
		try {
			value = std::stoi(str);
			return true;
		}
		catch (const std::exception& e) {
			value = 0;
			return false;
		}
	}

	bool TextReader::string2Double(const std::string& str, double& value)
	{
		try {
			value = std::stod(str);
			return true;
		}
		catch (const std::exception& e) {
			value = 0.0;
			return false;
		}
	}

	std::string TextReader::simplifyLine(const std::string& line)
	{
		std::string rt;
		for(const char& c : line)
		{
			if (!std::isspace(static_cast<unsigned char>(c)))
				rt += c;
		}
		return rt;
	}

	std::map<std::string, std::string> TextReader::parseInfoLine(const std::string& line,bool isToLower)
	{
		std::map<std::string, std::string> infoMap;
		bool isIn = false;
		std::string key, value;
		std::string* current = &key;
		for (const char& c : line)
		{
			if (c == '\"') {
				isIn = !isIn;
			}
			else if (isIn) {
				current->push_back(c);
			}
			else if(c == '=') {
				if (current == &key) {
					current = &value;
				}
				else {
					current->push_back(c);
				}
			}
			else if (c == ','|| std::isspace(static_cast<unsigned char>(c))) {
				if (current==&value) {
					// 完成一个键值对的读取
					infoMap[toLowerCase(key)] = value;
					// 重置状态
					key.clear();
					value.clear();
					current = &key;
				}
			}
			else if (!std::isspace(static_cast<unsigned char>(c))) {
				current->push_back(c);
			}
		}
		return infoMap;
	}

	std::vector<std::string> TextReader::split(const std::string& str, const std::string& delimiters)
	{
		std::vector<std::string> result;
		if (str.empty()) {
			return result;
		}
		std::string current;
		for (const char& c : str) {
			// 使用 std::find 检查当前字符是否为分隔符
			if (delimiters.find(c) != std::string::npos) {
				// 遇到分隔符，将当前字符串添加到结果中
				if (!current.empty()) {
					result.push_back(current);
					current.clear();
				}
			} else {
				current.push_back(c);
			}
		}
		// 添加最后一个子串
		if (!current.empty()) {
			result.push_back(current);
		}
		return result;
	}
	std::vector<double> TextReader::splitDoubles(const std::string& str, const std::string& delimiters)
	{
		std::vector<double> result;
		if (str.empty()) {
			return result;
		}
		std::string current;
		double curDouble;
		for (const char& c : str) {
			// 使用 std::find 检查当前字符是否为分隔符
			if (delimiters.find(c) != std::string::npos) {
				// 遇到分隔符，将当前字符串添加到结果中
				if (!current.empty()) {
					string2Double(current,curDouble);
					result.push_back(curDouble);
					current.clear();
				}
			} else {
				current.push_back(c);
			}
		}
		// 添加最后一个子串
		if (!current.empty()) {
			string2Double(current,curDouble);
			result.push_back(curDouble);
		}
		return result;
	}
	std::vector<int> TextReader::splitInts(const std::string& str, const std::string& delimiters)
	{
		std::vector<int> result;
		if (str.empty()) {
			return result;
		}
		std::string current;
		int curInt;
		for (const char& c : str) {
			// 使用 std::find 检查当前字符是否为分隔符
			if (delimiters.find(c) != std::string::npos) {
				// 遇到分隔符，将当前字符串添加到结果中
				if (!current.empty()) {
					string2Int(current,curInt);
					result.push_back(curInt);
					current.clear();
				}
			} else {
				current.push_back(c);
			}
		}
		// 添加最后一个子串
		if (!current.empty()) {
			string2Int(current,curInt);
			result.push_back(curInt);
		}
		return result;
	}

}
