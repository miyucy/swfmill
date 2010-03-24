#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace SWF {

class Parser {
	public:
		Parser(const char begin, const char end, const char parameter = 0);
		virtual ~Parser();
		
	protected:
		virtual void handleData(const string& outside, const vector<string>& inside) = 0;
		void doParse(const char* str);
		void trimString(string& s);			
		
	private:
		void handleDelimiter(string& tmp);
		void swapDelimiters();
		bool isWhitespace(const char c);
				
		char expectedDelimiter;
		char otherDelimiter;
		char parameterDelimiter;
		bool insideParams;
		vector<string> params;
		string outside;
};

}

#endif
